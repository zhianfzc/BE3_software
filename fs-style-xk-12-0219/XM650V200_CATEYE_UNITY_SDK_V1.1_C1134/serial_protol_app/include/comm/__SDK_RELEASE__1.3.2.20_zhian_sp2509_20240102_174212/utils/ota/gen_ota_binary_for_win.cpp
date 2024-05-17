// gen_ota_binary_for_win.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "pch.h"
#include <iostream>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;


#define SCPU_IMAGE_SIZE             160*1024//0x16000
#define NCPU_IMAGE_SIZE             0x10000

#define FW_INFO_SIZE       0x1000
#define ALL_MODEL_SIZE     3576800





/*********************
	CRC function
**********************/
#define CRC16_CONSTANT 0x8005

const u32 crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


u32 crc32( u8 *buf, size_t size)
 {
	const u8 *p = buf;
	u32 crc;
	u32 target_size = size;
	
	crc = ~0U;
	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);	
	return crc ^ ~0U;
}




#define CRC16_CONSTANT 0x8005

static u16 gen_crc16(u8 *data, u32 size)
{
    u16 out = 0;
    int bits_read = 0, bit_flag, i;

    /* Sanity check: */
    if (data == NULL)
        return 0;

    while (size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

                                         /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag)
            out ^= CRC16_CONSTANT;

    }

    // push out the last 16 bits
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag)
            out ^= CRC16_CONSTANT;
    }

    // reverse the bits
    u16 crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>= 1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}

int gen_ota_scpu(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;
    u16 crc16_w;
	u32 crc32_w;
	FILE *pftemp_bin;
	u32 temp_binsize;
	u8  *temp_buf;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > SCPU_IMAGE_SIZE)
    {
        printf("Error: intput SCPU file is larger than 90112 bytes\n");
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(SCPU_IMAGE_SIZE);
    if (buf == NULL)
    {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen)
    {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        return 0;
    }
    fclose(pfIn);

    memset(buf + inFileLen, 0, SCPU_IMAGE_SIZE - inFileLen);
	
	//scpu crc
	crc32_w = crc32(buf, SCPU_IMAGE_SIZE - 20);
	*(u32 *)(buf + SCPU_IMAGE_SIZE - 20 ) = crc32_w;

	//open ncpu 
	#if 1
	pftemp_bin = fopen("..\\..\\..\\..\\utils\\board_gen\\flash_bin\\fw_ncpu.bin", "rb");
	if( pftemp_bin!= NULL)
	{
		fseek(pftemp_bin, 0L, SEEK_END);
		temp_binsize = ftell(pftemp_bin);
		rewind(pftemp_bin);
	
		//locate an area
		temp_buf = (u8 *)malloc(temp_binsize);
		result = fread(temp_buf, 1, temp_binsize, pftemp_bin);
		crc32_w = crc32(temp_buf, temp_binsize );
		//crc32_w = 0x78875AA5;			//OK line
		*(u32 *)(buf + SCPU_IMAGE_SIZE - 20 + 4 ) = crc32_w;
		free(temp_buf);
		fclose(pftemp_bin);		
	}
	//open fw_info 
	pftemp_bin = fopen("..\\..\\..\\..\\utils\\board_gen\\flash_bin\\fw_info.bin", "rb");
	if( pftemp_bin!= NULL)
	{
		fseek(pftemp_bin, 0L, SEEK_END);
		temp_binsize = ftell(pftemp_bin);
		rewind(pftemp_bin);
	
		//locate an area
		temp_buf = (u8 *)malloc(temp_binsize);
		result = fread(temp_buf, 1, temp_binsize, pftemp_bin);
		crc32_w = crc32(temp_buf, temp_binsize );
		//crc32_w = 0x78875AA5;			//OK line
		*(u32 *)(buf + SCPU_IMAGE_SIZE - 20 + 4*2 ) = crc32_w;
		free(temp_buf);
		fclose(pftemp_bin);		
	}


	//open all_model_bin
	pftemp_bin = fopen("..\\..\\..\\..\\utils\\board_gen\\flash_bin\\all_models.bin", "rb");
	if( pftemp_bin!= NULL)
	{
		fseek(pftemp_bin, 0L, SEEK_END);
		temp_binsize = ftell(pftemp_bin);
		rewind(pftemp_bin);
	
		//locate an area
		temp_buf = (u8 *)malloc(temp_binsize);
		result = fread(temp_buf, 1, temp_binsize, pftemp_bin);
		crc32_w = crc32(temp_buf, temp_binsize );
		//crc32_w = 0x78875AA5;			//OK line
		*(u32 *)(buf + SCPU_IMAGE_SIZE - 20 + 4*3 ) = crc32_w;
		free(temp_buf);
		fclose(pftemp_bin);		
	}


	//open user_bin
	pftemp_bin = fopen("..\\..\\..\\..\\utils\\board_gen\\usr.bin", "rb");
	if( pftemp_bin!= NULL)
	{
		fseek(pftemp_bin, 0L, SEEK_END);
		temp_binsize = ftell(pftemp_bin);
		rewind(pftemp_bin);
	
		//locate an area
		temp_buf = (u8 *)malloc(temp_binsize);
		result = fread(temp_buf, 1, temp_binsize, pftemp_bin);
		crc32_w = crc32(temp_buf, temp_binsize );
		//crc32_w = 0x78875AA5;			//OK line
		*(u32 *)(buf + SCPU_IMAGE_SIZE - 20 + 4*4 ) = crc32_w;
		free(temp_buf);
		fclose(pftemp_bin);		
	}
	#endif

    fwrite(buf, SCPU_IMAGE_SIZE, 1, pfOut);
    fclose(pfOut);
    
    return 0;
}

