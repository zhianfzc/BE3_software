//
// Created by Liming Shao on 2018/4/24.
//
#ifndef __KDP_COMM_AES_H__
#define __KDP_COMM_AES_H__
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_protoco.h"

#if ( ENCRYPTION_MODE&AES_ENCRYPTION )
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct{
    uint32_t eK[44], dK[44];    // encKey, decKey
    int Nr; // 10 rounds
}AesKey;

int loadStateArray(uint8_t state[4][4], const uint8_t *in);

int storeStateArray(uint8_t state[4][4], uint8_t *out);

int keyExpansion(const uint8_t *key, uint32_t keyLen, AesKey *aesKey);

int addRoundKey(uint8_t state[4][4], const uint32_t key[4]);

int subBytes(uint8_t state[4][4]);

int invSubBytes(uint8_t state[4][4]);

int shiftRows(uint8_t state[4][4]);

int invShiftRows(uint8_t state[4][4]);

uint8_t GMul(uint8_t a, uint8_t b);

int mixColumns(uint8_t state[4][4]);

int invMixColumns(uint8_t state[4][4]);

// data length must be multiple of 16B, so data need to be padded before encryption/decryption
int aesEncrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *pt, uint8_t *ct, uint32_t len);

int aesDecrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *ct, uint8_t *pt, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif
#if ( ENCRYPTION_MODE&XOR_ENCRYPTION )
int encBytes(uint8_t *bytes, int length, const uint8_t *key, int stride, uint8_t *out);
int DecBytes(uint8_t *bytes, int length, const uint8_t *key, int stride, uint8_t *out);
#endif
#endif
#endif
#endif
#endif    //__KDP_COMM_AES_H__
