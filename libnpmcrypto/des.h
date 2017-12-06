/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifndef LIBNPMCRYPTO_DES_H_
#define LIBNPMCRYPTO_DES_H_

#include "crypto.h"

/* triple DES key only */
typedef struct {
    ulong32 ek[3][32], dk[3][32];
} des3_key;

/* setup the key for encryption and decryption */
int des3_setup(const unsigned char *key, int keylen, des3_key *skey);

/* encrypt */
int des3_encrypt(const unsigned char *in, unsigned char *out, des3_key *skey);

/* decrypt */
int des3_decrypt(const unsigned char *in, unsigned char *out, des3_key *skey);

#endif /* LIBNPMCRYPTO_DES_H_ */
