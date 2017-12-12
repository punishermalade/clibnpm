/*
 * 	prng.c
 *	The default PRNG implementation is Fortuna. It uses 32 pools and AES cipher.
 */

/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* Implementation of Fortuna by Tom St Denis

We deviate slightly here for reasons of simplicity [and to fit in the API].  First all "sources"
in the AddEntropy function are fixed to 0.  Second since no reliable timer is provided
we reseed automatically when len(pool0) >= 64 or every LTC_FORTUNA_WD calls to the read function */

#include "prng.h"

/**
 * Wrapper functions to make it easy to use.
 */
int prng_create(prng_state *prng, unsigned char *random, unsigned long inlen)
{
	int result = CRYPTO_OK;

	if ((result = fortuna_start(prng)) != CRYPTO_OK)
	{
		return result;
	}

	if ((result = fortuna_add_entropy(random, inlen, prng)) != CRYPTO_OK)
	{
		return result;
	}

	if ((result = fortuna_ready(prng)) != CRYPTO_OK)
	{
		return result;
	}

	return result;
}


int prng_read(prng_state *prng, unsigned char *out, unsigned long outlen, short keep)
{
	int result = CRYPTO_OK;

	if ((result = fortuna_read(out, outlen, prng)) != CRYPTO_OK)
	{
		return result;
	}

	/* checking if we need to close this prng after the read operation */
	if (keep == 0)
	{
		return prng_close(prng);
	}

	return result;
}

int prng_close(prng_state *prng)
{
	return fortuna_done(prng);
}

/* internal cleanup function for a memory block */
static void zeromem(volatile void *buf, unsigned int len)
{
	volatile unsigned char *mem = buf;
	while (len-- > 0)
	{
		*mem = '\0';
		mem++;
	}
}


/* update the IV */
static void _fortuna_update_iv(prng_state *prng)
{
   int            x;
   unsigned char *IV;
   /* update IV */
   IV = prng->fortuna.IV;
   for (x = 0; x < 16; x++) {
      IV[x] = (IV[x] + 1) & 255;
      if (IV[x] != 0) break;
   }
}

/* reseed the PRNG */
static int _fortuna_reseed(prng_state *prng)
{
   unsigned char tmp[MAXBLOCKSIZE];
   hash_state    md;
   int           err, x;

   ++prng->fortuna.reset_cnt;

   /* new K == LTC_SHA256(K || s) where s == LTC_SHA256(P0) || LTC_SHA256(P1) ... */
   sha256_init(&md);
   if ((err = sha256_process(&md, prng->fortuna.K, 32)) != CRYPTO_OK) {
      sha256_done(&md, tmp);
      return err;
   }

   for (x = 0; x < FORTUNA_POOLS; x++) {
       if (x == 0 || ((prng->fortuna.reset_cnt >> (x-1)) & 1) == 0) {
          /* terminate this hash */
          if ((err = sha256_done(&prng->fortuna.pool[x], tmp)) != CRYPTO_OK) {
             sha256_done(&md, tmp);
             return err;
          }
          /* add it to the string */
          if ((err = sha256_process(&md, tmp, 32)) != CRYPTO_OK) {
             sha256_done(&md, tmp);
             return err;
          }
          /* reset this pool */
          if ((err = sha256_init(&prng->fortuna.pool[x])) != CRYPTO_OK) {
             sha256_done(&md, tmp);
             return err;
          }
       } else {
          break;
       }
   }

   /* finish key */
   if ((err = sha256_done(&md, prng->fortuna.K)) != CRYPTO_OK) {
      return err;
   }
   if ((err = aes_key_setup(prng->fortuna.K, 32, &prng->fortuna.skey)) != CRYPTO_OK) {
      return err;
   }
   _fortuna_update_iv(prng);

   /* reset pool len */
   prng->fortuna.pool0_len = 0;
   prng->fortuna.wd        = 0;

   zeromem(&md, sizeof(md));
   zeromem(tmp, sizeof(tmp));

   return CRYPTO_OK;
}

