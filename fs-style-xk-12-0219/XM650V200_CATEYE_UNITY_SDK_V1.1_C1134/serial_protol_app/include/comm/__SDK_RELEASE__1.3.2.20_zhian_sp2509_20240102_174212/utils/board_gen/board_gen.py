'''
 @file board_gen.py
 @brief Generate flash final bin file from other seperate bin files.
'''

import argparse
import io
import os
import numpy as np
import ek_common
import cv2
import flash_programmer as fp
import setup
import logging
import re

import version

USER_FLASH_IMG_TABLE_FILE_NAME = 'usr_flash_img_table.h'
USER_DDR_IMG_TABLE_FILE_NAME = 'usr_ddr_img_table.h'
REGRESSION_UTILS_WORDS_PY_PATH = '../../regression/utils/words.py'

#flash_fdfr_bin_info = [
#                  [0x00000000, 8*1024,  'BOOT_SPL', './flash_bin/boot_spl.bin'],
#                  [0x00002000, 160*1024, 'FW_SCPU',  './flash_bin/fw_scpu.bin'],
#                  [0x0002A000, 64*1024, 'FW_NCPU', './flash_bin/fw_ncpu.bin'],
#                  [0x0003A000, 4*1024, 'BOOT_CFG0', './flash_bin/boot_cfg0.bin'],
#                  [0x0003B000, 24*1024, 'FW_RESERVED1', './flash_bin/fw_reserved.bin'],
#                  [0x00041000, 160*1024, 'FW_SCPU1', './flash_bin/fw_scpu1.bin'],
#                  [0x00069000, 64*1024, 'FW_NCPU1', './flash_bin/fw_ncpu1.bin'],
#                  [0x00079000, 4*1024, 'BOOT_CFG1', './flash_bin/boot_cfg1.bin'],
#                  [0x0007A000, 24*1024, 'FW_RESERVED2', './flash_bin/fw_reserved.bin'],
#                  [0x00080000, 2621440, 'FID_MAP', './flash_bin/fid_map.bin'],
#                  [0x00300000, 180, 'FW_INFO', './flash_bin/fw_info.bin'],
#                  [0x00301000, 16184832, 'ALL_MODELS', './flash_bin/all_models.bin'],
#                  [0x01271000, 614400, 'RGB', './flash_bin/rgb.bin'],
#                  [0x01307000, 307200, 'NIR', './flash_bin/nir.bin']]
#
#KDP_FLASH_END_ADDR = 0x01307000 + 307200

usr_bin=[]
g_fw_info_addr = 0
g_all_models_addr = 0
g_user_addr = 0

file_bin_info = 'flash_bin_info.cfg'

CFG_UI_ENABLE=0
CFG_UI_USR_IMG=0
CFG_AI_USE_FIXED_IMG=0
CFG_SNAPSHOT_ENABLE=0
CFG_SNAPSHOT_NUMS=0
CFG_SNAPSHOT_ADVANCED=0
CFG_KDP_SETTINGS_ENABLE=0
CFG_KDP_SETTINGS_SIZE=0
CFG_USR_SETTINGS_ENABLE=0
CFG_USR_SETTINGS_SIZE=0
CFG_OTA_FLASH_SLAVE_ENABLE=0

#flash type check
CFG_FLASH_VENDOR =199 #Jeff add
IMAGE_SIZE = 0

IMG_RGB_WIDTH = 640
IMG_RGB_HEIGHT = 480
IMG_NIR_WIDTH = 480
IMG_NIR_HEIGHT = 640

KDP_FLASH_USER_CFG_SIZE = 0x1000
KDP_FLASH_FW_INFO_RESERVED = 0x1000
KDP_FLASH_MODEL_RESERVED = 0x1500000
KDP_FLASH_USER_RESERVED = 0x500000
KDP_FLASH_32MB = 0x2000000
KDP_DDR_TEST_RGB_IMG_SIZE = IMG_RGB_WIDTH * IMG_RGB_HEIGHT * 2
KDP_DDR_TEST_NIR_IMG_SIZE = IMG_NIR_WIDTH * IMG_NIR_HEIGHT
KDP_DDR_TEST_INF_IMG_SIZE = 0x2000
KDP_DDR_TEST_IMG_NAME_SIZE = 0x1000
KDP_DDR_TEST_RGB_IMG_ADDR = 0
KDP_DDR_TEST_NIR_IMG_ADDR = 0
KDP_DDR_TEST_INF_IMG_ADDR = 0
KDP_DDR_TEST_IMG_NAME_ADDR = 0
KDP_DDR_TEST_RGB_FR_SIZE  = 0x400
KDP_DDR_TEST_NIR_FR_SIZE  = 0x400
KDP_DDR_TEST_USER_DB_SIZE = 0x4000


def jlink_gen():
    global CFG_FLASH_VENDOR
    
    fo = open("../JLink_programmer/flash_prog.jlink", "w")    
    
    str="si 1\n"
    fo.write( str )
    str="speed 10000\n"
    fo.write( str )
    str="device "
    fo.write( str )
    if CFG_FLASH_VENDOR=='GD25Q256D':
        str="KL520-GD\n"
        fo.write( str )
    elif CFG_FLASH_VENDOR=='GD25S512MD':
        str="KL520-GD64M\n"
        fo.write( str )
    elif CFG_FLASH_VENDOR=='W25Q256JV':
        str="KL520-WB\n"
        fo.write( str )    
    elif CFG_FLASH_VENDOR=='W25M512JV':
        str="KL520-GD64M\n"
        fo.write( str )        
    else:
        str="board gen error\n"
        fo.write( str )    

    #str="r\n"
    fo.write( "r\n" )

    #str="h\n"
    fo.write( "h\n" )

    #str="loadbin .\\bin\\flash_fdfr_image.bin,0x00000000\n"
    fo.write( "loadbin .\\bin\\flash_fdfr_image.bin,0x00000000\n" )

    #str="r\n"
    fo.write( "r\n" )
    #str="g\n"
    fo.write( "g\n" )
        
    #fo.write( "{}".format(CFG_FLASH_VENDOR) )
    fo.close()

def bin_gen(bin_info, ofile):
    bin_len = len(bin_info)
    total_bin_size = bin_info[bin_len - 1][0] + bin_info[bin_len - 1][1]
    bin_data = np.zeros(total_bin_size, dtype=np.uint8)
    bin_data.fill(0xff)
    for bin_file in bin_info:
        #print('start parsing, start: 0x{:08X}, size: {:08X}, file={:s}'.format(bin_file[0], bin_file[1], bin_file[2]))
        bin_start_addr = bin_file[0]
        bin_size = bin_file[1]
        bin_file_bak = bin_file[2]
        bin_file = bin_file[3]

        if os.path.isfile(bin_file) == 0:
            if  bin_file_bak != 'PADDING':
                print('{:s} is not exists'.format(bin_file))
        else:
            rdata = file_read_binary(bin_file)
            rdata = list(rdata)
            rdata = np.array(rdata)
            bin_data[bin_start_addr:bin_start_addr + len(rdata)] = rdata
    write_data_to_binary_file(ofile, bin_data)

def write_board_params(cnt, str_tmp, str_val, struct_code, header_file):
    if str_tmp[-1] == 'd':
        #struct_code.append([f'param_{cnt:d}', f'{str_val:s}'])
        #header_file.write(f'    int param_{cnt:d};\n')
        struct_code.append([f'{str_val:s}'])
    elif str_tmp[-1] == 'f':
        #struct_code.append([f'param_{cnt:d}', f'{str_val:s}f'])
        #header_file.write(f'    float param_{cnt:d};\n')
        struct_code.append([f'{str_val:s}f'])

def board_gen_cfg(build_name, cfg_file_path):
    global file_bin_info

    model_folder = './'

    cfg_file = open(cfg_file_path, 'r')
    alllines = cfg_file.readlines()
    cfg_file.close()
    target_header_file_name = 'board_cfg.h'
    header_file_name = f'../../build/{build_name:s}/config/{target_header_file_name:s}'
    header_file = open(header_file_name, 'w', newline='\n')
    if header_file is not None:
        header_file.write('#ifndef __BOARD_CFG_H__\n')
        header_file.write('#define __BOARD_CFG_H__\n\n\n')
        for line in alllines:
            if line[0] == '#': continue
            if line.strip()=='': continue
            if line[0] =='!':
                cfg_item_name = line.split('!')[1]
                header_file.write('// %s' %cfg_item_name)
                continue
            if line[0] =='&':
                [cfg_item_name, value] = line.strip().split('&')[1].split('=')
                if cfg_item_name == 'MODEL':
                    model_folder += value
                continue
            if line[0] == '@':
                params = line.split(',')
                cfg_item_name = line.split('=')[0][1:]
                value = '1'
                struct_code = []
                #header_file.write(f'#define {cfg_item_name:<40s}{value:>32s}')
                #header_file.write('\nstruct board_parameters {\n')
                cnt = 0
                for p in params:
                    res1 = p.find('=')
                    if res1 != -1:
                        str_tmp = p.split('=')[1]
                        str_val = str_tmp[:-1]
                        write_board_params(cnt, str_tmp, str_val, struct_code, header_file)
                        cnt = cnt + 1
                    else:
                        if p[-1] == '\n':
                            str_tmp = p[-2]
                            str_val = p[:-2]
                            write_board_params(cnt, str_tmp, str_val, struct_code, header_file)
                        else:
                            str_tmp = p[-1]
                            str_val = p[:-1]
                            write_board_params(cnt, str_tmp, str_val, struct_code, header_file)
                        cnt = cnt + 1
                #header_file.write('};\n')
                #header_file.write('static struct board_parameters board_params = {')
                cnt = 0
                item_list = '{'
                for item in struct_code:
                    if cnt == len(struct_code) -1:
                        #header_file.write(f'{item[1]:s} ')
                        item_list = item_list + item[0]
                    else:
                        #header_file.write(f'{item[1]:s}, ')
                        item_list = item_list + item[0] + ','
                    cnt = cnt + 1
                header_file.write(f'#define {cfg_item_name:<26s}{item_list:>55}' + '}\n')
                #header_file.write('};\n')
                continue


            cfg_item_name = line.split('=')[0]
            value = line.split('=')[1]
            header_file.write(f'#define {cfg_item_name:<26}{value:>57s}')
        header_file.write('\n' + '#endif' + '\n')
        header_file.close()

    user_all_models_name =  f'../../build/{build_name:s}/user_all_models/'
    default_bin_path = './default_bin'
    if os.path.isdir(model_folder):
        print('use ', model_folder)
        ek_common.mycopytree(model_folder, "./flash_bin/")
        print('Copy %s to %s' %(model_folder, "./flash_bin/"))
    elif os.path.exists(user_all_models_name):
        print('[USER MODEL COPY]  EXIST')
        ek_common.mycopytree(user_all_models_name, "./flash_bin/")
        print('Copy %s to %s' %(user_all_models_name, "./flash_bin/"))
    elif os.path.exists(default_bin_path):
        print('use default_bin')
        ek_common.mycopytree(default_bin_path, "./flash_bin/")
        print('Copy %s to %s' %(default_bin_path, "./flash_bin/"))
    else:
        print('use latest bin')



