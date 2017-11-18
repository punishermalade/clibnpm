/*
 * rsa.h
 * RSA implementation to create public/private key, sign, encrypt data.
 */
/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifndef LIBNPMCRYPTO_RSA_H_
#define LIBNPMCRYPTO_RSA_H_

#include "crypto.h"
#include "prng.h"

/* public or private key indicator */
enum {
   PK_PUBLIC 	= 0,
   PK_PRIVATE 	= 1
};

/* rsa key definition */
typedef struct rsa_key_struct {
    /** Type of key, PK_PRIVATE or PK_PUBLIC */
    int type;
    /** The public exponent */
    void *e;
    /** The private exponent */
    void *d;
    /** The modulus */
    void *N;
    /** The p factor of N */
    void *p;
    /** The q factor of N */
    void *q;
    /** The 1/q mod p CRT param */
    void *qP;
    /** The d mod (p - 1) CRT param */
    void *dP;
    /** The d mod (q - 1) CRT param */
    void *dQ;
} rsa_key;

/* prng function pointer structure, if another PRNG is desired
   map the function in this structure. */
struct prng_func_pt {

	/* get a random value */
	int (*read)(unsigned char *, long, prng_state *);

};

/*
   Create an RSA key
   @param prng     An active PRNG state
   @param size     The size of the modulus (key size) desired (octets)
   @param e        The "e" value (public key).  e==65537 is a good choice
   @param key      [out] Destination of a newly created private key pair
   @return CRYPTO_OK if successful, upon error all allocated ram is freed
 */
int rsa_create_key(prng_state *prng, int size, long e, rsa_key *key);

/*
   Free a RSA key from the memory
 */
void rsa_free(rsa_key *key);


#endif /* LIBNPMCRYPTO_RSA_H_ */
