/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* AES implementation by Tom St Denis
 *
 * Derived from the Public Domain source code by

---
  * rijndael-alg-fst.c
  *
  * @version 3.0 (December 2000)
  *
  * Optimised ANSI C code for the Rijndael cipher (now AES)
  *
  * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
  * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
  * @author Paulo Barreto <paulo.barreto@terra.com.br>
---
 */

#ifndef SRC_AES_H_
#define SRC_AES_H_

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

#endif /* SRC_AES_H_ */