###############################################################################
###############################################################################
###############################################################################
KDP_FLASH_TABLE_FILE_NAME = 'kdp_flash_table.h'
def board_gen_kdp_flash(build_name, gen_h_file, flash_bin_table):
    global CFG_SNAPSHOT_ENABLE
    global CFG_SNAPSHOT_ADVANCED
    global CFG_OTA_FLASH_SLAVE_ENABLE
    global IMAGE_SIZE
    global g_user_addr
    KDP_FLASH_NEXT_START_ADDR = 0 
    KDP_FLASH_OFFSET = 0
    header_file = None
    if gen_h_file == True:
        target_header_file_name = KDP_FLASH_TABLE_FILE_NAME
        header_file_name = f'../../build/{build_name:s}/config/{target_header_file_name:s}'
        header_file = open(header_file_name, 'w', newline='\n')

        if header_file is not None:
            header_file.write('#ifndef __KDP_FLASH_TABLE_H__\n')
            header_file.write('#define __KDP_FLASH_TABLE_H__\n\n\n')

    for info in flash_bin_table:
        if header_file is not None:
            tmp = '#define KDP_FLASH_%s_ADDR' %(info[2].upper())
            str = '%-43s 0x%08X // file=%s' %(tmp, info[0], info[3])
            header_file.write(str + '\n')
            tmp = '#define KDP_FLASH_%s_SIZE' %(info[2].upper())
            str = '%-43s 0x%08X' %(tmp, info[1])
            header_file.write(str + '\n')

        if (CFG_OTA_FLASH_SLAVE_ENABLE == 1) and (IMAGE_SIZE > 16):
            KDP_FLASH_NEXT_START_ADDR = info[0] + KDP_FLASH_MODEL_RESERVED
        else:
            KDP_FLASH_NEXT_START_ADDR = info[0] + info[1]

    KDP_FLASH_NEXT_START_ADDR = (KDP_FLASH_NEXT_START_ADDR + 4095) & 0xfffff000

    if header_file is not None:
        tmp = '#define KDP_FLASH_INF_ADDR'
        str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
        header_file.write(str + '\n')
        tmp = '#define KDP_FLASH_INF_SIZE'
        str = '%-43s 0x%08X' %(tmp, KDP_DDR_TEST_INF_IMG_SIZE)
        header_file.write(str + '\n')
    flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_DDR_TEST_INF_IMG_SIZE, 'INFO', 'test info'])
    KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_DDR_TEST_INF_IMG_SIZE
    
    if CFG_SNAPSHOT_ENABLE == 1 and CFG_SNAPSHOT_ADVANCED == 1:
        KDP_SNAPSHOT_IMG_SIZE = 0x26000 #320x240x2 + Head Infor(0x30)
        KDP_SNAPSHOT_IMG_SIZE_4K = 0x25800 #320x240x2
        sector_num = KDP_SNAPSHOT_IMG_SIZE_4K // 4096
        if (KDP_SNAPSHOT_IMG_SIZE_4K % 4096) > 0:
            sector_num = sector_num + 1
        KDP_SNAPSHOT_IMG_SIZE_4K = sector_num * 4096


        if header_file is not None:
            header_file.write('///////////////////////////////////////////\n')
            header_file.write('/* optional functions - snapshot section */\n')
            header_file.write('///////////////////////////////////////////\n')

            tmp = '#define KDP_FLASH_SNAPSHOT_IMG_SIZE'
            str = '%-43s 0x%08X' %(tmp, KDP_SNAPSHOT_IMG_SIZE)
            header_file.write(str + '\n')

        for i in range(CFG_SNAPSHOT_NUMS):
            tmp = '#define KDP_FLASH_SNAPSHOT_IMG_%02d_ADDR' %(i)
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
            if header_file is not None:
                header_file.write(str + '\n')

            str2 = 'snapshot_reserved_%d' %(i)
            flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_SNAPSHOT_IMG_SIZE, str2, str2])
            #KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_SNAPSHOT_IMG_SIZE_4K
            KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_SNAPSHOT_IMG_SIZE

            if (KDP_FLASH_NEXT_START_ADDR % 4096) > 0:
                remainder = 4096 - (KDP_FLASH_NEXT_START_ADDR % 4096)
                flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, remainder, 'PADDING', 'padding'])
                KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + remainder

    if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
        if (KDP_FLASH_32MB - KDP_FLASH_NEXT_START_ADDR) > (KDP_FLASH_USER_CFG_SIZE * 2) :
            remainder = KDP_FLASH_32MB - (2*KDP_FLASH_USER_CFG_SIZE) - KDP_FLASH_NEXT_START_ADDR
            flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, remainder, 'PADDING', 'padding'])
            KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + remainder
        
        if header_file is not None:
            header_file.write('////////////////////////////////////////////\n')
            header_file.write('/* optional functions - ota slave section */\n')
            header_file.write('////////////////////////////////////////////\n')

            tmp = '#define KDP_FLASH_USER_CFG0_ADDR'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
            header_file.write(str + '\n')
            tmp = '#define KDP_FLASH_USER_CFG0_SIZE'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_USER_CFG_SIZE)
            header_file.write(str + '\n')
        
        flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_FLASH_USER_CFG_SIZE, 'USER_CFG0', './flash_bin/user_cfg0.bin'])
        KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_FLASH_USER_CFG_SIZE

        if header_file is not None:
            tmp = '#define KDP_FLASH_USER_CFG1_ADDR'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
            header_file.write(str + '\n')
            tmp = '#define KDP_FLASH_USER_CFG1_SIZE'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_USER_CFG_SIZE)
            header_file.write(str + '\n')
        
        flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_FLASH_USER_CFG_SIZE, 'USER_CFG1', './flash_bin/user_cfg1.bin'])
        KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_FLASH_USER_CFG_SIZE

        if (KDP_FLASH_32MB - KDP_FLASH_NEXT_START_ADDR) > 0: 
            remainder = KDP_FLASH_32MB - KDP_FLASH_NEXT_START_ADDR
            flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, remainder, 'PADDING', 'padding'])
            KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + remainder

        if header_file is not None:
            tmp = '#define KDP_FLASH_FW_INFO_OFFSET_1'
            KDP_FLASH_OFFSET = KDP_FLASH_NEXT_START_ADDR - g_fw_info_addr
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_OFFSET)
            header_file.write(str + '\n')

        flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_FLASH_FW_INFO_RESERVED, 'FW_INFO_1', './flash_bin/fw_info.bin'])
        KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_FLASH_FW_INFO_RESERVED

        if header_file is not None:
            tmp = '#define KDP_FLASH_ALL_MODEL_OFFSET_1'
            KDP_FLASH_OFFSET = KDP_FLASH_NEXT_START_ADDR - g_all_models_addr
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_OFFSET)
            header_file.write(str + '\n')
            
        flash_fdfr_bin_info.append([KDP_FLASH_NEXT_START_ADDR, KDP_FLASH_MODEL_RESERVED, 'ALL_MODEL_1', './flash_bin/all_models.bin'])
        KDP_FLASH_NEXT_START_ADDR = KDP_FLASH_NEXT_START_ADDR + KDP_FLASH_MODEL_RESERVED

        if header_file is not None:
            g_user_addr = KDP_FLASH_NEXT_START_ADDR
            tmp = '#define KDP_FLASH_USER_ADDR'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
            header_file.write(str + '\n')

            tmp = '#define KDP_FLASH_USER_OFFSET_1'
            KDP_FLASH_OFFSET = KDP_FLASH_USER_RESERVED
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_OFFSET)
            header_file.write(str + '\n')
            

    if header_file is not None:
        board_gen_write_file_s___(header_file, '\n')

    sector_num = KDP_FLASH_NEXT_START_ADDR // 4096
    if (KDP_FLASH_NEXT_START_ADDR % 4096) > 0:
        sector_num = sector_num + 1
    KDP_FLASH_NEXT_START_ADDR = sector_num * 4096

    if header_file is not None:
        if CFG_OTA_FLASH_SLAVE_ENABLE == 0:  
            tmp = '#define KDP_FLASH_LAST_ADDR'
            str = '%-43s 0x%08X' %(tmp, KDP_FLASH_NEXT_START_ADDR)
            header_file.write(str + '\n')
        board_gen_write_file_s___(header_file, '\n')
        board_gen_write_file_s___(header_file, '#endif\n')


    return KDP_FLASH_NEXT_START_ADDR

###############################################################################
###############################################################################
###############################################################################
def board_gen_write_file_s___(header_file, string):
    header_file.write('%s' %(string))
def board_gen_write_file_sh__(header_file, string, address):
    header_file.write('%-40s    0x%08X\n' %(string, address))
def board_gen_write_file_ss__(header_file, string, string2):
    header_file.write('%-40s    %s\n' %(string, string2))
def board_gen_write_file_ssh_(header_file, string, string2, address):
    header_file.write('%-40s    %s 0x%08X\n' %(string, string2, address))
def board_gen_write_file_shs_(header_file, string, address, string2):
    header_file.write('%-40s    0x%08X%s\n' %(string, address, string2))