int gen_ota_ncpu(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;
    u16 crc16_w;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > NCPU_IMAGE_SIZE)
    {
        printf("Error: intput NCPU file is larger than 65536 bytes\n");
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(NCPU_IMAGE_SIZE);
    if (buf == NULL)
    {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen)
    {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        return 0;
    }

    fclose(pfIn);

    memset(buf + inFileLen, 0, NCPU_IMAGE_SIZE - inFileLen);

    crc16_w = gen_crc16(buf, NCPU_IMAGE_SIZE - 2);
    *(u16 *)(buf + NCPU_IMAGE_SIZE - 2) = crc16_w;

    fwrite(buf, NCPU_IMAGE_SIZE, 1, pfOut);
    fclose(pfOut);
    return 0;
}

int gen_ota_model(char * fw_info_file, char * all_model_file, char * out_file)
{
    FILE *pfFW, *pfModel, *pfOut;
    u16 crc16_w;
    u32 total_size;

    pfFW = fopen(fw_info_file, "rb");
    if (pfFW == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, fw_info_file);
        return -1;
    }

    pfModel = fopen(all_model_file, "rb");
    if (pfModel == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, all_model_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfFW, 0, SEEK_END);
    u32 FwFileLen = ftell(pfFW);
    if (FwFileLen > FW_INFO_SIZE)
    {
        printf("Error: intput fw_info file is larger than 232 bytes\n");
        return -1;
    }
    fseek(pfFW, 0, SEEK_SET);

    fseek(pfModel, 0, SEEK_END);
    u32 ModelFileLen = ftell(pfModel);
    if (ModelFileLen > ALL_MODEL_SIZE)
    {
        printf("Error: intput all_model_info file is larger than 3576800 bytes\n");
        return -1;
    }
    fseek(pfModel, 0, SEEK_SET);

    total_size = FW_INFO_SIZE + ModelFileLen + 2;   // 2 bytes for crc word

    if ((total_size % 4) != 0)
    {
        total_size = ((total_size + 3) / 4) * 4;
    }

    u8 * buf = (u8 *)malloc(total_size);
    if (buf == NULL)
    {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, FwFileLen, pfFW);
    if (result != FwFileLen)
    {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        return 0;
    }

    fclose(pfFW);

    memset(buf + FwFileLen, 0xff, FW_INFO_SIZE - FwFileLen);

    u8 * pModel = buf + FW_INFO_SIZE;

    result = fread(pModel, 1, ModelFileLen, pfModel);
    if (result != ModelFileLen)
    {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        return 0;
    }

    fclose(pfModel);

    crc16_w = gen_crc16(buf, total_size - 2);
    *(u16 *)(buf + total_size - 2) = crc16_w;

    fwrite(buf, total_size, 1, pfOut);
    fclose(pfOut);
    return 0;
}

/* 
convert the SPL binary file to the fixed size 8K for flash convenience

*/

int gen_ota_spl(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;
    u16 crc16_w;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL)
    {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > 8192)
    {
        printf("Error: intput SPL file is larger than 8192 bytes\n");
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(8192);
    if (buf == NULL)
    {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen)
    {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        return 0;
    }

    fclose(pfIn);

    memset(buf + inFileLen, 0, 8192 - inFileLen);

    fwrite(buf, 8192, 1, pfOut);
    fclose(pfOut);
    return 0;
}


/* usage: generate_ota_binary -scpu scpu_in_file scpu_out_file
        or generate_ota_binary -ncpu ncpu_in_file ncpu_out_file
        or generate_ota_binary -model fw_info_file_in  all_model_file_in model_out_file
*/

int main(int argc, char* argv[])
{

    char *in_file, *out_file, *fw_info_file, *all_model_file;

    if ((argc != 4) && (argc != 5))
    {
        printf("This utility converts scpu/ncpu/model binary to OTA format output\n");
        printf("usage 1: generate_ota_binary -scpu scpu_in_file scpu_out_file\n");
        printf("usage 2: generate_ota_binary -ncpu ncpu_in_file ncpu_out_file\n");
        printf("usage 3: generate_ota_binary -model fw_info_file_in  all_model_file_in model_out_file\n");
        printf("usage 4: generate_ota_binary -spl spl_file_in  spl_out_file\n");
        return 0;
    }

    if (strcmp(argv[1], "-scpu") == 0)
    {
        in_file = argv[2];
        out_file = argv[3];
        printf("start scpu bin ota gen\n");
        gen_ota_scpu(in_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-ncpu") == 0)
    {
        in_file = argv[2];
        out_file = argv[3];
        gen_ota_ncpu(in_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-model") == 0)
    {
        fw_info_file = argv[2];
        all_model_file = argv[3];
        out_file = argv[4];
        gen_ota_model(fw_info_file, all_model_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-spl") == 0)
    {
        in_file = argv[2];
        out_file = argv[3];
        gen_ota_spl(in_file, out_file);
        return 0;
    }

}