/**
  Start the PRNG
  @param prng     [out] The PRNG state to initialize
  @return CRYPOT_OK if successful
*/
int fortuna_start(prng_state *prng)
{
   int err, x, y;
   unsigned char tmp[MAXBLOCKSIZE];
   prng->ready = 0;

   /* initialize the pools */
   for (x = 0; x < FORTUNA_POOLS; x++) {
       if ((err = sha256_init(&prng->fortuna.pool[x])) != CRYPTO_OK) {
          for (y = 0; y < x; y++) {
              sha256_done(&prng->fortuna.pool[y], tmp);
          }
          return err;
       }
   }
   prng->fortuna.pool_idx = prng->fortuna.pool0_len = prng->fortuna.wd = 0;
   prng->fortuna.reset_cnt = 0;

   /* reset bufs */
   zeromem(prng->fortuna.K, 32);
   if ((err = aes_key_setup(prng->fortuna.K, 32, &prng->fortuna.skey)) != CRYPTO_OK) {
      for (x = 0; x < FORTUNA_POOLS; x++) {
          sha256_done(&prng->fortuna.pool[x], tmp);
      }
      return err;
   }

   zeromem(prng->fortuna.IV, 16);

   return CRYPTO_OK;
}

/**
  Add entropy to the PRNG state
  @param in       The data to add
  @param inlen    Length of the data to add
  @param prng     PRNG state to update
  @return CRYPT_OK if successful
*/
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   unsigned char tmp[2];
   int           err;

   /* ensure inlen <= 32 */
   if (inlen > 32) {
      inlen = 32;
   }

   /* add s || length(in) || in to pool[pool_idx] */
   tmp[0] = 0;
   tmp[1] = (unsigned char)inlen;

   if ((err = sha256_process(&prng->fortuna.pool[prng->fortuna.pool_idx], tmp, 2)) != CRYPTO_OK) {
      goto LBL_UNLOCK;
   }
   if ((err = sha256_process(&prng->fortuna.pool[prng->fortuna.pool_idx], in, inlen)) != CRYPTO_OK) {
      goto LBL_UNLOCK;
   }
   if (prng->fortuna.pool_idx == 0) {
      prng->fortuna.pool0_len += inlen;
   }
   if (++(prng->fortuna.pool_idx) == FORTUNA_POOLS) {
      prng->fortuna.pool_idx = 0;
   }
   err = CRYPTO_OK; /* success */

LBL_UNLOCK:
   return err;
}

/**
  Make the PRNG ready to read from
  @param prng   The PRNG to make active
  @return CRYPT_OK if successful
*/
int fortuna_ready(prng_state *prng)
{
   int err;
   err = _fortuna_reseed(prng);
   prng->ready = (err == CRYPTO_OK) ? 1 : 0;
   return err;
}

/**
  Read from the PRNG
  @param out      Destination
  @param outlen   Length of output
  @param prng     The active PRNG to read from
  @return Number of octets read
*/
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
   unsigned char tmp[16];
   unsigned long tlen = 0;

   if (outlen == 0 || prng == NULL || out == NULL) return 0;

   if (!prng->ready) {
      goto LBL_UNLOCK;
   }

   /* do we have to reseed? */
   if (++prng->fortuna.wd == FORTUNA_WD || prng->fortuna.pool0_len >= 64) {
      if (_fortuna_reseed(prng) != CRYPTO_OK) {
         goto LBL_UNLOCK;
      }
   }

   /* now generate the blocks required */
   tlen = outlen;

   /* handle whole blocks without the extra XMEMCPY */
   while (outlen >= 16) {
      /* encrypt the IV and store it */
      aes_ecb_encrypt(prng->fortuna.IV, FORTUNA_IV_SIZE, out, &prng->fortuna.skey);
      out += 16;
      outlen -= 16;
      _fortuna_update_iv(prng);
   }

   /* left over bytes? */
   if (outlen > 0) {
      aes_ecb_encrypt(prng->fortuna.IV, FORTUNA_IV_SIZE, tmp, &prng->fortuna.skey);
      XMEMCPY(out, tmp, outlen);
      _fortuna_update_iv(prng);
   }

   /* generate new key */
   aes_ecb_encrypt(prng->fortuna.IV, FORTUNA_IV_SIZE, prng->fortuna.K, &prng->fortuna.skey);
   _fortuna_update_iv(prng);

   aes_ecb_encrypt(prng->fortuna.IV, FORTUNA_IV_SIZE, prng->fortuna.K+16, &prng->fortuna.skey);
   _fortuna_update_iv(prng);

   if (aes_key_setup(prng->fortuna.K, 32, &prng->fortuna.skey) != CRYPTO_OK) {
      tlen = 0;
   }