KDP_DDR_TABLE_FILE_NAME = 'kdp_ddr_table.h'
def board_gen_kdp_ddr(build_name, gen_h_file, cfg_file_path):
    global CFG_UI_ENABLE
    global CFG_UI_USR_IMG
    global CFG_SNAPSHOT_ENABLE
    global CFG_SNAPSHOT_NUMS
    global CFG_KDP_SETTINGS_ENABLE
    global CFG_KDP_SETTINGS_SIZE
    global CFG_USR_SETTINGS_ENABLE
    global CFG_USR_SETTINGS_SIZE
    global KDP_DDR_TEST_RGB_IMG_ADDR
    global KDP_DDR_TEST_NIR_IMG_ADDR
    global CFG_OTA_FLASH_SLAVE_ENABLE

    global CFG_FLASH_VENDOR
    global IMAGE_SIZE
    global file_bin_info
    
    cfg_file = open(cfg_file_path, 'r')
    alllines = cfg_file.readlines()
    cfg_file.close()

    header_file = None
    if gen_h_file == True:
        target_header_file_name = KDP_DDR_TABLE_FILE_NAME
        header_file_name = f'../../build/{build_name:s}/config/{target_header_file_name:s}'
        header_file = open(header_file_name, 'w', newline='\n')


    KDP_DDR_MEM_START = 0x60000000
    KDP_DDR_MEM_END = 0x63FFFFFF
    MODEL_RESERVED_SIZE = 0x1E00000
    KDP_DDR_MODEL_RESERVED_END = KDP_DDR_MEM_START + MODEL_RESERVED_SIZE - 1
    KDP_DDR_BASE_SYSTEM_RESERVED = 0#0x61FD0000
    KDP_DDR_MODEL_INFO_TEMP = 0#0x61FFC000
    KDP_DDR_MODEL_END_ADDR = 0#0x61FFFFFF

    if header_file is not None:
        board_gen_write_file_s___(header_file, '#ifndef __KDP_DDR_TABLE_H__\n')
        board_gen_write_file_s___(header_file, '#define __KDP_DDR_TABLE_H__\n\n\n')
        board_gen_write_file_s___(header_file, '#include "board_kl520.h"\n')
        board_gen_write_file_s___(header_file, '\n')
        board_gen_write_file_sh__(header_file, '#define KDP_DDR_MEM_START', KDP_DDR_MEM_START)
        board_gen_write_file_sh__(header_file, '#define KDP_DDR_MEM_END', KDP_DDR_MEM_END)
        board_gen_write_file_s___(header_file, '//////////////////////////\n')
        board_gen_write_file_s___(header_file, '/* ddr - models section */\n')
        board_gen_write_file_s___(header_file, '//////////////////////////\n')
        board_gen_write_file_ss__(header_file, '#define KDP_DDR_MODEL_START_ADDR', 'KDP_DDR_MEM_START')
        board_gen_write_file_shs_(header_file, '#define KDP_DDR_MODEL_RESERVED_END', KDP_DDR_MODEL_RESERVED_END, ' //reserved 30 MB for models')

    global KDP_DDR_TEST_RGB_IMG_SIZE
    global KDP_DDR_TEST_NIR_IMG_SIZE
    CFG_SENSOR_TYPE = 0
    CFG_SENSOR_0_FULL_RESOLUTION = 0
    CFG_SENSOR_1_FULL_RESOLUTION = 0
    CFG_AI_TYPE = 3
    CFG_AI_TYPE_STRUCT_LIGHT = 0
    CFG_AI_TYPE_NIR_STAGE_LIGHT = 0

    SENSOR_RGB_INDEX = '0'
    SENSOR_NIR_INDEX = '1'
    SENSOR_0_FORMAT = 2
    SENSOR_1_FORMAT = 1
	
    for line in alllines:
        if line[0] == '#': continue
        if line.strip()=='': continue
        if line[0] == '!': continue
        line = line.rstrip('\n')
        [cfg_item_name, value] = line.split('=')

        if cfg_item_name == 'CFR_CAM_RGB':
            SENSOR_RGB_INDEX = value
        if cfg_item_name == 'CFR_CAM_NIR':
            SENSOR_NIR_INDEX = value

        if cfg_item_name == 'CFG_SENSOR_0_WIDTH':
            SENSOR_0_WIDTH = int(value)
        if cfg_item_name == 'CFG_SENSOR_0_HEIGHT':
            SENSOR_0_HEIGHT = int(value)

        if cfg_item_name == 'CFG_SENSOR_1_WIDTH':
            SENSOR_1_WIDTH = int(value)
        if cfg_item_name == 'CFG_SENSOR_1_HEIGHT':
            SENSOR_1_HEIGHT = int(value)
        if cfg_item_name == 'CFG_AI_TYPE' and value == 'AI_TYPE_N1':
            CFG_AI_TYPE = 2
        if CFG_AI_TYPE == 2 and cfg_item_name == 'CFG_E2E_STRUCT_LIGHT' and value == '1':
            CFG_AI_TYPE_STRUCT_LIGHT = 1
        if cfg_item_name == 'CFG_E2E_NIR_TWO_STAGE_LIGHT' and value == '1':
            CFG_AI_TYPE_NIR_STAGE_LIGHT = 2
        if cfg_item_name == 'CFR_SENSOR_0_FORMAT':
            if value == 'IMAGE_FORMAT_RAW8':
                SENSOR_0_FORMAT = 1
            else:
                SENSOR_0_FORMAT = 2
        if cfg_item_name == 'CFR_SENSOR_1_FORMAT':
            if value == 'IMAGE_FORMAT_RAW8':
                SENSOR_1_FORMAT = 1
            else:
                SENSOR_1_FORMAT = 2		    
						
    if SENSOR_RGB_INDEX == '0':
        IMG_RGB_WIDTH = SENSOR_0_WIDTH
        IMG_RGB_HEIGHT = SENSOR_0_HEIGHT
    else:
        IMG_RGB_WIDTH = SENSOR_1_WIDTH
        IMG_RGB_HEIGHT = SENSOR_1_HEIGHT

    if SENSOR_NIR_INDEX == '0':
        IMG_NIR_WIDTH = SENSOR_0_WIDTH
        IMG_NIR_HEIGHT = SENSOR_0_HEIGHT
    else:
        IMG_NIR_WIDTH = SENSOR_1_WIDTH
        IMG_NIR_HEIGHT = SENSOR_1_HEIGHT

    #print( 'RGB', SENSOR_RGB_INDEX, IMG_RGB_WIDTH, IMG_RGB_HEIGHT, 'NIR', SENSOR_NIR_INDEX, IMG_NIR_WIDTH, IMG_NIR_HEIGHT )

    KDP_DDR_TEST_RGB_IMG_SIZE = IMG_RGB_WIDTH * IMG_RGB_HEIGHT * SENSOR_0_FORMAT
    KDP_DDR_TEST_NIR_IMG_SIZE = IMG_NIR_WIDTH * IMG_NIR_HEIGHT * SENSOR_1_FORMAT

    if CFG_AI_TYPE_STRUCT_LIGHT == 1 or CFG_AI_TYPE_NIR_STAGE_LIGHT == 2:
        KDP_DDR_TEST_RGB_IMG_SIZE = KDP_DDR_TEST_NIR_IMG_SIZE 

    # KDP_DDR_HEAP_HEAD_FOR_MALLOC = KDP_DDR_MODEL_RESERVED_END + 0xCB8000 + (KDP_DDR_TEST_RGB_IMG_SIZE + KDP_DDR_TEST_NIR_IMG_SIZE + KDP_DDR_TEST_INF_IMG_SIZE ) * (3 + 1) + (100 * 16 * 1024)
    KDP_DDR_BASE_SYSTEM_RESERVED = KDP_DDR_MODEL_RESERVED_END + 1 #0x61FD0000
    KDP_DDR_MODEL_INFO_TEMP = (KDP_DDR_BASE_SYSTEM_RESERVED + 0x2c000) #0x61FFC000
    KDP_DDR_MODEL_END_ADDR = (KDP_DDR_MODEL_INFO_TEMP + 0x3FFF) #0x61FFFFFF

    if header_file is not None:
        # board_gen_write_file_shs_(header_file, '#define KDP_DDR_HEAP_HEAD_FOR_MALLOC', KDP_DDR_HEAP_HEAD_FOR_MALLOC, ' //reserved 1856 KB for internal memory use')
        board_gen_write_file_shs_(header_file, '#define KDP_DDR_BASE_SYSTEM_RESERVED', KDP_DDR_BASE_SYSTEM_RESERVED, ' //reserved 191 KB')
        board_gen_write_file_shs_(header_file, '#define KDP_DDR_MODEL_INFO_TEMP', KDP_DDR_MODEL_INFO_TEMP, ' //reserved 16 KB for model info')
        board_gen_write_file_sh__(header_file, '#define KDP_DDR_MODEL_END_ADDR', KDP_DDR_MODEL_END_ADDR)
        board_gen_write_file_s___(header_file, '///////////////////////////\n')
        board_gen_write_file_s___(header_file, '/* ddr - drivers section */\n')
        board_gen_write_file_s___(header_file, '///////////////////////////\n')
        board_gen_write_file_ss__(header_file, '#define KDP_DDR_DRV_START_ADDR', '(KDP_DDR_MODEL_END_ADDR + 1)')
    KDP_DDR_DRV_START_ADDR = KDP_DDR_MODEL_END_ADDR + 1
    KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_START_ADDR
    KDP_DDR_TEST_RGB_IMG_ADDR = 0
    KDP_DDR_TEST_NIR_IMG_ADDR = 0
    KDP_DDR_DRV_MIPIRX_RGB_TO_NCPU = 0
    KDP_DDR_DRV_MIPIRX_NIR_TO_NCPU = 0
    #opt_snapshot_enable = 0
    for line in alllines:
        if line[0] == '#': continue
        if line[0] == '!': continue
        if line.strip()=='': continue
        line = line.rstrip('\n')
        [cfg_item_name, value] = line.split('=')

        if line[0] =='&':
            [cfg_item_name, value] = line.strip().split('&')[1].split('=')
            if cfg_item_name == 'BIN_INFO':
                file_bin_info = value
                print('file_bin_info:', file_bin_info)

        if cfg_item_name == 'CFG_SENSOR_TYPE':
            KDP_DDR_DRV_MIPIRX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            KDP_DDR_DRV_RGB_IMG_MAX_SIZE = 0x000EA600
            KDP_DDR_DRV_NIR_IMG_MAX_SIZE = 0x00075300

        elif cfg_item_name == 'CFG_PANEL_TYPE':

            if value == '0' or value == 'PANEL_NULL':
                CFG_UI_ENABLE = 0
            else:
                CFG_UI_ENABLE = 1

                KDP_DDR_DRV_LCM_RESERVED = 0x000EA600
                if header_file is not None:
                    board_gen_write_file_s___(header_file, '\n')
                    board_gen_write_file_s___(header_file, '/* @media driver section : lcm */\n')
                    board_gen_write_file_s___(header_file, '#if CFG_LCM_DMA_ENABLE == YES\n')
                    board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_LCM_RESERVED', KDP_DDR_DRV_LCM_RESERVED)
                KDP_DDR_DRV_LCM_START_ADDR = KDP_DDR_NEXT_START_ADDR
                if header_file is not None:
                    board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_LCM_START_ADDR', KDP_DDR_DRV_LCM_START_ADDR)
                KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_LCM_START_ADDR + KDP_DDR_DRV_LCM_RESERVED
                if header_file is not None:
                    board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_LCM_ENDP1_ADDR', '(KDP_DDR_DRV_LCM_START_ADDR + KDP_DDR_DRV_LCM_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                    board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART1_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART1_TX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART1 TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART1_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART1_TX_RESERVED', KDP_DDR_DRV_UART1_TX_RESERVED)
            KDP_DDR_DRV_UART1_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART1_TX_START_ADDR', KDP_DDR_DRV_UART1_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART1_TX_START_ADDR + KDP_DDR_DRV_UART1_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART1_TX_ENDP1_ADDR', '(KDP_DDR_DRV_UART1_TX_START_ADDR + KDP_DDR_DRV_UART1_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART1_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART1_RX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART1 RX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART1_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART1_RX_RESERVED', KDP_DDR_DRV_UART1_RX_RESERVED)
            KDP_DDR_DRV_UART1_RX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART1_RX_START_ADDR', KDP_DDR_DRV_UART1_RX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART1_RX_START_ADDR + KDP_DDR_DRV_UART1_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART1_RX_ENDP1_ADDR', '(KDP_DDR_DRV_UART1_RX_START_ADDR + KDP_DDR_DRV_UART1_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART2_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART2_TX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART2 TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART2_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART2_TX_RESERVED', KDP_DDR_DRV_UART2_TX_RESERVED)
            KDP_DDR_DRV_UART2_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART2_TX_START_ADDR', KDP_DDR_DRV_UART2_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART2_TX_START_ADDR + KDP_DDR_DRV_UART2_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART2_TX_ENDP1_ADDR', '(KDP_DDR_DRV_UART2_TX_START_ADDR + KDP_DDR_DRV_UART2_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART2_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART2_RX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART2 RX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART2_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART2_RX_RESERVED', KDP_DDR_DRV_UART2_RX_RESERVED)
            KDP_DDR_DRV_UART2_RX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART2_RX_START_ADDR', KDP_DDR_DRV_UART2_RX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART2_RX_START_ADDR + KDP_DDR_DRV_UART2_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART2_RX_ENDP1_ADDR', '(KDP_DDR_DRV_UART2_RX_START_ADDR + KDP_DDR_DRV_UART2_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART3_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART3_TX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART3 TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART3_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART3_TX_RESERVED', KDP_DDR_DRV_UART3_TX_RESERVED)
            KDP_DDR_DRV_UART3_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART3_TX_START_ADDR', KDP_DDR_DRV_UART3_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART3_TX_START_ADDR + KDP_DDR_DRV_UART3_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART3_TX_ENDP1_ADDR', '(KDP_DDR_DRV_UART3_TX_START_ADDR + KDP_DDR_DRV_UART3_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART3_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART3_RX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART3 RX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART3_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART3_RX_RESERVED', KDP_DDR_DRV_UART3_RX_RESERVED)
            KDP_DDR_DRV_UART3_RX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART3_RX_START_ADDR', KDP_DDR_DRV_UART3_RX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART3_RX_START_ADDR + KDP_DDR_DRV_UART3_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART3_RX_ENDP1_ADDR', '(KDP_DDR_DRV_UART3_RX_START_ADDR + KDP_DDR_DRV_UART3_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART4_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART4_TX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART4 TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART4_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART4_TX_RESERVED', KDP_DDR_DRV_UART4_TX_RESERVED)
            KDP_DDR_DRV_UART4_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART4_TX_START_ADDR', KDP_DDR_DRV_UART4_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART4_TX_START_ADDR + KDP_DDR_DRV_UART4_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART4_TX_ENDP1_ADDR', '(KDP_DDR_DRV_UART4_TX_START_ADDR + KDP_DDR_DRV_UART4_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_UART4_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_UART4_RX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : UART4 RX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_UART4_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART4_RX_RESERVED', KDP_DDR_DRV_UART4_RX_RESERVED)
            KDP_DDR_DRV_UART4_RX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_UART4_RX_START_ADDR', KDP_DDR_DRV_UART4_RX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_UART4_RX_START_ADDR + KDP_DDR_DRV_UART4_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_UART4_RX_ENDP1_ADDR', '(KDP_DDR_DRV_UART4_RX_START_ADDR + KDP_DDR_DRV_UART4_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_ADC0_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_ADC0_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : ADC0 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_ADC0_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC0_RESERVED', KDP_DDR_DRV_ADC0_RESERVED)
            KDP_DDR_DRV_ADC0_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC0_START_ADDR', KDP_DDR_DRV_ADC0_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_ADC0_START_ADDR + KDP_DDR_DRV_ADC0_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_ADC0_ENDP1_ADDR', '(KDP_DDR_DRV_ADC0_START_ADDR + KDP_DDR_DRV_ADC0_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_ADC1_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_ADC1_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : ADC1 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_ADC1_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC1_RESERVED', KDP_DDR_DRV_ADC1_RESERVED)
            KDP_DDR_DRV_ADC1_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC1_START_ADDR', KDP_DDR_DRV_ADC1_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_ADC1_START_ADDR + KDP_DDR_DRV_ADC1_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_ADC1_ENDP1_ADDR', '(KDP_DDR_DRV_ADC1_START_ADDR + KDP_DDR_DRV_ADC1_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_ADC2_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_ADC2_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : ADC2 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_ADC2_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC2_RESERVED', KDP_DDR_DRV_ADC2_RESERVED)
            KDP_DDR_DRV_ADC2_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC2_START_ADDR', KDP_DDR_DRV_ADC2_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_ADC2_START_ADDR + KDP_DDR_DRV_ADC2_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_ADC2_ENDP1_ADDR', '(KDP_DDR_DRV_ADC2_START_ADDR + KDP_DDR_DRV_ADC2_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_ADC3_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_ADC3_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : ADC3 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_ADC3_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC3_RESERVED', KDP_DDR_DRV_ADC3_RESERVED)
            KDP_DDR_DRV_ADC3_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_ADC3_START_ADDR', KDP_DDR_DRV_ADC3_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_ADC3_START_ADDR + KDP_DDR_DRV_ADC3_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_ADC3_ENDP1_ADDR', '(KDP_DDR_DRV_ADC3_START_ADDR + KDP_DDR_DRV_ADC3_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM1_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM1_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM1 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM1_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM1_RESERVED', KDP_DDR_DRV_PWM1_RESERVED)
            KDP_DDR_DRV_PWM1_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM1_START_ADDR', KDP_DDR_DRV_PWM1_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM1_START_ADDR + KDP_DDR_DRV_PWM1_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM1_ENDP1_ADDR', '(KDP_DDR_DRV_PWM1_START_ADDR + KDP_DDR_DRV_PWM1_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM2_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM2_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM2 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM2_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM2_RESERVED', KDP_DDR_DRV_PWM2_RESERVED)
            KDP_DDR_DRV_PWM2_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM2_START_ADDR', KDP_DDR_DRV_PWM2_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM2_START_ADDR + KDP_DDR_DRV_PWM2_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM2_ENDP1_ADDR', '(KDP_DDR_DRV_PWM2_START_ADDR + KDP_DDR_DRV_PWM2_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM3_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM3_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM3 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM3_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM3_RESERVED', KDP_DDR_DRV_PWM3_RESERVED)
            KDP_DDR_DRV_PWM3_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM3_START_ADDR', KDP_DDR_DRV_PWM3_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM3_START_ADDR + KDP_DDR_DRV_PWM3_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM3_ENDP1_ADDR', '(KDP_DDR_DRV_PWM3_START_ADDR + KDP_DDR_DRV_PWM3_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM4_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM4_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM4 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM4_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM4_RESERVED', KDP_DDR_DRV_PWM4_RESERVED)
            KDP_DDR_DRV_PWM4_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM4_START_ADDR', KDP_DDR_DRV_PWM4_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM4_START_ADDR + KDP_DDR_DRV_PWM4_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM4_ENDP1_ADDR', '(KDP_DDR_DRV_PWM4_START_ADDR + KDP_DDR_DRV_PWM4_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM5_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM5_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM5 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM5_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM5_RESERVED', KDP_DDR_DRV_PWM5_RESERVED)
            KDP_DDR_DRV_PWM5_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM5_START_ADDR', KDP_DDR_DRV_PWM5_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM5_START_ADDR + KDP_DDR_DRV_PWM5_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM5_ENDP1_ADDR', '(KDP_DDR_DRV_PWM5_START_ADDR + KDP_DDR_DRV_PWM5_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_PWM6_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_PWM6_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @driver section : PWM6 */\n')
                board_gen_write_file_s___(header_file, '#if CFG_PWM6_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM6_RESERVED', KDP_DDR_DRV_PWM6_RESERVED)
            KDP_DDR_DRV_PWM6_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_PWM6_START_ADDR', KDP_DDR_DRV_PWM6_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_PWM6_START_ADDR + KDP_DDR_DRV_PWM6_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_PWM6_ENDP1_ADDR', '(KDP_DDR_DRV_PWM6_START_ADDR + KDP_DDR_DRV_PWM6_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_SSP0_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SSP0_TX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SSP0_TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SSP0_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP0_TX_RESERVED', KDP_DDR_DRV_SSP0_TX_RESERVED)
            KDP_DDR_DRV_SSP0_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP0_TX_START_ADDR', KDP_DDR_DRV_SSP0_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SSP0_TX_START_ADDR + KDP_DDR_DRV_SSP0_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SSP0_TX_ENDP1_ADDR', '(KDP_DDR_DRV_SSP0_TX_START_ADDR + KDP_DDR_DRV_SSP0_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_SSP0_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SSP0_RX_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SSP0_TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SSP0_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP0_RX_RESERVED', KDP_DDR_DRV_SSP0_RX_RESERVED)
            KDP_DDR_DRV_SSP0_RX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP0_RX_START_ADDR', KDP_DDR_DRV_SSP0_RX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SSP0_RX_START_ADDR + KDP_DDR_DRV_SSP0_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SSP0_RX_ENDP1_ADDR', '(KDP_DDR_DRV_SSP0_RX_START_ADDR + KDP_DDR_DRV_SSP0_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_SSP1_TX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SSP1_TX_RESERVED = 0x00001400
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SSP1_TX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SSP1_TX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP1_TX_RESERVED', KDP_DDR_DRV_SSP1_TX_RESERVED)
            KDP_DDR_DRV_SSP1_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP1_TX_START_ADDR', KDP_DDR_DRV_SSP1_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SSP1_TX_START_ADDR + KDP_DDR_DRV_SSP1_TX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SSP1_TX_ENDP1_ADDR', '(KDP_DDR_DRV_SSP1_TX_START_ADDR + KDP_DDR_DRV_SSP1_TX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_SSP1_RX_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SSP1_RX_RESERVED = 0x00001400
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SSP1_RX */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SSP1_RX_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP1_RX_RESERVED', KDP_DDR_DRV_SSP1_RX_RESERVED)
            KDP_DDR_DRV_SSP1_RX0_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP1_RX0_START_ADDR', KDP_DDR_DRV_SSP1_RX0_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SSP1_RX0_START_ADDR + KDP_DDR_DRV_SSP1_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SSP1_RX0_ENDP1_ADDR', '(KDP_DDR_DRV_SSP1_RX0_START_ADDR + KDP_DDR_DRV_SSP1_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
            KDP_DDR_DRV_SSP1_RX1_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SSP1_RX1_START_ADDR', KDP_DDR_DRV_SSP1_RX1_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SSP1_RX1_START_ADDR + KDP_DDR_DRV_SSP1_RX_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SSP1_RX1_ENDP1_ADDR', '(KDP_DDR_DRV_SSP1_RX1_START_ADDR + KDP_DDR_DRV_SSP1_RX_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
            if header_file is not None:
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_COM_BUS_TYPE':
            KDP_DDR_DRV_COM_BUS_RESERVED = 0x00001400
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @Communication bus buffer : COM_BUS_xx */\n')
                board_gen_write_file_s___(header_file, '#if CFG_COM_BUS_TYPE >= 0\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_COM_BUS_RESERVED', KDP_DDR_DRV_COM_BUS_RESERVED)
            KDP_DDR_DRV_COM_BUS_TX_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_COM_BUS_TX_START_ADDR', KDP_DDR_DRV_COM_BUS_TX_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_COM_BUS_TX_START_ADDR+ KDP_DDR_DRV_COM_BUS_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_COM_BUS_TX_ENDP1_ADDR', '(KDP_DDR_DRV_COM_BUS_TX_START_ADDR + KDP_DDR_DRV_COM_BUS_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
            KDP_DDR_DRV_COM_BUS_RX0_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_COM_BUS_RX0_START_ADDR', KDP_DDR_DRV_COM_BUS_RX0_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_COM_BUS_RX0_START_ADDR+ KDP_DDR_DRV_COM_BUS_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_COM_BUS_RX0_ENDP1_ADDR', '(KDP_DDR_DRV_COM_BUS_RX0_START_ADDR + KDP_DDR_DRV_COM_BUS_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
            KDP_DDR_DRV_COM_BUS_RX1_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_COM_BUS_RX1_START_ADDR', KDP_DDR_DRV_COM_BUS_RX1_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_COM_BUS_RX1_START_ADDR+ KDP_DDR_DRV_COM_BUS_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_COM_BUS_RX1_ENDP1_ADDR', '(KDP_DDR_DRV_COM_BUS_RX1_START_ADDR + KDP_DDR_DRV_COM_BUS_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
            if header_file is not None:
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_SPI_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SPI_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SPI */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SPI_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SPI_RESERVED', KDP_DDR_DRV_SPI_RESERVED)
            KDP_DDR_DRV_SPI_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SPI_START_ADDR', KDP_DDR_DRV_SPI_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SPI_START_ADDR + KDP_DDR_DRV_SPI_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SPI_ENDP1_ADDR', '(KDP_DDR_DRV_SPI_START_ADDR + KDP_DDR_DRV_SPI_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_SD_DMA_ENABLE' and value == '1':
            KDP_DDR_DRV_SD_RESERVED = 0x00000800
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : SD */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SD_DMA_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SD_RESERVED', KDP_DDR_DRV_SD_RESERVED)
            KDP_DDR_DRV_SD_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SD_START_ADDR', KDP_DDR_DRV_SD_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SD_START_ADDR + KDP_DDR_DRV_SD_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SD_ENDP1_ADDR', '(KDP_DDR_DRV_SD_START_ADDR + KDP_DDR_DRV_SD_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_USB_OTG_ENABLE' and value == '1':
            KDP_DDR_NEXT_START_ADDR = (KDP_DDR_NEXT_START_ADDR + 0x1000 - 1) & 0xfffff000
            KDP_DDR_DRV_USB_OTG_RESERVED = 0x00013000
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @High speed driver section : USB-OTG */\n')
                board_gen_write_file_s___(header_file, '#if CFG_USB_OTG_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_USB_OTG_RESERVED', KDP_DDR_DRV_USB_OTG_RESERVED)
            KDP_DDR_DRV_USB_OTG_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_USB_OTG_START_ADDR', KDP_DDR_DRV_USB_OTG_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_USB_OTG_START_ADDR + KDP_DDR_DRV_USB_OTG_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_USB_OTG_ENDP1_ADDR', '(KDP_DDR_DRV_USB_OTG_START_ADDR + KDP_DDR_DRV_USB_OTG_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_AI_USE_FIXED_IMG':
            #CFG_AI_USE_FIXED_IMG = 1
            #KDP_DDR_TEST_RGB_IMG_S_SIZE = 0x96000
            if header_file is not None:
                board_gen_write_file_s___(header_file, '///////////////////////////\n')
                board_gen_write_file_s___(header_file, '/* ddr - testing section */\n')
                board_gen_write_file_s___(header_file, '///////////////////////////\n')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_RGB_IMG_SIZE', KDP_DDR_TEST_RGB_IMG_SIZE, ' // RGB565')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_NIR_IMG_SIZE', KDP_DDR_TEST_NIR_IMG_SIZE, ' // RAW8')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_INF_IMG_SIZE', KDP_DDR_TEST_INF_IMG_SIZE, ' // img info')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_IMG_NAME_SIZE', KDP_DDR_TEST_IMG_NAME_SIZE, ' // img info')
            #header_file.write('#define KDP_DDR_TEST_RGB_IMG_S_SIZE     0x25800 //240x320x2(RGB565)\n')
            KDP_DDR_TEST_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_TEST_START_ADDR', KDP_DDR_TEST_START_ADDR)
            KDP_DDR_TEST_RGB_IMG_ADDR = KDP_DDR_TEST_START_ADDR
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_RGB_IMG_ADDR', 'KDP_DDR_TEST_START_ADDR //', KDP_DDR_TEST_RGB_IMG_ADDR)
            KDP_DDR_TEST_NIR_IMG_ADDR = KDP_DDR_TEST_RGB_IMG_ADDR + KDP_DDR_TEST_RGB_IMG_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_NIR_IMG_ADDR', '(KDP_DDR_TEST_RGB_IMG_ADDR + KDP_DDR_TEST_RGB_IMG_SIZE) //', KDP_DDR_TEST_NIR_IMG_ADDR)
            KDP_DDR_TEST_INF_IMG_ADDR = KDP_DDR_TEST_NIR_IMG_ADDR + KDP_DDR_TEST_NIR_IMG_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_INF_IMG_ADDR', '(KDP_DDR_TEST_NIR_IMG_ADDR + KDP_DDR_TEST_NIR_IMG_SIZE) //', KDP_DDR_TEST_INF_IMG_ADDR)
            KDP_DDR_TEST_IMG_NAME_ADDR = KDP_DDR_TEST_INF_IMG_ADDR + KDP_DDR_TEST_INF_IMG_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_IMG_NAME_ADDR', '(KDP_DDR_TEST_INF_IMG_ADDR + KDP_DDR_TEST_INF_IMG_SIZE) //', KDP_DDR_TEST_INF_IMG_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_TEST_IMG_NAME_ADDR + KDP_DDR_TEST_IMG_NAME_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_ENDP1_ADDR', '(KDP_DDR_TEST_IMG_NAME_ADDR + KDP_DDR_TEST_IMG_NAME_SIZE) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#if CFG_AI_USE_FIXED_IMG == YES\n')
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_AI_RGB_SRC_ADDR', 'KDP_DDR_TEST_RGB_IMG_ADDR //', KDP_DDR_TEST_RGB_IMG_ADDR)
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_AI_NIR_SRC_ADDR', 'KDP_DDR_TEST_NIR_IMG_ADDR //', KDP_DDR_TEST_NIR_IMG_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
                board_gen_write_file_s___(header_file, '\n')
        elif cfg_item_name == 'CFG_SNAPSHOT_ENABLE':
            CFG_SNAPSHOT_ENABLE = 2
            KDP_DDR_DRV_SNAPSHOT_RESERVED = 0x00026000
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @snapshot driver section */\n')
                board_gen_write_file_s___(header_file, '#if CFG_SNAPSHOT_ENABLE == 2\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SNAPSHOT_RESERVED', KDP_DDR_DRV_SNAPSHOT_RESERVED)
            KDP_DDR_DRV_SNAPSHOT_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_DRV_SNAPSHOT_START_ADDR', KDP_DDR_DRV_SNAPSHOT_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_DRV_SNAPSHOT_START_ADDR + KDP_DDR_DRV_SNAPSHOT_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_DRV_SNAPSHOT_ENDP1_ADDR', '(KDP_DDR_DRV_SNAPSHOT_START_ADDR + KDP_DDR_DRV_SNAPSHOT_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_SNAPSHOT_NUMS':
            CFG_SNAPSHOT_NUMS = int(value)
            #print(CFG_SNAPSHOT_NUMS)
        elif cfg_item_name == 'CFG_SNAPSHOT_ADVANCED':
            CFG_SNAPSHOT_ADVANCED = int(value)
            #print(CFG_SNAPSHOT_ADVANCED)
        elif cfg_item_name == 'CFG_UI_USR_IMG':
            if value == '1':
                CFG_UI_USR_IMG = 1
            else:
                CFG_UI_USR_IMG = 0
        elif cfg_item_name == 'CFG_OTA_FLASH_BUF_ENABLE' and value == '1':
            KDP_DDR_OTA_FLASH_BUF_RESERVED = 0x00002000
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @Flash controller use : FLASH */\n')
                board_gen_write_file_s___(header_file, '#if CFG_OTA_FLASH_BUF_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_OTA_FLASH_BUF_RESERVED', KDP_DDR_OTA_FLASH_BUF_RESERVED)
            KDP_DDR_OTA_FLASH_BUF_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_OTA_FLASH_BUF_START_ADDR', KDP_DDR_OTA_FLASH_BUF_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_OTA_FLASH_BUF_START_ADDR + KDP_DDR_OTA_FLASH_BUF_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_OTA_FLASH_BUF_ENDP1_ADDR', '(KDP_DDR_OTA_FLASH_BUF_START_ADDR + KDP_DDR_OTA_FLASH_BUF_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        elif cfg_item_name == 'CFG_OTA_IMAGE_BUF_ENABLE' and value == '1':
            KDP_DDR_OTA_IMAGE_BUF_RESERVED = 0x00032000
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @IMAGE DDR : FLASH */\n')
                board_gen_write_file_s___(header_file, '#if CFG_OTA_IMAGE_BUF_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_OTA_IMAGE_BUF_RESERVED', KDP_DDR_OTA_IMAGE_BUF_RESERVED)
            KDP_DDR_OTA_IMAGE_BUF_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_OTA_IMAGE_BUF_START_ADDR', KDP_DDR_OTA_IMAGE_BUF_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_OTA_IMAGE_BUF_START_ADDR + KDP_DDR_OTA_IMAGE_BUF_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_OTA_IMAGE_BUF_ENDP1_ADDR', '(KDP_DDR_OTA_IMAGE_BUF_START_ADDR + KDP_DDR_OTA_IMAGE_BUF_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')

        # keep settings section at the last of kdp ddr region
        elif cfg_item_name == 'CFG_KDP_SETTINGS_ENABLE' and value == '1':
            CFG_KDP_SETTINGS_ENABLE = 1

        elif cfg_item_name == 'CFG_KDP_SETTINGS_SIZE' and CFG_KDP_SETTINGS_ENABLE == 1:
            CFG_KDP_SETTINGS_SIZE = int(value)
            KDP_DDR_SETTINGS_RESERVED = CFG_KDP_SETTINGS_SIZE
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @kdp settings section */\n')
                board_gen_write_file_s___(header_file, '#if CFG_KDP_SETTINGS_ENABLE == YES\n')
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_SETTINGS_RESERVED', KDP_DDR_SETTINGS_RESERVED)
            KDP_DDR_SETTINGS_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_SETTINGS_START_ADDR', KDP_DDR_SETTINGS_START_ADDR)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_SETTINGS_START_ADDR + KDP_DDR_SETTINGS_RESERVED
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_SETTINGS_ENDP1_ADDR', '(KDP_DDR_SETTINGS_START_ADDR + KDP_DDR_SETTINGS_RESERVED) //', KDP_DDR_NEXT_START_ADDR)
                board_gen_write_file_s___(header_file, '#endif\n')
        elif cfg_item_name == 'CFG_USR_SETTINGS_ENABLE' and value == '1':
            CFG_USR_SETTINGS_ENABLE = 1
        elif cfg_item_name == 'CFG_USR_SETTINGS_SIZE' and CFG_USR_SETTINGS_ENABLE == 1:
            CFG_USR_SETTINGS_SIZE = int(value)
        #elif cfg_item_name = 'CFG_COM_BUS_TYPE':
        #    CFG_COM_BUS_TYPE = int(value)
        elif cfg_item_name =='CFG_OTA_FLASH_SLAVE_ENABLE' and value == '1':
            CFG_OTA_FLASH_SLAVE_ENABLE = 1
        elif cfg_item_name =='FLASH_VENDOR_SELECT': #and value =='GD25S512MD':
            CFG_FLASH_VENDOR = value
            jlink_gen()
        elif cfg_item_name == 'CFG_FMAP_EXTRA_ENABLE':
            CFG_FMAP_EXTRA_ENABLE = value
            if header_file is not None:
                board_gen_write_file_s___(header_file, '\n')
                board_gen_write_file_s___(header_file, '/* @extraction driver section */\n')
                #board_gen_write_file_s___(header_file, '#if (CFG_FMAP_EXTRA_ENABLE == YES) \n')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_RGB_FR_SIZE', KDP_DDR_TEST_RGB_FR_SIZE, ' // RGB')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_NIR_FR_SIZE', KDP_DDR_TEST_NIR_FR_SIZE, ' // NIR')
                board_gen_write_file_shs_(header_file, '#define KDP_DDR_TEST_USER_DB_SIZE', KDP_DDR_TEST_USER_DB_SIZE, ' // USER_DB')
            KDP_DDR_TEST_EXTRA_START_ADDR = KDP_DDR_NEXT_START_ADDR
            if header_file is not None:
                board_gen_write_file_sh__(header_file, '#define KDP_DDR_TEST_EXTRA_START_ADDR', KDP_DDR_TEST_EXTRA_START_ADDR)
            KDP_DDR_TEST_EXTRA_RGB_ADDR = KDP_DDR_TEST_EXTRA_START_ADDR
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_EXTRA_RGB_ADDR', 'KDP_DDR_TEST_EXTRA_START_ADDR //', KDP_DDR_TEST_EXTRA_RGB_ADDR + KDP_DDR_TEST_RGB_FR_SIZE)
            KDP_DDR_TEST_EXTRA_NIR_ADDR = KDP_DDR_TEST_EXTRA_RGB_ADDR + KDP_DDR_TEST_RGB_FR_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_EXTRA_NIR_ADDR', '(KDP_DDR_TEST_EXTRA_RGB_ADDR + KDP_DDR_TEST_RGB_FR_SIZE) //', KDP_DDR_TEST_EXTRA_NIR_ADDR + KDP_DDR_TEST_NIR_FR_SIZE)
            KDP_DDR_TEST_EXTRA_DB_ADDR = KDP_DDR_TEST_EXTRA_NIR_ADDR + KDP_DDR_TEST_NIR_FR_SIZE
            if header_file is not None:
                board_gen_write_file_ssh_(header_file, '#define KDP_DDR_TEST_EXTRA_DB_ADDR', '(KDP_DDR_TEST_EXTRA_NIR_ADDR + KDP_DDR_TEST_NIR_FR_SIZE) //', KDP_DDR_TEST_EXTRA_DB_ADDR + KDP_DDR_TEST_USER_DB_SIZE)
            KDP_DDR_NEXT_START_ADDR = KDP_DDR_TEST_EXTRA_DB_ADDR + KDP_DDR_TEST_USER_DB_SIZE
            if header_file is not None:
                #board_gen_write_file_s___(header_file, '#endif\n')
                board_gen_write_file_s___(header_file, '\n')
        elif cfg_item_name == 'IMAGE_SIZE':
            if value == 'IMAGE_16MB':
                IMAGE_SIZE = 16
            elif value == 'IMAGE_32MB':
                IMAGE_SIZE = 32
            elif value == 'IMAGE_64MB':
                IMAGE_SIZE = 64


    KDP_DDR_LAST_ADDR = KDP_DDR_NEXT_START_ADDR
    #print('KDP_DDR_LAST_ADDR=',KDP_DDR_LAST_ADDR)
    if header_file is not None:
        board_gen_write_file_s___(header_file, '\n')
        board_gen_write_file_sh__(header_file, '#define KDP_DDR_LAST_ADDR', KDP_DDR_LAST_ADDR)
        board_gen_write_file_s___(header_file, '\n')
        board_gen_write_file_s___(header_file, '#endif\n')

    return KDP_DDR_LAST_ADDR


