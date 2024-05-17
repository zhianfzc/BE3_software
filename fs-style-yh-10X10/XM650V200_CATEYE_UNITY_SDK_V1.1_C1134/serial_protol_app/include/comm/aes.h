#ifndef __AES_H__
#define __AES_H__

#include <stdio.h>

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

uint16_t DSM_CheckBodySize( uint16_t body_size);
int aesEncrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *pt, uint8_t *ct, uint32_t len);
int aesDecrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *ct, uint8_t *pt, uint32_t len);


#endif    //__AES_H__

