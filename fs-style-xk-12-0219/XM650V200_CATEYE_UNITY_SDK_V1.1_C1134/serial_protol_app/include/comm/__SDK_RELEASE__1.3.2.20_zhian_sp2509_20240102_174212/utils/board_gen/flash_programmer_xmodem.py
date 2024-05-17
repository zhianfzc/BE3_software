import setup_xmodem
import random
import pdb
import time
import words
import time
import sys
import logging
import datetime
import argparse
import io
import os.path
import serial


###############################################################################
# Flash Read/Write/Erase test
###############################################################################
def flash_fw_info():
    print('Engineering mode: show debugging info')
    setup_xmodem.showdebuginfo()
    time.sleep(0.05)

def flash_verify(file, size = 0):
    logging.info('Flash: Start Verify')
    start_millis = int(round(time.time() * 1000))
    #print(start_millis)
    rdata = list(setup_xmodem.file_read_binary(file))
    if size == 0:
        size = len(rdata)
    rdata = rdata[0:size]
    fdata = setup_xmodem.flash_read(0, size)
    with io.open('flash_dump.bin', 'wb') as fo:
        fo.write(bytearray(fdata))
    if (rdata == fdata):
        logging.info('flash bin comparsion PASS')
    else:
        logging.info('ERR: flash bin file comparsion error')
    end_millis = int(round(time.time() * 1000))
    #print(end_millis)
    millis = end_millis - start_millis
    print("Total verify time",millis,"ms")
    print("Total verify time",millis/1000,"sec")


def flash_bin_program(file, size = 0):
    start_millis = int(round(time.time() * 1000))
    #print(start_millis)
    rdata = list(setup_xmodem.file_read_binary(file))
    if size == 0:
        size = len(rdata)
    setup_xmodem.flash_write(0, rdata, size, 4096)
    end_millis = int(round(time.time() * 1000))
    #print(end_millis)
    millis = end_millis - start_millis
    print("Total programming time",millis,"ms")
    print("Total programming time",millis/1000,"sec")

def flash_chip_erase():
    print('Flash chip erase, please wait until its done!')
    start_millis = int(round(time.time() * 1000))
    setup_xmodem.flash_chip_erase()
    for i in range(60):
        ret = setup_xmodem.erase_readline()
        if(ret == False):
            #print("\r"+'{:d}/60'.format(i),end = "",flush=True)
            print("\r" + '%s' % ('>'*i),end = "",flush=True)
        else:
            print("ERASE Done")
            break
        #time.sleep(0.5)
    #print("\r"+'{:d}/60'.format(60),end = "",flush=True)
    print("ERASE Done!!!")
    end_millis = int(round(time.time() * 1000))
    millis = end_millis - start_millis
    print("Total erasing time",millis,"ms")
    print("Total erasing time",millis/1000,"sec")

def flash_boot_sector_erase():
    print('Boot sector 0 (4k) erase, please wait(5s)')
    setup_xmodem.flash_sector_erase(0)
    for i in range(5):
        time.sleep(1)
        print('{:d}/5'.format(i))
    print('Boot sector 1 (4k) erase, please wait(5s)')
    setup_xmodem.flash_sector_erase(4096)
    for i in range(5):
        time.sleep(1)
        print('{:d}/5'.format(i))