def _convert_img2bin(img, bin_name):

    img = img.astype(np.uint16)
    (B, G, R) = cv2.split(img)
    img = np.zeros(B.shape, dtype = "uint16")
    img = (((R >> 3) & 0x1F) << 11) + (((G >> 2) & 0x3F) << 5) + ((B >> 3) & 0x1F)
    data = img.tobytes() # turn image data to bin
	# save bin file
    img_fo = open(bin_name, "wb")
    img_fo.write(data)
    img_fo.close()

def _update_words_py(file_path, addr):
    if os.path.isfile(file_path):
        global KDP_DDR_TEST_RGB_IMG_ADDR
        global KDP_DDR_TEST_NIR_IMG_ADDR
        with open(file_path, 'r') as f_r, open('%s.bak' %file_path, 'w', encoding='utf-8') as f_w:
            datafile = f_r.readlines()
            for line in datafile:
                if "'DDR_FREE'" in line:
                    f_w.write(f"    'DDR_FREE':   0x{addr:X},\n")
                elif "'RGB_IMG'" in line:
                    f_w.write(f"    'RGB_IMG' :   0x{KDP_DDR_TEST_RGB_IMG_ADDR:X},\n")
                elif "'NIR_IMG'" in line:
                    f_w.write(f"    'NIR_IMG' :   0x{KDP_DDR_TEST_NIR_IMG_ADDR:X},\n")
                elif "'INF_IMG'" in line:
                    f_w.write(f"    'INF_IMG' :   0x{KDP_DDR_TEST_INF_IMG_ADDR:X},\n")
                else:
                    f_w.write(line)
        os.remove(file_path)
        os.rename('%s.bak' %file_path, file_path)


