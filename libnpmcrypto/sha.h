/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#ifndef NPM_LIB_SHA_H_
#define NPM_LIB_SHA_H_

#include "crypto.h"

struct md5_state {
    ulong64 length;
    ulong32 state[4], curlen;
    unsigned char buf[64];
};

struct sha1_state {
    ulong64 length;
    ulong32 state[5], curlen;
    unsigned char buf[64];
};

struct sha256_state {
    ulong64 length;
    ulong32 state[8], curlen;
    unsigned char buf[64];
};

struct sha512_state {
    ulong64  length, state[8];
    unsigned long curlen;
    unsigned char buf[128];
};

typedef union u_hash_state {
    char dummy[1];
    struct md5_state    md5;
    struct sha1_state   sha1;
    struct sha256_state sha256;
    struct sha512_state sha512;
    void *data;
} hash_state;

int md5_init(hash_state * md);
int md5_process(hash_state * md, const unsigned char *in, unsigned long inlen);
int md5_done(hash_state * md, unsigned char *hash);

int sha1_init(hash_state * md);
int sha1_process(hash_state * md, const unsigned char *in, unsigned long inlen);
int sha1_done(hash_state * md, unsigned char *hash);

int sha256_init(hash_state * md);
int sha256_process(hash_state * md, const unsigned char *in, unsigned long inlen);
int sha256_done(hash_state * md, unsigned char *hash);

int sha512_init(hash_state * md);
int sha512_process(hash_state * md, const unsigned char *in, unsigned long inlen);
int sha512_done(hash_state * md, unsigned char *hash);

#define HASH_PROCESS(func_name, compress_name, state_var, block_size)                       \
int func_name (hash_state * md, const unsigned char *in, unsigned long inlen)                \
{                                                                                           \
    unsigned long n;                                                                        \
    int           err;                                                                      \
    if (md-> state_var .curlen > sizeof(md-> state_var .buf)) {                             \
       return CRYPTO_INVALID_ARG;                                                           \
    }                                                                                       \
    if ((md-> state_var .length + inlen) < md-> state_var .length) {                        \
      return CRYPTO_HASH_OVERFLOW;                                                          \
    }                                                                                       \
    while (inlen > 0) {                                                                     \
        if (md-> state_var .curlen == 0 && inlen >= block_size) {                           \
           if ((err = compress_name (md, (unsigned char *)in)) != CRYPTO_OK) {              \
              return err;                                                                   \
           }                                                                                \
           md-> state_var .length += block_size * 8;                                        \
           in             += block_size;                                                    \
           inlen          -= block_size;                                                    \
        } else {                                                                            \
           n = MIN(inlen, (block_size - md-> state_var .curlen));                           \
           XMEMCPY(md-> state_var .buf + md-> state_var.curlen, in, (size_t)n);             \
           md-> state_var .curlen += n;                                                     \
           in             += n;                                                             \
           inlen          -= n;                                                             \
           if (md-> state_var .curlen == block_size) {                                      \
              if ((err = compress_name (md, md-> state_var .buf)) != CRYPTO_OK) {           \
                 return err;                                                                \
              }                                                                             \
              md-> state_var .length += 8*block_size;                                       \
              md-> state_var .curlen = 0;                                                   \
           }                                                                                \
       }                                                                                    \
    }                                                                                       \
    return CRYPTO_OK;                                                                       \
}

#endif /* SRC_SHA_H_ */
