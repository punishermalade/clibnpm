/*
 * math.h
 * Math functions
 */

#ifndef LIBNPMCRYPTO_MATH_H_
#define LIBNPMCRYPTO_MATH_H_

#include <limits.h>
#include "crypto.h"
#include "prng.h"

/* Equalities used in the math functions.
   The macro that starts with MP are from the math library,
   LTC are from the crypto library */
#define MP_LT        -1   /* less than */
#define MP_EQ         0   /* equal to */
#define MP_GT         1   /* greater than */

#define MP_ZPOS       0   /* positive integer */
#define MP_NEG        1   /* negative */

#define MP_OKAY       0   /* ok result */
#define MP_MEM        -2  /* out of mem */
#define MP_VAL        -3  /* invalid input */
#define MP_RANGE      MP_VAL

#define MP_YES        1   /* yes response */
#define MP_NO         0   /* no response */

#define LTC_MP_NO     0	 /* crypto no response */
#define LTC_MP_YES    1	 /* crypto yes response */

#define LTC_MP_LT    -1  /* crypto less than */
#define LTC_MP_EQ     0  /* crypto equals */
#define LTC_MP_GT     1  /* crypto greater than */

#define DIGIT_BIT     32	/* for high memory system, otherwise change to 8 */
#define MP_PREC       32
#define MP_MASK       ((((mp_digit)1)<<((mp_digit)DIGIT_BIT))-((mp_digit)1))
#define MP_DIGIT_BIT  DIGIT_BIT
#define PRIME_SIZE	  256

/* macro function */
#define mp_iszero(a)     (mp_cmp_d(a, 0) == LTC_MP_EQ ? LTC_MP_YES : LTC_MP_NO)
#define mp_iseven(a) 	((((a)->used == 0) || (((a)->dp[0] & 1u) == 0u)) ? MP_YES : MP_NO)
#define mp_isodd(a)  	((((a)->used > 0) && (((a)->dp[0] & 1u) == 1u)) ? MP_YES : MP_NO)
#define mp_isneg(a) 		(((a)->sign != MP_ZPOS) ? MP_YES : MP_NO)

/* this is to make porting into LibTomCrypt easier :-) */
typedef uint32_t             mp_digit;
typedef uint64_t             mp_word;
typedef unsigned long 		ltc_mp_digit;

/* mp_int structure */
typedef struct  {
   int used, alloc, sign;
   mp_digit *dp;
} mp_int;

/* math functions */
int rand_prime(void *N, long len, prng_state *prng, int (*read)(unsigned char *, long, prng_state *));
int mp_init_multi(void **a, ...);
void mp_deinit_multi(void *a, ...);
void mp_cleanup_multi(void **a, ...);
void mp_clear_multi(mp_int *mp, ...);

/* clear one (frees)  */
void mp_clear(mp_int *a);

int mp_read_unsigned_bin(mp_int *a, const unsigned char *b, int c);
int mp_grow(mp_int *a, int size);
void mp_zero(mp_int *a);
int mp_mul_2d(const mp_int *a, int b, mp_int *c);
int mp_copy(const mp_int *a, mp_int *b);
void mp_clamp(mp_int *a);
int mp_lshd(mp_int *a, int b);
int mp_cmp_d(const mp_int *a, mp_digit b);
int mp_set_int(mp_int *a, unsigned long b);
int mp_gcd(const mp_int *a, const mp_int *b, mp_int *c);
int mp_init_copy(mp_int *a, const mp_int *b);
int mp_init_size(mp_int *a, int size);
int mp_cnt_lsb(const mp_int *a);
int mp_lcm(const mp_int *a, const mp_int *b, mp_int *c);
int mp_cmp_mag(const mp_int *a, const mp_int *b);
int mp_div(const mp_int *a, const mp_int *b, mp_int *c, mp_int *d);
int mp_count_bits(const mp_int *a);
int mp_abs(const mp_int *a, mp_int *b);
int mp_cmp(const mp_int *a, const mp_int *b);
int mp_sub(const mp_int *a, const mp_int *b, mp_int *c);
int mp_add(const mp_int *a, const mp_int *b, mp_int *c);
int mp_sub_d(const mp_int *a, mp_digit b, mp_int *c);
int mp_add_d(const mp_int *a, mp_digit b, mp_int *c);
int mp_mod_2d(const mp_int *a, int b, mp_int *c);

/* low level subtraction (assumes |a| > |b|), HAC pp.595 Algorithm 14.9 */
int s_mp_sub(const mp_int *a, const mp_int *b, mp_int *c);

int mp_invmod(const mp_int *a, const mp_int *b, mp_int *c);
int mp_invmod_slow(const mp_int *a, const mp_int *b, mp_int *c);

int mp_mod(const mp_int *a, const mp_int *b, mp_int *c);

/* set to a digit */
void mp_set(mp_int *a, mp_digit b);

int mp_init(mp_int *a);
void mp_exch(mp_int *a, mp_int *b);

/* high level multiplication (handles sign) */
int mp_mul(const mp_int *a, const mp_int *b, mp_int *c);

/* b = a/2 */
int mp_div_2(const mp_int *a, mp_int *b);

int s_mp_add(const mp_int *a, const mp_int *b, mp_int *c);

/* shift right a certain amount of digits */
void mp_rshd(mp_int *a, int b);

/* shift right by a certain bit count (store quotient in c, optional remainder in d) */
int mp_div_2d(const mp_int *a, int b, mp_int *c, mp_int *d);


/* determines if an integers is divisible by one
 * of the first PRIME_SIZE primes or not
 *
 * sets result to 0 if not, 1 if yes
 */
int mp_prime_is_divisible(const mp_int *a, int *result);

/* performs a variable number of rounds of Miller-Rabin
 *
 * Probability of error after t rounds is no more than

 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
int mp_prime_is_prime(const mp_int *a, int t, int *result);

/* Miller-Rabin test of "a" to the base of "b" as described in
 * HAC pp. 139 Algorithm 4.24
 *
 * Sets result to 0 if definitely composite or 1 if probably prime.
 * Randomly the chance of error is no more than 1/4 and often
 * very much lower.
 */
int mp_prime_miller_rabin(const mp_int *a, const mp_int *b, int *result);

/* c = a * a (mod b) */
int mp_sqrmod(const mp_int *a, const mp_int *b, mp_int *c);

#endif /* LIBNPMCRYPTO_MATH_H_ */