def board_gen_usr_bin(build_name, cfg_ui_enable, cfg_ui_usr_img, cfg_usr_settings_enable, gen_h_file, dst_path, ddr_last_addr, flash_last_addr):

    user_flash_img_table_file = None
    user_ddr_img_table_file = None
    flash_total_size = 0
    ddr_total_size = 0
    comment_list=[]

    if gen_h_file == True:
        if user_flash_img_table_file is None:
            #user_flash_img_table_file = open(USER_FLASH_IMG_TABLE_FILE_NAME, 'w', newline='\n')
            user_flash_img_table_file = open(os.path.join(dst_path, USER_FLASH_IMG_TABLE_FILE_NAME), 'w', newline='\n')
            user_flash_img_table_file.write('#ifndef __USR_FLASH_IMG_TABLE_H__' + '\n')
            user_flash_img_table_file.write('#define __USR_FLASH_IMG_TABLE_H__' + '\n\n\n')
        if user_ddr_img_table_file is None:
            user_ddr_img_table_file = open(os.path.join(dst_path, USER_DDR_IMG_TABLE_FILE_NAME), 'w', newline='\n')
            #user_ddr_img_table_file = open(USER_DDR_IMG_TABLE_FILE_NAME, 'w', newline='\n')
            user_ddr_img_table_file.write('#ifndef __USR_DDR_IMG_TABLE_H__' + '\n')
            user_ddr_img_table_file.write('#define __USR_DDR_IMG_TABLE_H__' + '\n\n\n')

        comment_list.append('\n/*\n')
        comment_list.append('//Copy the content of this annotation to kl520_api_ddr_img_user()')

    if cfg_usr_settings_enable == 1:
        base_name = 'usr_settings'
        bin_file_name = os.path.join('flash_bin', base_name)
        bin_file_name = os.path.splitext(bin_file_name)[0]+'.bin'
        bin_file_size = CFG_USR_SETTINGS_SIZE

        #print('user_ddr_img_table_file=',user_ddr_img_table_file)
        if user_ddr_img_table_file is not None:
            tmp_ddr_addr = ddr_last_addr + ddr_total_size
            base_up_name = base_name.upper()
            tmp = f'#define USR_DDR_SETTINGS_ADDR'
            tmp2 = f'(KDP_DDR_LAST_ADDR + 0x{ddr_total_size:08X}) // 0x{tmp_ddr_addr:08X}, file={bin_file_name:s}'
            str = f'{tmp:<60s}{tmp2:s}\n'
            user_ddr_img_table_file.write(str)

            tmp = f'#define USR_DDR_SETTINGS_SIZE'
            tmp2 = f'0x{bin_file_size:08X}'
            str = f'{tmp:<60s}{tmp2:s}\n'
            user_ddr_img_table_file.write(str)

        ddr_total_size = ddr_total_size + bin_file_size

        #print('user_flash_img_table_file=',user_flash_img_table_file)
        if user_flash_img_table_file is not None:
            tmp_flash_addr = flash_last_addr + flash_total_size
            base_up_name = base_name.upper()
            tmp = f'#define USR_FLASH_SETTINGS_ADDR'
            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                tmp2 = f'(KDP_FLASH_USER_ADDR + 0x{flash_total_size:08X}) // 0x{tmp_flash_addr:08X}, file={bin_file_name:s}'
            else:    
                tmp2 = f'(KDP_FLASH_LAST_ADDR + 0x{flash_total_size:08X}) // 0x{tmp_flash_addr:08X}, file={bin_file_name:s}'
            str = f'{tmp:<60s}{tmp2:s}\n'
            user_flash_img_table_file.write(str)

            tmp = f'#define USR_FLASH_SETTINGS_SIZE'
            tmp2 = f'0x{bin_file_size:08X}'
            str = f'{tmp:<60s}{tmp2:s}\n'
            user_flash_img_table_file.write(str)

            # tmp = '#define USR_FLASH_SETTINGS_ADDR'
            # str = '%-43s (KDP_FLASH_LAST_ADDR + 0x%08X) // 0x%08X, file=%s' %( \
            # tmp, flash_total_size, flash_last_addr + flash_total_size, bin_file_name)
            # user_flash_img_table_file.write(str + '\n')

            # tmp = '#define USR_FLASH_SETTINGS_SIZE'
            # str = '%-43s 0x%08X' %(tmp, bin_file_size)
            # user_flash_img_table_file.write(str + '\n')

        flash_fdfr_bin_info.append([flash_last_addr + flash_total_size, bin_file_size, bin_file_name.upper(), bin_file_name])
        usr_bin.append([flash_total_size, bin_file_size, bin_file_name.upper(), bin_file_name])
        flash_total_size = flash_total_size + bin_file_size

        if (flash_total_size % 4096) > 0:
            remainder = 4096 - (flash_total_size % 4096)
            flash_fdfr_bin_info.append([flash_last_addr + flash_total_size, remainder, 'PADDING', 'padding'])
            usr_bin.append([flash_total_size, remainder, 'PADDING', 'padding'])
            flash_total_size = flash_total_size + remainder

        comment_1 = 'USR_DDR_SETTINGS_ADDR'
        comment_2 = 'USR_FLASH_SETTINGS_ADDR'
        comment_3 = 'USR_FLASH_SETTINGS_SIZE'
        comment_list.append('kdp_memxfer_flash_to_ddr(%s, %s, %s);' %(comment_1, comment_2, comment_3))

    if cfg_ui_enable == 1:
        if cfg_ui_usr_img == 1:
            path = f'./img_{build_name:s}'
            if os.path.isdir(path) is False:
                path = f'./img_default'
        else:
            path = f'./img_default'

        if os.path.isdir(path):
            file_list=[]
            for root, dirs, files in os.walk(path):
                for name in files:
                    base_name = os.path.splitext(os.path.basename(name))[0]
                    file_name = os.path.join(root, name)
                    img = cv2.imread(file_name)
                    if img is not None:
                        file_list.append(file_name)
                        base, ext = os.path.splitext(file_name)

                        bin_file_name = os.path.basename(file_name)
                        bin_file_name = os.path.join('flash_bin', bin_file_name)
                        bin_file_name = os.path.splitext(bin_file_name)[0]+'.bin'

                        _convert_img2bin(img, bin_file_name)

                        bin_file_size = os.path.getsize(bin_file_name)
                        bin_file_mod = bin_file_size%4
                        if 0 != bin_file_mod:
                            bin_file_size = bin_file_size + 4 - bin_file_mod

                        if user_ddr_img_table_file is not None:
                            tmp_ddr_addr = ddr_last_addr + ddr_total_size
                            base_up_name = base_name.upper()
                            tmp = f'#define USR_DDR_IMG_{base_up_name:s}_ADDR'
                            tmp2 = f'(KDP_DDR_LAST_ADDR + 0x{ddr_total_size:08X}) // 0x{tmp_ddr_addr:08X}, file={bin_file_name:s}'
                            str = f'{tmp:<60s}{tmp2:s}\n'
                            user_ddr_img_table_file.write(str)

                            tmp = f'#define USR_DDR_IMG_{base_up_name:s}_SIZE'
                            tmp2 = f'0x{bin_file_size:08X}'
                            str = f'{tmp:<60s}{tmp2:s}\n'
                            user_ddr_img_table_file.write(str)

                        ddr_total_size = ddr_total_size + bin_file_size

                        if user_flash_img_table_file is not None:
                            tmp_flash_addr = flash_last_addr + flash_total_size
                            base_up_name = base_name.upper()
                            tmp = f'#define USR_FLASH_IMG_{base_up_name:s}_ADDR'
                            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                                tmp2 = f'(KDP_FLASH_USER_ADDR + 0x{flash_total_size:08X}) // 0x{tmp_flash_addr:08X}, file={bin_file_name:s}'
                            else:    
                                tmp2 = f'(KDP_FLASH_LAST_ADDR + 0x{flash_total_size:08X}) // 0x{tmp_flash_addr:08X}, file={bin_file_name:s}'
                            str = f'{tmp:<60s}{tmp2:s}\n'
                            user_flash_img_table_file.write(str)

                            tmp = f'#define USR_FLASH_IMG_{base_up_name:s}_SIZE'
                            tmp2 = f'0x{bin_file_size:08X}'
                            str = f'{tmp:<60s}{tmp2:s}\n'
                            user_flash_img_table_file.write(str)

                            # tmp = '#define USR_FLASH_IMG_%s_ADDR' %(base_name.upper())
                            # str = '%-43s (KDP_FLASH_LAST_ADDR + 0x%08X) // 0x%08X, file=%s' %( \
                            # tmp, flash_total_size, flash_last_addr + flash_total_size, bin_file_name)
                            # user_flash_img_table_file.write(str + '\n')

                            # tmp = '#define USR_FLASH_IMG_%s_SIZE' %(base_name.upper())
                            # str = '%-43s 0x%08X' %(tmp, bin_file_size)
                            # user_flash_img_table_file.write(str + '\n')

                        #addr = flash_last_addr + flash_total_size
                        #print(f'bin_file_name={bin_file_name:s}  addr={addr:8x}')
                        flash_fdfr_bin_info.append([flash_last_addr + flash_total_size, bin_file_size, bin_file_name.upper(), bin_file_name])
                        usr_bin.append([flash_total_size, bin_file_size, bin_file_name.upper(), bin_file_name])
                        flash_total_size = flash_total_size + bin_file_size

                        if (flash_total_size % 4096) > 0:
                            remainder = 4096 - (flash_total_size % 4096)
                            flash_fdfr_bin_info.append([flash_last_addr + flash_total_size, remainder, 'PADDING', 'padding'])
                            usr_bin.append([flash_total_size, remainder, 'PADDING', 'padding'])
                            flash_total_size = flash_total_size + remainder

                        comment_1 = 'USR_DDR_IMG_%s_ADDR' %(base_name.upper())
                        comment_2 = 'USR_FLASH_IMG_%s_ADDR' %(base_name.upper())
                        comment_3 = 'USR_FLASH_IMG_%s_SIZE' %(base_name.upper())
                        comment_list.append('kdp_memxfer_flash_to_ddr(%s, %s, %s);' %(comment_1, comment_2, comment_3))

    flash_last_addr = flash_last_addr + flash_total_size
    ddr_last_addr = ddr_last_addr + ddr_total_size
    _update_words_py(REGRESSION_UTILS_WORDS_PY_PATH, ddr_last_addr)

    if gen_h_file == True:
        comment_list.append('*/ \n')

        if user_flash_img_table_file:
            define_name = '#define USR_FLASH_LAST_ADDR'
            define_val = f'0x{flash_last_addr:08X}'
            user_flash_img_table_file.write(f'{define_name:<60s}{define_val:s}\n')

            user_flash_img_table_file.write('\n' + '#endif' + '\n')
            user_flash_img_table_file.close()

        if user_ddr_img_table_file:
            define_name = '#define USR_DDR_LAST_ADDR'
            define_val = f'0x{ddr_last_addr:08X}'
            user_ddr_img_table_file.write(f'{define_name:<60s}{define_val:s}\n')
            for item in comment_list:
                user_ddr_img_table_file.write('%s\n' %(item))

            define_name = '#define KDP_DDR_HEAP_HEAD_FOR_MALLOC_END'
            define_val = (ddr_last_addr + 15) & 0xFFFFFFF0
            user_ddr_img_table_file.write(f'{define_name:<60s}(0x{define_val:08X} - 1)\n')

            define_name = '#define KDP_DDR_HEAP_HEAD_FOR_MALLOC'
            define_val = 0x63FFFFFF
            user_ddr_img_table_file.write(f'{define_name:<60s}0x{define_val:08X}\n')

            user_ddr_img_table_file.write('\n' + '#endif' + '\n')
            user_ddr_img_table_file.close()

