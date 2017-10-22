/*
    aes.h
    Prototypes for the AES cipher. The aim is to have a lighweight 
    library that only has what is needed to do AES encryptions.
    
    Based on the LibTomCrypt library.
    
    MIT License

    Copyright (c) [2017] [Neilson P. Marcil]

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE. */

#ifndef NPM_LIB_AES_H_
#define NPM_LIB_AES_H_

#include "crypto.h"

/* AES key definition */
typedef struct {
	ulong32 eK[60], dK[60];
	ulong32 num_rounds;
} aes_key;

/* prepare the symmetric key for encryption or decryption */
extern int aes_key_setup(unsigned char *key, unsigned int size, aes_key *sKey);

/* encrypt a block of text that must be 16 bytes*/
extern int aes_ecb_block_encrypt(unsigned char *data, unsigned char *out, aes_key *key);

/* decrypt a block of text that must be 16 bytes */
extern int aes_ecb_block_decrypt(unsigned char *data, unsigned char *out, aes_key *key);

/* encrypt multiple block with ECB mode. The data length must be a multiple of 16 */
extern int aes_ecb_encrypt(unsigned char *data, unsigned int len, unsigned char *out, aes_key *key);

/* decrypt multiple block with ECB mode. The data length must be a multiple of 16 */
extern int aes_ecb_decrypt(unsigned char *data, unsigned int len, unsigned char *out, aes_key *key);

/* for later */
int aes_cbc_encrypt(unsigned char *data, unsigned char *iv, unsigned int size, aes_key *key);
int aes_cbc_decrypt(unsigned char *data, unsigned int size, aes_key *key);

#endif /* NPM_LIB_AES_H_ */