def flash_fw_erase():
    for i in range(192//4):
        print('Boot sector %d (4k) erase' %(i * 4096))
        setup_xmodem.flash_sector_erase(i * 4096)
        time.sleep(0.5)


def flash_fw_programming(fw_bin):
    logging.info('=================================================================')
    logging.info('Flash: Sector Erase Start')
    flash_fw_erase();
    flash_programming(fw_bin, 192*1024)
    flash_verify(fw_bin, 192*1024)

def flash_spl_programming(spl_bin):
    logging.info('=================================================================')
    logging.info('Flash: SPL Sector Erase Start')
    flash_boot_sector_erase()
    flash_bin_program(spl_bin, 8*1024)
    flash_verify(spl_bin, 8*1024)

def flash_chip_programming(chip_bin):
    logging.info('=================================================================')
    logging.info('Flash: Chip Erase Start')
    flash_chip_erase()
    flash_programming(chip_bin)


def flash_programming(flash_bin, size = 0):
    logging.info('Flash: Start Programming')
    flash_bin_program(flash_bin, size)
    flash_verify(flash_bin, size)


def mem_test():
    ''' Memory read/write verification '''
    '''
        ###addr = 0x10100000 #SiRAM, can't test for FW code
        #addr = 0x10200000 #SdRAM
        #addr = 0x20200000 #NiRAM
        #addr = 0x28000000 #NdRAM
        #addr = 0x60000000 #DDR
    '''
    setup_xmodem.npu_reset()
    test_loop = 100
    test_len = 0x100
    mem = [words.__MEM_SCPU_ADDR__['SDRAM'],
           words.__MEM_SCPU_ADDR__['NIRAM'],
           words.__MEM_SCPU_ADDR__['NDRAM'],
           words.__MEM_SCPU_ADDR__['NPURAM'],
           words.__MEM_SCPU_ADDR__['DDR']]
    for addr in mem:
        print('--------------------------------------')
        print('Start ADDR=0x%X memory R/W test' %addr)
        for i in range(test_loop):
            wbuf = [random.randint(0x0, 0xFF) for i in range(test_len)]
            setup_xmodem.mem_block_write(addr, wbuf, len(wbuf))

            ''' Memory read '''
            rbuf = setup_xmodem.mem_block_read(addr, test_len)
            if (rbuf != wbuf):
                print('Memory read fail')
                print('=> Write dump')
                setup_xmodem.mem_dump(wbuf)
                print('=> Read dump')
                setup_xmodem.mem_dump(rbuf)
                logging.info('ERR: [%s] Memory Read/Write verify FAIL (%d/%d)' % (words.get_mem_name(words.__MEM_SCPU_ADDR__, addr), i+1, test_loop))
                sys.exit(-1)
        logging.info('[%s] Memory Read/Write verify PASS (%d/%d)' % (words.get_mem_name(words.__MEM_SCPU_ADDR__, addr), i+1, test_loop))




###############################################################################
###############################################################################
###############################################################################
def flash_specific_erase(idx):
    print('erase sector', idx)
    setup_xmodem.flash_sector_erase(idx * 4096)
    time.sleep(0.1)

def flash_specific_bin_program(addr, file, size = 0):
    rdata = list(setup_xmodem.file_read_binary(file))
    if size == 0:
        size = len(rdata)
    setup_xmodem.flash_write(addr, rdata, size, 4096)

def flash_specific_verify(add, file, size = 0):
    logging.info('Flash: Start Verify')
    rdata = list(setup_xmodem.file_read_binary(file))
    if size == 0:
        size = len(rdata)
    rdata = rdata[0:size]
    fdata = setup_xmodem.flash_read(add, size)
    print('fffffffff file=', file)
    print('ssssssssss size=', size)
    with io.open('flash_dump.bin', 'wb') as fo:
        fo.write(bytearray(fdata))
    if (rdata == fdata):
        logging.info('flash bin comparsion PASS')
    else:
        logging.info('ERR: flash bin file comparsion error')

def flash_specific_programming(start, fw_bin):
    logging.info('=================================================================')
    logging.info('Flash: (Specific) Sector Erase Start')
    start = int(start,16)
    start_idx = start // 4096
    #print('dddd =', os.path.exists(fw_bin))
    file_size = os.path.getsize(fw_bin) 
    sector_num = file_size // 4096
    remainder = file_size % 4096
    if remainder > 0:
        sector_num = sector_num + 1
        #print('aaaaa sector_num=', sector_num)
        #print('bbbbb =', file_size/4096)
    
    print('flash_specific_programming start_idx=%d sector_num=%d file=%s [file_size=%d]' %(start_idx, sector_num, fw_bin, file_size))
    for i in range(start_idx, start_idx + sector_num):
        flash_specific_erase(i)
    flash_specific_bin_program(start, fw_bin, file_size)
    flash_specific_verify(start, fw_bin, file_size)


if __name__ == "__main__":

    # set log file
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s : %(message)s', filename='test_report.txt')
    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    logging.getLogger('').addHandler(console)
    setup_xmodem.dev_init()
    print('Device init successfully')

    ret = setup_xmodem.xmodem_send_bin()
    print("xmodem_send bin file result = ",ret)
    #ret = 1
#check xmodem_send_bin() result, if its failed, stop fw update.
    if(ret):
        # arugment parser
        parser = argparse.ArgumentParser()
        parser.add_argument('-a', '--FLASH_ALL', help='Flash all data programming')
        parser.add_argument('-f', '--FLASH_FIRMWARE_PARTITION', help='Fimware partition flash programming')
        parser.add_argument('-s', '--FLASH_SPL_PARTITION', help='SPL flash programming')
        parser.add_argument('-v', '--FLASH_VERIFICATION', help='Flash verification')
        parser.add_argument('-e', '--FLASH_ERASE', action='store_true', help='Flash chip erase', default=False)
        parser.add_argument('-t', '--MEM_TEST', action='store_true', help='Mozart memory verification test', default=False)
        parser.add_argument('-d', '--FLASH_INFO', action='store_true', help='Show debugging information', default=False)
        parser.add_argument('-i', '--START_ADDR', help='Set start address', default=0)
        parser.add_argument('-p', '--FLASH_SPECIFIC_PARTITION', help='Specific partition flash programming')
    
    
        args = parser.parse_args()
        if (args.MEM_TEST):
            mem_test()
        if (args.FLASH_INFO):
            flash_fw_info()
        if (args.FLASH_ALL):
            flash_chip_programming(args.FLASH_ALL)
        elif (args.FLASH_FIRMWARE_PARTITION):
            flash_fw_programming(args.FLASH_FIRMWARE_PARTITION)
        elif (args.FLASH_SPL_PARTITION):
            flash_spl_programming(args.FLASH_SPL_PARTITION)
        elif (args.FLASH_VERIFICATION):
            flash_verify(args.FLASH_VERIFICATION)
        elif (args.FLASH_ERASE):
            flash_chip_erase()
        elif (args.FLASH_SPECIFIC_PARTITION):
            flash_specific_programming(args.START_ADDR, args.FLASH_SPECIFIC_PARTITION)
    else:
        print("RESET!!!")
        print("RESET then do it AGAIN!!!")