#        for f in flash_fdfr_bin_info:
#            print(f)
#        for f in usr_bin:
#            print(f)


###############################################################################
###############################################################################
###############################################################################
def file_read_binary(src):
     ''' Load depth file '''
     with open(src, "rb") as f:
         data = f.read()
         return data

def write_data_to_binary_file(ofile, data):
    with io.open(ofile, 'wb') as fo:
        fo.write(bytearray(data))


def board_gen_bin(bin_info, ofile):
    bin_len = len(bin_info)
    total_bin_size = bin_info[bin_len - 1][0] + bin_info[bin_len - 1][1]
    bin_data = np.zeros(total_bin_size, dtype=np.uint8)
    bin_data.fill(0xff)
    for bin_file in bin_info:
        #print('start parsing, start: 0x{:08X}, size: {:08X}, file={:s}, addr={:s}'.format(bin_file[0], bin_file[1], bin_file[2], bin_file[3]))
        bin_start_addr = bin_file[0]
        bin_size = bin_file[1]
        bin_file = bin_file[3]
        if os.path.isfile(bin_file) == 1:
            rdata = file_read_binary(bin_file)
            rdata = list(rdata)
            rdata = np.array(rdata)
            bin_data[bin_start_addr:bin_start_addr + len(rdata)] = rdata
        #else:
        #    print('{:s} is not exists'.format(bin_file))
    write_data_to_binary_file(ofile, bin_data)
    
    #for f in bin_info:
         #print('0x{:08x} {:<10d} {:<20s} {:s}\n'.format(f[0], f[1], f[2], f[3]))

