from pickletools import uint8
import numpy as np
import argparse
import re
import time
import hashlib
from version import *

type_scpu = 1
type_ncpu = 2
type_model = 3
type_image = [type_scpu, type_ncpu, type_model]

def file_read_binary(src):
    if src == '':
        return b''

    with open(src, "rb") as f:
        data = f.read()
        return data

def num2bytearray(num, nbyte, byte_order='little'):
    byte_array = bytearray(nbyte)
    for i in range(nbyte):
        if byte_order == 'big':
            byte_array[-1-i] = (num >> (i*8)) & 0xff
        else:
            byte_array[i] = (num >> (i*8)) & 0xff

    return byte_array

def update_part_header(part_type, length, version, addr):
    ret_array = bytearray(13)

    ret_array[0] = part_type
    ret_array[1:5] = num2bytearray(length, 4)
    ret_array[5:9] = version
    ret_array[9:13] = num2bytearray(addr, 4)
    # print(ret_array.hex())

    return ret_array

def get_version_info(ver_path):
    ver_dict = dict()
    with open(ver_path, "r") as f:
        while True:
            data = f.readline()
            # data_array = data.split('\w')
            data_array = re.split('(?:[= \n\r])', data)

            if not data:
                break

            ver_dict[data_array[0]] = data_array[1]

    print(ver_dict)
    return ver_dict

class image_class():
    def __init__(self, enable=False, path = ''):
        self.enable = enable
        self.data = file_read_binary(path)
        self.size = len(self.data)
        pass

    def update_image(self, path):
        if type(path) != str and type(path) != list:
            raise 'type(path) is not list or str'

        if type(path) == str:
            self.data = file_read_binary(path)
            self.size = len(self.data)
            if self.size > 0:
                self.enable = True
        else:
            data_pack = np.empty(0, dtype=np.uint8)

            cnt = 0
            for file_path in path:
                data = file_read_binary(file_path)
                rdata = np.array(list(data), dtype= np.uint8)
                
                if cnt < len(path)-1:
                    rdata = np.append(rdata, np.zeros((((len(rdata)+4096-1) & 0xFFFFF000))-len(rdata), dtype=np.uint8))

                if len(rdata) > 0:
                    data_pack = np.append(data_pack, rdata)
                else:
                    raise 'Read %s error'%file_path

                    

                cnt += 1

            if len(data_pack) > 0:
                self.data = bytes(data_pack)
                self.size = len(self.data)
                self.enable = True

