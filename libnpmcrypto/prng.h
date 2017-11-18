/*
 * 	prng.h
 *	The default PRNG implementation is Fortuna. It uses 32 pools and AES cipher.
 *	This PRNG should be used with cryptographic operations.
 */

/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifndef LIBNPMCRYPTO_PRNG_H_
#define LIBNPMCRYPTO_PRNG_H_

#include "crypto.h"
#include "sha.h"
#include "aes.h"

/* number of pools, must be a number between 4 and 32 */
#define FORTUNA_POOLS 32

/* number of read before a reseeding */
#define FORTUNA_WD    10

/* export size */
#define FORTUNA_EXPORT_SIZE (32 * FORTUNA_POOLS)

/* IV default size */
#define FORTUNA_IV_SIZE 16

struct fortuna_prng {
    hash_state pool[FORTUNA_POOLS];     /* the  pools */

    aes_key skey;

    unsigned char K[32],      			  /* the current key */
                  IV[FORTUNA_IV_SIZE];     /* IV for CTR mode */

    unsigned long pool_idx,   /* current pool we will add to */
                  pool0_len,  /* length of 0'th pool */
                  wd;

    ulong64       reset_cnt;  /* number of times we have reset */
};

typedef struct {
   union {
      char dummy[1];
      struct fortuna_prng fortuna;
   };
   short ready;            	/* ready flag 0-1 */
} prng_state;

/* fortuna implementation functions. Can be used directly */
int fortuna_start(prng_state *prng);
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng);
int fortuna_ready(prng_state *prng);
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng);
int fortuna_done(prng_state *prng);
int  fortuna_export(unsigned char *out, unsigned long *outlen, prng_state *prng);
int  fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng);

/* create a new PRNG with a random byte array that must be supplied to the function */
int prng_create(prng_state *prng, unsigned char *random, unsigned long inlen);

/* get a random value from the PRNG. If keep == 0, the PRNG will be close and unusable
   after this call. */
int prng_read(prng_state *prng, unsigned char *out, unsigned long inlen, short keep);

/* close the PRNG */
int prng_close(prng_state *prng);


#endif /* LIBNPMCRYPTO_PRNG_H_ */