def adjust_addr():
    global g_fw_info_addr
    global g_all_models_addr
    global file_bin_info

    flag = 0
    next_addr = 0
    for bin_file in flash_fdfr_bin_info:
        bin_file[0] = next_addr
        if bin_file[2] == 'FW_INFO':
            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                bin_file[1] = KDP_FLASH_FW_INFO_RESERVED
                g_fw_info_addr = bin_file[0]
            else:
                bin_file[1] = os.path.getsize(bin_file[3])
                g_fw_info_addr = bin_file[0]
            flag = 1
        elif bin_file[2] == 'ALL_MODELS':
            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                bin_file[1] = KDP_FLASH_MODEL_RESERVED
                g_all_models_addr = bin_file[0]
            else:
                bin_file[1] = os.path.getsize(bin_file[3])
                g_all_models_addr = bin_file[0]
            flag = 1
        elif bin_file[2] == 'RGB':
            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                bin_file[1] = 614400
            else:
                bin_file[1] = os.path.getsize(bin_file[3])
            flag = 1

        elif bin_file[2] == 'NIR':
            if CFG_OTA_FLASH_SLAVE_ENABLE == 1:
                bin_file[1] = 307200
            else:
                bin_file[1] = os.path.getsize(bin_file[3])    
            flag = 1

        next_addr = (bin_file[0] + bin_file[1] + 0x1000 - 1) & 0xfffff000
        #print('%#x'%bin_file[0], bin_file[1], bin_file[2], bin_file[3])

    with open(file_bin_info,'w') as file:
        for bin_file in flash_fdfr_bin_info:
            file.write('0x{:08x} {:<10d} {:<20s} {:s}\n'.format(bin_file[0], bin_file[1], bin_file[2], bin_file[3]))