class ota_class():
    def __init__(self):
        self.images = {
            type_scpu: image_class(),
            type_ncpu: image_class(),
            type_model: image_class(),
        }

        self.header_size = 1024
        self.size_part_header = 13
        self.header_data = bytearray(self.header_size)

        self.old_ver_info = version_class()
        self.ver_info = version_class()
        pass

    def __pack_image_header(self, offset, type, version, addr):
        if self.images[type].data != None:
            self.header_data[offset:offset+self.size_part_header] = update_part_header(type, self.images[type].size, version, addr)
            return self.images[type].size
        else:
            raise 'image bin is not exist'
        pass

    def update_version(self, path):
        self.ver_info.get_version_info(path)

    def add_image(self, type, path):
        self.images[type].update_image(path=path)

    def gen_ota_bin(self):
        if self.images[type_scpu].enable == False and \
            self.images[type_ncpu].enable == False and \
            self.images[type_model].enable == False:
            print('No image to update.')
            return

        str_scpu_ver = ''
        str_ncpu_ver = ''
        str_model_ver = ''

        self.header_data[0] = 0xA9
        self.header_data[1] = 0x56

        offset_next_part = 38

        image_addr = self.header_size

        if self.images[type_scpu].enable == True:
            ret_size = self.__pack_image_header(offset_next_part, type=type_scpu, version=self.ver_info.scpu_version, addr=image_addr)
            offset_next_part += self.size_part_header
            image_addr += ret_size
            str_scpu_ver = '_scpu_' + '.'.join(str(i) for i in self.ver_info.scpu_version)
        if self.images[type_ncpu].enable == True:
            ret_size = self.__pack_image_header(offset_next_part, type=type_ncpu, version=self.ver_info.ncpu_version, addr=image_addr)
            offset_next_part += self.size_part_header
            image_addr += ret_size
            str_ncpu_ver = '_ncpu_' + '.'.join(str(i) for i in self.ver_info.ncpu_version)
        if self.images[type_model].enable == True:
            ret_size = self.__pack_image_header(offset_next_part, type=type_model, version=self.ver_info.model_version, addr=image_addr)
            offset_next_part += self.size_part_header
            image_addr += ret_size
            str_model_ver = '_model_' + '.'.join(str(i) for i in self.ver_info.model_version)

        total_size = self.header_size
        if self.images[type_scpu].enable == True:
            total_size += self.images[type_scpu].size
        if self.images[type_ncpu].enable == True:
            total_size += self.images[type_ncpu].size
        if self.images[type_model].enable == True:
            total_size += self.images[type_model].size
        self.header_data[2:6] = num2bytearray(total_size, 4)

        ota_bin_data = bytearray(0)
        ota_bin_data += self.header_data

        if self.images[type_scpu].enable == True:
            ota_bin_data += self.images[type_scpu].data
        if self.images[type_ncpu].enable == True:
            ota_bin_data += self.images[type_ncpu].data
        if self.images[type_model].enable == True:
            ota_bin_data += self.images[type_model].data

        md5_value = hashlib.md5(ota_bin_data[38:]).hexdigest()
        md5_middle_str = bytearray(md5_value, encoding='utf-8')

        merge_str = bytearray(b'KNERON IPO') + md5_middle_str
        md5_value = hashlib.md5(merge_str).hexdigest()
        md5_final_str = bytearray(md5_value, encoding='utf-8')

        ota_bin_data[6:6+32] = md5_final_str
        # print(' '.join(hex(i).replace('0x', '') for i in ota_bin_data[6:6+32]))

        time_str = time.strftime('%Y%m%d_%H%M%S', time.localtime(time.time()))
        merge_file = './ota' + str_scpu_ver + str_ncpu_ver + str_model_ver + '_' + time_str + '.bin'

        try:
            with open(merge_file, 'wb') as binary_file:
                binary_file.write(ota_bin_data)
            print('Write file %s' %merge_file)
        except IOError as e:
            print("Couldn't open or write file (%s)." % e)
        

def __sample_gen_ota_bin(args):
    ota = ota_class()
    ota.old_ver_info.get_version_info('./old_version.cfg')
    ota.ver_info.get_version_info('./version.cfg')
    if args.scpu or args.ncpu or args.model:
        if args.scpu:
            ota.add_image(type=type_scpu, path='./flash_bin/fw_scpu.bin')
        if args.ncpu:
            ota.add_image(type=type_ncpu, path='./flash_bin/fw_ncpu.bin')
        if args.model:
            ota.add_image(type=type_model, path=['./flash_bin/fw_info.bin', './flash_bin/all_models.bin'])
    else:
        if ota.old_ver_info.scpu_version != ota.ver_info.scpu_version:
            ota.add_image(type=type_scpu, path='./flash_bin/fw_scpu.bin')
        if ota.old_ver_info.ncpu_version != ota.ver_info.ncpu_version:
            ota.add_image(type=type_ncpu, path='./flash_bin/fw_ncpu.bin')
        if ota.old_ver_info.model_version != ota.ver_info.model_version:
            ota.add_image(type=type_model, path=['./flash_bin/fw_info.bin', './flash_bin/all_models.bin'])
    ota.gen_ota_bin()
    ota.ver_info.backup_version('./old_version.cfg')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', help='add scpu bin into ota.bin', dest='scpu', action='store_true')
    parser.add_argument('-n', help='add ncpu bin into ota.bin', dest='ncpu', action='store_true')
    parser.add_argument('-m', help='add model bin into ota.bin', dest='model', action='store_true')

    args = parser.parse_args()
    __sample_gen_ota_bin(args)