LBL_UNLOCK:
   zeromem(tmp, sizeof(tmp));
   return tlen;
}

/**
  Terminate the PRNG
  @param prng   The PRNG to terminate
  @return CRYPT_OK if successful
*/
int fortuna_done(prng_state *prng)
{
   int           err, x;
   unsigned char tmp[32];

   prng->ready = 0;

   /* terminate all the hashes */
   for (x = 0; x < FORTUNA_POOLS; x++) {
       if ((err = sha256_done(&(prng->fortuna.pool[x]), tmp)) != CRYPTO_OK) {
          goto LBL_UNLOCK;
       }
   }
   /* call cipher done when we invent one ;-) */
   err = CRYPTO_OK; /* success */

LBL_UNLOCK:
   zeromem(tmp, sizeof(tmp));
   return err;
}

/**
  Export the PRNG state
  @param out       [out] Destination
  @param outlen    [in/out] Max size and resulting size of the state
  @param prng      The PRNG to export
  @return CRYPT_OK if successful
*/
int fortuna_export(unsigned char *out, unsigned long *outlen, prng_state *prng)
{
   int         x, err;
   hash_state *md;
   unsigned long len = FORTUNA_EXPORT_SIZE;

   if (!prng->ready) {
      err = CRYPTO_ERROR;
      goto LBL_UNLOCK;
   }

   /* we'll write bytes for s&g's */
   if (*outlen < len) {
      *outlen = len;
      err = CRYPTO_BUFFER_OVERFLOW;
      goto LBL_UNLOCK;
   }

   md = XMALLOC(sizeof(hash_state));
   if (md == NULL) {
      err = CRYPTO_MEMORY;
      goto LBL_UNLOCK;
   }

   /* to emit the state we copy each pool, terminate it then hash it again so
    * an attacker who sees the state can't determine the current state of the PRNG
    */
   for (x = 0; x < FORTUNA_POOLS; x++) {
      /* copy the PRNG */
      XMEMCPY(md, &(prng->fortuna.pool[x]), sizeof(*md));

      /* terminate it */
      if ((err = sha256_done(md, out+x*32)) != CRYPTO_OK) {
         goto LBL_ERR;
      }

      /* now hash it */
      if ((err = sha256_init(md)) != CRYPTO_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256_process(md, out+x*32, 32)) != CRYPTO_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256_done(md, out+x*32)) != CRYPTO_OK) {
         goto LBL_ERR;
      }
   }
   *outlen = len;
   err = CRYPTO_OK;

LBL_ERR:
   zeromem(md, sizeof(*md));
   XFREE(md);
LBL_UNLOCK:
   return err;
}

/**
  Import a PRNG state
  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return CRYPTO_OK if successful
*/
int fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   int err, x;
   if (inlen < (unsigned long)FORTUNA_EXPORT_SIZE) {
      return CRYPTO_INVALID_ARG;
   }

   if ((err = fortuna_start(prng)) != CRYPTO_OK) {
      return err;
   }
   for (x = 0; x < FORTUNA_POOLS; x++) {
      if ((err = fortuna_add_entropy(in+x*32, 32, prng)) != CRYPTO_OK) {
         return err;
      }
   }
   return CRYPTO_OK;
}