def get_flash_bin_info():
    global file_bin_info
    global flash_fdfr_bin_info
    flash_fdfr_bin_info = []
    with open(file_bin_info,'r') as file:
        for line in file:
            flash_fdfr_bin_info.append(line.split())
    for bin_file in flash_fdfr_bin_info:
        if bin_file[0].startswith('0x'):
            bin_file[0] = int(bin_file[0], 16)
        else:
            bin_file[0] = int(bin_file[0])
        if bin_file[1].startswith('0x'):
            bin_file[1] = int(bin_file[1], 16)
        else:
            bin_file[1] = int(bin_file[1])
        #print('0x{:08x} {:<10d} {:<20s} {:s}\n'.format(bin_file[0], bin_file[1], bin_file[2], bin_file[3]))

BIN_FROM_PROJECT=0
SPL_CPU_ONLY=0


def alter_kdp_model(file_path, model_info_addr, model_pool_addr):

    #f = open('test.h', 'w', encoding="UTF-8")
    #print(f)
    str_info = 'MODEL_MGR_FLASH_ADDR_HEAD'
    str_pool = 'MODEL_MGR_FLASH_ADDR_MODEL_POOL'
    pattern_info = re.compile(r'#define {string}([\t ]*)(.*)'.format(string = str_info))
    pattern_pool = re.compile(r'#define {string}([\t ]*)(.*)'.format(string = str_pool))
    with open(file_path, 'r') as f_r, open('%s.bak' %file_path, 'w', encoding='utf-8') as f_w:
        for line in f_r:
            str = pattern_info.match(line)
            if str is not None:
                f_w.write(f'#define {str_info:40s}0x{model_info_addr:08X}\n')
                continue
            str = pattern_pool.match(line)
            if str is not None:
                f_w.write(f'#define {str_pool:40s}0x{model_pool_addr:08X}\n')
                continue
            f_w.write(line)
    os.remove(file_path)
    os.rename('%s.bak' %file_path, file_path)


if __name__ == "__main__":

    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s : %(message)s', filename='test_report.txt')
    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    logging.getLogger('').addHandler(console)

    parser = argparse.ArgumentParser()
    parser.add_argument('-s', help='system config is ?', dest='system_config')
    parser.add_argument('-t', help='target bin is ?', dest='target_bin')
    parser.add_argument('-i', '--START_ADDR', help='Set start address', default=0)
    parser.add_argument('-p', '--FLASH_SPECIFIC_PARTITION', help='Specific partition flash programming')
    parser.add_argument('-u', '--FLASH_USR_PARTITION', help='UI partition flash programming', action ='store_true')
    parser.add_argument('-a', '--FLASH_ALL', help='Flash all data programming')
    parser.add_argument('-w', '--MODE', help='board_gen_cfg is ?', default=1)
    parser.add_argument('-c', '--CONFIG', help='which config fileis used?', default='scpu')

    if SPL_CPU_ONLY == 1:
        flash_fdfr_bin_info = flash_fdfr_bin_info[:3]

        idx=0
        for bin_file in flash_fdfr_bin_info:
            if BIN_FROM_PROJECT == 1:
                if idx == 1:
                    bin_file[3] = '../../scpu/rtos2/Objects/fw_scpu.bin'
                elif idx == 2:
                    bin_file[3] = '../../ncpu/rtos2/Objects/fw_ncpu.bin'
                idx += 1
            #print('start addr: 0x{:08X}, size: {:08X}, file={:s}'.format(bin_file[0], bin_file[1], bin_file[2]))
        bin_gen(flash_fdfr_bin_info, 'flash_fdfr_image_cpuonly.bin')

    else:
        args = parser.parse_args()

        if (args.system_config):
            version_file = f'../../build/{args.system_config:s}/project/version.cfg'
            if not os.path.isfile(version_file):
                version_file = './version.cfg'
            version_info = version.version_class()
            version_info.get_version_info(version_file)
            # version_info.print_values()
            version_info.update_scpu_version(r'../../scpu/share/version.c')
            version_info.update_model_version(r'../../scpu/share/version.c')
            if args.CONFIG == 'ncpu':
                version_info.update_ncpu_version(r'../../ncpu/device/include/Kneron/version.h')

            dst_path = f'../../build/{args.system_config:s}/config/'
            if args.CONFIG == 'scpu_slave':
                cfg_file_path = f'../../build/{args.system_config:s}/project/board_slave.cfg'
                print('CONFIG is scpu_slave')
            else:
                cfg_file_path = f'../../build/{args.system_config:s}/project/board.cfg'
                print('CONFIG is scpu')
            board_gen_cfg(args.system_config, cfg_file_path)
            ddr_last_addr = board_gen_kdp_ddr(args.system_config, True, cfg_file_path)
            get_flash_bin_info()
            adjust_addr()
            flash_last_addr = board_gen_kdp_flash(args.system_config, True, flash_fdfr_bin_info)
            board_gen_usr_bin(args.system_config, CFG_UI_ENABLE, CFG_UI_USR_IMG, CFG_USR_SETTINGS_ENABLE, True, dst_path, ddr_last_addr, flash_last_addr)
            bin_gen(usr_bin, 'usr.bin')

            file_path = '../../scpu/lib/kdp_application/include/kdp_model.h'
            alter_kdp_model(file_path, g_fw_info_addr, g_all_models_addr)
            #board_gen_bin(flash_fdfr_bin_info, 'flash_fdfr_image.bin')

        if (args.target_bin):
            dst_path = f'../../build/{args.target_bin:s}/config/'
            cfg_file_path = f'../../build/{args.target_bin:s}/project/board.cfg'
            ddr_last_addr = board_gen_kdp_ddr(args.target_bin, False, cfg_file_path)
            get_flash_bin_info()
            adjust_addr()
            flash_last_addr = board_gen_kdp_flash(args.target_bin, False, flash_fdfr_bin_info)
            board_gen_usr_bin(args.target_bin, CFG_UI_ENABLE, CFG_UI_USR_IMG, CFG_USR_SETTINGS_ENABLE, False, dst_path, ddr_last_addr, flash_last_addr)
            board_gen_bin(flash_fdfr_bin_info, 'flash_fdfr_image.bin')
            ek_common.mycopyfile('./flash_bin/boot_spl.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('./flash_bin/fw_scpu.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('./flash_bin/fw_scpu1.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('./flash_bin/fw_ncpu.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('./flash_bin/all_models.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('./flash_bin/fw_info.bin', '../JLink_programmer/bin')
            ek_common.mycopyfile('flash_fdfr_image.bin', '../JLink_programmer/bin')
            #bin_gen(usr_bin, 'usr.bin')

        if (args.FLASH_SPECIFIC_PARTITION):
            setup.dev_init()
            print('Device init successfully')
            print(args.START_ADDR)
            fp.flash_specific_programming(args.START_ADDR, args.FLASH_SPECIFIC_PARTITION)

        elif (args.FLASH_USR_PARTITION):
            setup.dev_init()
            print('Device init successfully')
            print(ddr_last_addr)
            print(hex(ddr_last_addr))
            fp.flash_specific_programming(hex(ddr_last_addr), 'usr.bin')

        elif (args.FLASH_ALL):
            setup.dev_init()
            print('Device init successfully')
            print(args.START_ADDR)
            fp.flash_chip_programming(args.FLASH_ALL)

        if(args.MODE):
            filecfg = open("prebuild.cfg","w")
            filecfg.write("board_gen_mode=" + str(args.MODE))
            filecfg.close()

