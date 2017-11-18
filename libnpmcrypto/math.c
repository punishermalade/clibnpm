/*
 * math.c
 * Math function implementation
 */

#include <stdarg.h>
#include "math.h"

/* this function can be used directly */
int rand_prime(void *N, long len, prng_state *prng, int (*read)(unsigned char *, long, prng_state *))
{
   int            err, res, type;
   unsigned char *buf;

   /* get type */
   if (len < 0) {
      type = 1;
      len = -len;
   } else {
      type = 0;
   }

   /* allow sizes between 2 and 512 bytes for a prime size */
   if (len < 2 || len > 512) {
      return CRYPTO_INVALID_PRIME_SIZE;
   }

   /* allocate buffer to work with */
   buf = XCALLOC(1, len);
   if (buf == NULL) {
       return CRYPTO_MEMORY;
   }

   do {
      /* generate value */
      if (read(buf, len, prng) != (unsigned long)len) {
         XFREE(buf);
         return CRYPTO_ERROR_READPRNG;
      }

      /* munge bits */
      buf[0]     |= 0x80 | 0x40;
      buf[len-1] |= 0x01 | ((type & 1) ? 0x02 : 0x00);

      /* load value */
      if ((err = mp_read_unsigned_bin(N, buf, len)) != CRYPTO_OK) {
         XFREE(buf);
         return err;
      }

      /* test */
      if ((err = mp_prime_is_prime(N, 40, &res)) != CRYPTO_OK) {
         XFREE(buf);
         return err;
      }
   } while (res == MP_NO);

   //zeromem(buf, len);
   XFREE(buf);
   return CRYPTO_OK;
}

int mp_init_multi(void **a, ...)
{
   void    **cur = a;
   int       np  = 0;
   va_list   args;

   va_start(args, a);
   while (cur != NULL) {
       if (mp_init(cur) != CRYPTO_OK) {
          /* failed */
          va_list clean_list;

          va_start(clean_list, a);
          cur = a;
          while (np--) {
              mp_clear(*cur);
              cur = va_arg(clean_list, void**);
          }
          va_end(clean_list);
          va_end(args);
          return CRYPTO_MEMORY;
       }
       ++np;
       cur = va_arg(args, void**);
   }
   va_end(args);
   return CRYPTO_OK;
}

void mp_deinit_multi(void *a, ...)
{
   void     *cur = a;
   va_list   args;

   va_start(args, a);
   while (cur != NULL) {
       mp_clear(cur);
       cur = va_arg(args, void *);
   }
   va_end(args);
}

void mp_cleanup_multi(void **a, ...)
{
   void **cur = a;
   va_list args;

   va_start(args, a);
   while (cur != NULL) {
      if (*cur != NULL) {
         mp_clear(*cur);
         *cur = NULL;
      }
      cur = va_arg(args, void**);
   }
   va_end(args);
   return;
}

void mp_clear_multi(mp_int *mp, ...)
{
   mp_int *next_mp = mp;
   va_list args;
   va_start(args, mp);
   while (next_mp != NULL) {
      mp_clear(next_mp);
      next_mp = va_arg(args, mp_int *);
   }
   va_end(args);
}

/* reads a unsigned char array, assumes the msb is stored first [big endian] */
int mp_read_unsigned_bin(mp_int *a, const unsigned char *b, int c)
{
   int     res;

   /* make sure there are at least two digits */
   if (a->alloc < 2) {
      if ((res = mp_grow(a, 2)) != MP_OKAY) {
         return res;
      }
   }

   /* zero the int */
   mp_zero(a);

   /* read the bytes in */
   while (c-- > 0) {
      if ((res = mp_mul_2d(a, 8, a)) != MP_OKAY) {
         return res;
      }
      a->dp[0] |= *b++;
      a->used += 1;
   }
   mp_clamp(a);
   return MP_OKAY;
}

/* grow as required */
int mp_grow(mp_int *a, int size)
{
   int     i;
   mp_digit *tmp;

   /* if the alloc size is smaller alloc more ram */
   if (a->alloc < size) {
      /* ensure there are always at least MP_PREC digits extra on top */
      size += (MP_PREC * 2) - (size % MP_PREC);

      /* reallocate the array a->dp
       *
       * We store the return in a temporary variable
       * in case the operation failed we don't want
       * to overwrite the dp member of a.
       */
      tmp = XREALLOC(a->dp, sizeof(mp_digit) * (size_t)size);
      if (tmp == NULL) {
         /* reallocation failed but "a" is still valid [can be freed] */
         return MP_MEM;
      }

      /* reallocation succeeded so set a->dp */
      a->dp = tmp;

      /* zero excess digits */
      i        = a->alloc;
      a->alloc = size;
      for (; i < a->alloc; i++) {
         a->dp[i] = 0;
      }
   }
   return MP_OKAY;
}

/* set to zero */
void mp_zero(mp_int *a)
{
   int       n;
   mp_digit *tmp;

   a->sign = MP_ZPOS;
   a->used = 0;

   tmp = a->dp;
   for (n = 0; n < a->alloc; n++) {
      *tmp++ = 0;
   }
}

/* shift left by a certain bit count */
int mp_mul_2d(const mp_int *a, int b, mp_int *c)
{
   mp_digit d;
   int      res;

   /* copy */
   if (a != c) {
      if ((res = mp_copy(a, c)) != MP_OKAY) {
         return res;
      }
   }

   if (c->alloc < (c->used + (b / DIGIT_BIT) + 1)) {
      if ((res = mp_grow(c, c->used + (b / DIGIT_BIT) + 1)) != MP_OKAY) {
         return res;
      }
   }

   /* shift by as many digits in the bit count */
   if (b >= DIGIT_BIT) {
      if ((res = mp_lshd(c, b / DIGIT_BIT)) != MP_OKAY) {
         return res;
      }
   }

   /* shift any bit count < DIGIT_BIT */
   d = (mp_digit)(b % DIGIT_BIT);
   if (d != 0u) {
      mp_digit *tmpc, shift, mask, r, rr;
      int x;

      /* bitmask for carries */
      mask = ((mp_digit)1 << d) - (mp_digit)1;

      /* shift for msbs */
      shift = (mp_digit)DIGIT_BIT - d;

      /* alias */
      tmpc = c->dp;

      /* carry */
      r    = 0;
      for (x = 0; x < c->used; x++) {
         /* get the higher bits of the current word */
         rr = (*tmpc >> shift) & mask;

         /* shift the current word and OR in the carry */
         *tmpc = ((*tmpc << d) | r) & MP_MASK;
         ++tmpc;

         /* set the carry to the carry bits of the current word */
         r = rr;
      }

      /* set final carry */
      if (r != 0u) {
         c->dp[(c->used)++] = r;
      }
   }
   mp_clamp(c);
   return MP_OKAY;
}

/* copy, b = a */
int mp_copy(const mp_int *a, mp_int *b)
{
   int     res, n;

   /* if dst == src do nothing */
   if (a == b) {
      return MP_OKAY;
   }

   /* grow dest */
   if (b->alloc < a->used) {
      if ((res = mp_grow(b, a->used)) != MP_OKAY) {
         return res;
      }
   }

   /* zero b and copy the parameters over */
   {
      mp_digit *tmpa, *tmpb;

      /* pointer aliases */

      /* source */
      tmpa = a->dp;

      /* destination */
      tmpb = b->dp;

      /* copy all the digits */
      for (n = 0; n < a->used; n++) {
         *tmpb++ = *tmpa++;
      }

      /* clear high digits */
      for (; n < b->used; n++) {
         *tmpb++ = 0;
      }
   }

   /* copy used count and sign */
   b->used = a->used;
   b->sign = a->sign;
   return MP_OKAY;
}

/* trim unused digits
 *
 * This is used to ensure that leading zero digits are
 * trimed and the leading "used" digit will be non-zero
 * Typically very fast.  Also fixes the sign if there
 * are no more leading digits
 */
void mp_clamp(mp_int *a)
{
   /* decrease used while the most significant digit is
    * zero.
    */
   while ((a->used > 0) && (a->dp[a->used - 1] == 0u)) {
      --(a->used);
   }

   /* reset the sign flag if used == 0 */
   if (a->used == 0) {
      a->sign = MP_ZPOS;
   }
}

/* shift left a certain amount of digits */
int mp_lshd(mp_int *a, int b)
{
   int     x, res;

   /* if its less than zero return */
   if (b <= 0) {
      return MP_OKAY;
   }
   /* no need to shift 0 around */
   if (mp_iszero(a) == MP_YES) {
      return MP_OKAY;
   }

   /* grow to fit the new digits */
   if (a->alloc < (a->used + b)) {
      if ((res = mp_grow(a, a->used + b)) != MP_OKAY) {
         return res;
      }
   }

   {
      mp_digit *top, *bottom;

      /* increment the used by the shift amount then copy upwards */
      a->used += b;

      /* top */
      top = a->dp + a->used - 1;

      /* base */
      bottom = (a->dp + a->used - 1) - b;

      /* much like mp_rshd this is implemented using a sliding window
       * except the window goes the otherway around.  Copying from
       * the bottom to the top.  see bn_mp_rshd.c for more info.
       */
      for (x = a->used - 1; x >= b; x--) {
         *top-- = *bottom--;
      }

      /* zero the lower digits */
      top = a->dp;
      for (x = 0; x < b; x++) {
         *top++ = 0;
      }
   }
   return MP_OKAY;
}

/* compare a digit */
int mp_cmp_d(const mp_int *a, mp_digit b)
{
   /* compare based on sign */
   if (a->sign == MP_NEG) {
      return MP_LT;
   }

   /* compare based on magnitude */
   if (a->used > 1) {
      return MP_GT;
   }

   /* compare the only digit of a to b */
   if (a->dp[0] > b) {
      return MP_GT;
   } else if (a->dp[0] < b) {
      return MP_LT;
   } else {
      return MP_EQ;
   }
}

/* set a 32-bit const */
int mp_set_int(mp_int *a, unsigned long b)
{
   int     x, res;

   mp_zero(a);

   /* set four bits at a time */
   for (x = 0; x < 8; x++) {
      /* shift the number up four bits */
      if ((res = mp_mul_2d(a, 4, a)) != MP_OKAY) {
         return res;
      }

      /* OR in the top four bits of the source */
      a->dp[0] |= (mp_digit)(b >> 28) & 15uL;

      /* shift the source up to the next four bits */
      b <<= 4;

      /* ensure that digits are not clamped off */
      a->used += 1;
   }
   mp_clamp(a);
   return MP_OKAY;
}

/* single digit subtraction */
int mp_sub_d(const mp_int *a, mp_digit b, mp_int *c)
{
   mp_digit *tmpa, *tmpc, mu;
   int       res, ix, oldused;

   /* grow c as required */
   if (c->alloc < (a->used + 1)) {
      if ((res = mp_grow(c, a->used + 1)) != MP_OKAY) {
         return res;
      }
   }

   /* if a is negative just do an unsigned
    * addition [with fudged signs]
    */
   if (a->sign == MP_NEG) {
      mp_int a_ = *a;
      a_.sign = MP_ZPOS;
      res     = mp_add_d(&a_, b, c);
      c->sign = MP_NEG;

      /* clamp */
      mp_clamp(c);

      return res;
   }

   /* setup regs */
   oldused = c->used;
   tmpa    = a->dp;
   tmpc    = c->dp;

   /* if a <= b simply fix the single digit */
   if (((a->used == 1) && (a->dp[0] <= b)) || (a->used == 0)) {
      if (a->used == 1) {
         *tmpc++ = b - *tmpa;
      } else {
         *tmpc++ = b;
      }
      ix      = 1;

      /* negative/1digit */
      c->sign = MP_NEG;
      c->used = 1;
   } else {
      /* positive/size */
      c->sign = MP_ZPOS;
      c->used = a->used;

      /* subtract first digit */
      *tmpc    = *tmpa++ - b;
      mu       = *tmpc >> ((sizeof(mp_digit) * (size_t)CHAR_BIT) - 1u);
      *tmpc++ &= MP_MASK;

      /* handle rest of the digits */
      for (ix = 1; ix < a->used; ix++) {
         *tmpc    = *tmpa++ - mu;
         mu       = *tmpc >> ((sizeof(mp_digit) * (size_t)CHAR_BIT) - 1u);
         *tmpc++ &= MP_MASK;
      }
   }

   /* zero excess digits */
   while (ix++ < oldused) {
      *tmpc++ = 0;
   }
   mp_clamp(c);
   return MP_OKAY;
}

/* Greatest Common Divisor using the binary method */
int mp_gcd(const mp_int *a, const mp_int *b, mp_int *c)
{
   mp_int  u, v;
   int     k, u_lsb, v_lsb, res;

   /* either zero than gcd is the largest */
   if (mp_iszero(a) == MP_YES) {
      return mp_abs(b, c);
   }
   if (mp_iszero(b) == MP_YES) {
      return mp_abs(a, c);
   }

   /* get copies of a and b we can modify */
   if ((res = mp_init_copy(&u, a)) != MP_OKAY) {
      return res;
   }

   if ((res = mp_init_copy(&v, b)) != MP_OKAY) {
      goto LBL_U;
   }

   /* must be positive for the remainder of the algorithm */
   u.sign = v.sign = MP_ZPOS;

   /* B1.  Find the common power of two for u and v */
   u_lsb = mp_cnt_lsb(&u);
   v_lsb = mp_cnt_lsb(&v);
   k     = MIN(u_lsb, v_lsb);

   if (k > 0) {
      /* divide the power of two out */
      if ((res = mp_div_2d(&u, k, &u, NULL)) != MP_OKAY) {
         goto LBL_V;
      }

      if ((res = mp_div_2d(&v, k, &v, NULL)) != MP_OKAY) {
         goto LBL_V;
      }
   }

   /* divide any remaining factors of two out */
   if (u_lsb != k) {
      if ((res = mp_div_2d(&u, u_lsb - k, &u, NULL)) != MP_OKAY) {
         goto LBL_V;
      }
   }

   if (v_lsb != k) {
      if ((res = mp_div_2d(&v, v_lsb - k, &v, NULL)) != MP_OKAY) {
         goto LBL_V;
      }
   }

   while (mp_iszero(&v) == MP_NO) {
      /* make sure v is the largest */
      if (mp_cmp_mag(&u, &v) == MP_GT) {
         /* swap u and v to make sure v is >= u */
         mp_exch(&u, &v);
      }

      /* subtract smallest from largest */
      if ((res = s_mp_sub(&v, &u, &v)) != MP_OKAY) {
         goto LBL_V;
      }

      /* Divide out all factors of two */
      if ((res = mp_div_2d(&v, mp_cnt_lsb(&v), &v, NULL)) != MP_OKAY) {
         goto LBL_V;
      }
   }

   /* multiply by 2**k which we divided out at the beginning */
   if ((res = mp_mul_2d(&u, k, c)) != MP_OKAY) {
      goto LBL_V;
   }
   c->sign = MP_ZPOS;
   res = MP_OKAY;
LBL_V:
   mp_clear(&u);
LBL_U:
   mp_clear(&v);
   return res;
}

/* creates "a" then copies b into it */
int mp_init_copy(mp_int *a, const mp_int *b)
{
   int     res;

   if ((res = mp_init_size(a, b->used)) != MP_OKAY) {
      return res;
   }

   if ((res = mp_copy(b, a)) != MP_OKAY) {
      mp_clear(a);
   }

   return res;
}

/* init an mp_init for a given size */
int mp_init_size(mp_int *a, int size)
{
   int x;

   /* pad size so there are always extra digits */
   size += (MP_PREC * 2) - (size % MP_PREC);

   /* alloc mem */
   a->dp = XMALLOC(sizeof(mp_digit) * (size_t)size);
   if (a->dp == NULL) {
      return MP_MEM;
   }

   /* set the members */
   a->used  = 0;
   a->alloc = size;
   a->sign  = MP_ZPOS;

   /* zero the digits */
   for (x = 0; x < size; x++) {
      a->dp[x] = 0;
   }

   return MP_OKAY;
}

static const int lnz[16] = {
   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/* Counts the number of lsbs which are zero before the first zero bit */
int mp_cnt_lsb(const mp_int *a)
{
   int x;
   mp_digit q, qq;

   /* easy out */
   if (mp_iszero(a) == MP_YES) {
      return 0;
   }

   /* scan lower digits until non-zero */
   for (x = 0; (x < a->used) && (a->dp[x] == 0u); x++) {}
   q = a->dp[x];
   x *= DIGIT_BIT;

   /* now scan this digit until a 1 is found */
   if ((q & 1u) == 0u) {
      do {
         qq  = q & 15u;
         x  += lnz[qq];
         q >>= 4;
      } while (qq == 0u);
   }
   return x;
}

/* computes least common multiple as |a*b|/(a, b) */
int mp_lcm(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     res;
   mp_int  t1, t2;


   if ((res = mp_init_multi(&t1, &t2, NULL)) != MP_OKAY) {
      return res;
   }

   /* t1 = get the GCD of the two inputs */
   if ((res = mp_gcd(a, b, &t1)) != MP_OKAY) {
      goto LBL_T;
   }

   /* divide the smallest by the GCD */
   if (mp_cmp_mag(a, b) == MP_LT) {
      /* store quotient in t2 such that t2 * b is the LCM */
      if ((res = mp_div(a, &t1, &t2, NULL)) != MP_OKAY) {
         goto LBL_T;
      }
      res = mp_mul(b, &t2, c);
   } else {
      /* store quotient in t2 such that t2 * a is the LCM */
      if ((res = mp_div(b, &t1, &t2, NULL)) != MP_OKAY) {
         goto LBL_T;
      }
      res = mp_mul(a, &t2, c);
   }

   /* fix the sign to positive */
   c->sign = MP_ZPOS;

LBL_T:
   mp_clear_multi(&t1, &t2, NULL);
   return res;
}

/* compare maginitude of two ints (unsigned) */
int mp_cmp_mag(const mp_int *a, const mp_int *b)
{
   int     n;
   mp_digit *tmpa, *tmpb;

   /* compare based on # of non-zero digits */
   if (a->used > b->used) {
      return MP_GT;
   }

   if (a->used < b->used) {
      return MP_LT;
   }

   /* alias for a */
   tmpa = a->dp + (a->used - 1);

   /* alias for b */
   tmpb = b->dp + (a->used - 1);

   /* compare based on digits  */
   for (n = 0; n < a->used; ++n, --tmpa, --tmpb) {
      if (*tmpa > *tmpb) {
         return MP_GT;
      }

      if (*tmpa < *tmpb) {
         return MP_LT;
      }
   }
   return MP_EQ;
}

/* slower bit-bang division... also smaller */
int mp_div(const mp_int *a, const mp_int *b, mp_int *c, mp_int *d)
{
   mp_int ta, tb, tq, q;
   int    res, n, n2;

   /* is divisor zero ? */
   if (mp_iszero(b) == MP_YES) {
      return MP_VAL;
   }

   /* if a < b then q=0, r = a */
   if (mp_cmp_mag(a, b) == MP_LT) {
      if (d != NULL) {
         res = mp_copy(a, d);
      } else {
         res = MP_OKAY;
      }
      if (c != NULL) {
         mp_zero(c);
      }
      return res;
   }

   /* init our temps */
   if ((res = mp_init_multi(&ta, &tb, &tq, &q, NULL)) != MP_OKAY) {
      return res;
   }


   mp_set(&tq, 1uL);
   n = mp_count_bits(a) - mp_count_bits(b);
   if (((res = mp_abs(a, &ta)) != MP_OKAY) ||
       ((res = mp_abs(b, &tb)) != MP_OKAY) ||
       ((res = mp_mul_2d(&tb, n, &tb)) != MP_OKAY) ||
       ((res = mp_mul_2d(&tq, n, &tq)) != MP_OKAY)) {
      goto LBL_ERR;
   }

   while (n-- >= 0) {
      if (mp_cmp(&tb, &ta) != MP_GT) {
         if (((res = mp_sub(&ta, &tb, &ta)) != MP_OKAY) ||
             ((res = mp_add(&q, &tq, &q)) != MP_OKAY)) {
            goto LBL_ERR;
         }
      }
      if (((res = mp_div_2d(&tb, 1, &tb, NULL)) != MP_OKAY) ||
          ((res = mp_div_2d(&tq, 1, &tq, NULL)) != MP_OKAY)) {
         goto LBL_ERR;
      }
   }

   /* now q == quotient and ta == remainder */
   n  = a->sign;
   n2 = (a->sign == b->sign) ? MP_ZPOS : MP_NEG;
   if (c != NULL) {
      mp_exch(c, &q);
      c->sign  = (mp_iszero(c) == MP_YES) ? MP_ZPOS : n2;
   }
   if (d != NULL) {
      mp_exch(d, &ta);
      d->sign = (mp_iszero(d) == MP_YES) ? MP_ZPOS : n;
   }
LBL_ERR:
   mp_clear_multi(&ta, &tb, &tq, &q, NULL);
   return res;
}

/* returns the number of bits in an int */
int mp_count_bits(const mp_int *a)
{
   int     r;
   mp_digit q;

   /* shortcut */
   if (a->used == 0) {
      return 0;
   }

   /* get number of digits and add that */
   r = (a->used - 1) * DIGIT_BIT;

   /* take the last digit and count the bits in it */
   q = a->dp[a->used - 1];
   while (q > (mp_digit)0) {
      ++r;
      q >>= (mp_digit)1;
   }
   return r;
}

/* b = |a|
 *
 * Simple function copies the input and fixes the sign to positive
 */
int mp_abs(const mp_int *a, mp_int *b)
{
   int     res;

   /* copy a to b */
   if (a != b) {
      if ((res = mp_copy(a, b)) != MP_OKAY) {
         return res;
      }
   }

   /* force the sign of b to positive */
   b->sign = MP_ZPOS;

   return MP_OKAY;
}

/* compare two ints (signed)*/
int mp_cmp(const mp_int *a, const mp_int *b)
{
   /* compare based on sign */
   if (a->sign != b->sign) {
      if (a->sign == MP_NEG) {
         return MP_LT;
      } else {
         return MP_GT;
      }
   }

   /* compare digits */
   if (a->sign == MP_NEG) {
      /* if negative compare opposite direction */
      return mp_cmp_mag(b, a);
   } else {
      return mp_cmp_mag(a, b);
   }
}

/* high level subtraction (handles signs) */
int mp_sub(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     sa, sb, res;

   sa = a->sign;
   sb = b->sign;

   if (sa != sb) {
      /* subtract a negative from a positive, OR */
      /* subtract a positive from a negative. */
      /* In either case, ADD their magnitudes, */
      /* and use the sign of the first number. */
      c->sign = sa;
      res = s_mp_add(a, b, c);
   } else {
      /* subtract a positive from a positive, OR */
      /* subtract a negative from a negative. */
      /* First, take the difference between their */
      /* magnitudes, then... */
      if (mp_cmp_mag(a, b) != MP_LT) {
         /* Copy the sign from the first */
         c->sign = sa;
         /* The first has a larger or equal magnitude */
         res = s_mp_sub(a, b, c);
      } else {
         /* The result has the *opposite* sign from */
         /* the first number. */
         c->sign = (sa == MP_ZPOS) ? MP_NEG : MP_ZPOS;
         /* The second has a larger magnitude */
         res = s_mp_sub(b, a, c);
      }
   }
   return res;
}

/* high level addition (handles signs) */
int mp_add(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     sa, sb, res;

   /* get sign of both inputs */
   sa = a->sign;
   sb = b->sign;

   /* handle two cases, not four */
   if (sa == sb) {
      /* both positive or both negative */
      /* add their magnitudes, copy the sign */
      c->sign = sa;
      res = s_mp_add(a, b, c);
   } else {
      /* one positive, the other negative */
      /* subtract the one with the greater magnitude from */
      /* the one of the lesser magnitude.  The result gets */
      /* the sign of the one with the greater magnitude. */
      if (mp_cmp_mag(a, b) == MP_LT) {
         c->sign = sb;
         res = s_mp_sub(b, a, c);
      } else {
         c->sign = sa;
         res = s_mp_sub(a, b, c);
      }
   }
   return res;
}

/* shift right by a certain bit count (store quotient in c, optional remainder in d) */
int mp_div_2d(const mp_int *a, int b, mp_int *c, mp_int *d)
{
   mp_digit D, r, rr;
   int     x, res;

   /* if the shift count is <= 0 then we do no work */
   if (b <= 0) {
      res = mp_copy(a, c);
      if (d != NULL) {
         mp_zero(d);
      }
      return res;
   }

   /* copy */
   if ((res = mp_copy(a, c)) != MP_OKAY) {
      return res;
   }
   /* 'a' should not be used after here - it might be the same as d */

   /* get the remainder */
   if (d != NULL) {
      if ((res = mp_mod_2d(a, b, d)) != MP_OKAY) {
         return res;
      }
   }

   /* shift by as many digits in the bit count */
   if (b >= DIGIT_BIT) {
      mp_rshd(c, b / DIGIT_BIT);
   }

   /* shift any bit count < DIGIT_BIT */
   D = (mp_digit)(b % DIGIT_BIT);
   if (D != 0u) {
      mp_digit *tmpc, mask, shift;

      /* mask */
      mask = ((mp_digit)1 << D) - 1uL;

      /* shift for lsb */
      shift = (mp_digit)DIGIT_BIT - D;

      /* alias */
      tmpc = c->dp + (c->used - 1);

      /* carry */
      r = 0;
      for (x = c->used - 1; x >= 0; x--) {
         /* get the lower  bits of this word in a temp */
         rr = *tmpc & mask;

         /* shift the current word and mix in the carry bits from the previous word */
         *tmpc = (*tmpc >> D) | (r << shift);
         --tmpc;

         /* set the carry to the carry bits of the current word found above */
         r = rr;
      }
   }
   mp_clamp(c);
   return MP_OKAY;
}

/* single digit addition */
int mp_add_d(const mp_int *a, mp_digit b, mp_int *c)
{
   int     res, ix, oldused;
   mp_digit *tmpa, *tmpc, mu;

   /* grow c as required */
   if (c->alloc < (a->used + 1)) {
      if ((res = mp_grow(c, a->used + 1)) != MP_OKAY) {
         return res;
      }
   }

   /* if a is negative and |a| >= b, call c = |a| - b */
   if ((a->sign == MP_NEG) && ((a->used > 1) || (a->dp[0] >= b))) {
      mp_int a_ = *a;
      /* temporarily fix sign of a */
      a_.sign = MP_ZPOS;

      /* c = |a| - b */
      res = mp_sub_d(&a_, b, c);

      /* fix sign  */
      c->sign = MP_NEG;

      /* clamp */
      mp_clamp(c);

      return res;
   }

   /* old number of used digits in c */
   oldused = c->used;

   /* source alias */
   tmpa    = a->dp;

   /* destination alias */
   tmpc    = c->dp;

   /* if a is positive */
   if (a->sign == MP_ZPOS) {
      /* add digit, after this we're propagating
       * the carry.
       */
      *tmpc   = *tmpa++ + b;
      mu      = *tmpc >> DIGIT_BIT;
      *tmpc++ &= MP_MASK;

      /* now handle rest of the digits */
      for (ix = 1; ix < a->used; ix++) {
         *tmpc   = *tmpa++ + mu;
         mu      = *tmpc >> DIGIT_BIT;
         *tmpc++ &= MP_MASK;
      }
      /* set final carry */
      ix++;
      *tmpc++  = mu;

      /* setup size */
      c->used = a->used + 1;
   } else {
      /* a was negative and |a| < b */
      c->used  = 1;

      /* the result is a single digit */
      if (a->used == 1) {
         *tmpc++  =  b - a->dp[0];
      } else {
         *tmpc++  =  b;
      }

      /* setup count so the clearing of oldused
       * can fall through correctly
       */
      ix       = 1;
   }

   /* sign always positive */
   c->sign = MP_ZPOS;

   /* now zero to oldused */
   while (ix++ < oldused) {
      *tmpc++ = 0;
   }
   mp_clamp(c);

   return MP_OKAY;
}

/* calc a value mod 2**b */
int mp_mod_2d(const mp_int *a, int b, mp_int *c)
{
   int     x, res;

   /* if b is <= 0 then zero the int */
   if (b <= 0) {
      mp_zero(c);
      return MP_OKAY;
   }

   /* if the modulus is larger than the value than return */
   if (b >= (a->used * DIGIT_BIT)) {
      res = mp_copy(a, c);
      return res;
   }

   /* copy */
   if ((res = mp_copy(a, c)) != MP_OKAY) {
      return res;
   }

   /* zero digits above the last digit of the modulus */
   for (x = (b / DIGIT_BIT) + (((b % DIGIT_BIT) == 0) ? 0 : 1); x < c->used; x++) {
      c->dp[x] = 0;
   }
   /* clear the digit that is not completely outside/inside the modulus */
   c->dp[b / DIGIT_BIT] &=
      ((mp_digit)1 << (mp_digit)(b % DIGIT_BIT)) - (mp_digit)1;
   mp_clamp(c);
   return MP_OKAY;
}

/* low level subtraction (assumes |a| > |b|), HAC pp.595 Algorithm 14.9 */
int s_mp_sub(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     olduse, res, min, max;

   /* find sizes */
   min = b->used;
   max = a->used;

   /* init result */
   if (c->alloc < max) {
      if ((res = mp_grow(c, max)) != MP_OKAY) {
         return res;
      }
   }
   olduse = c->used;
   c->used = max;

   {
      mp_digit u, *tmpa, *tmpb, *tmpc;
      int i;

      /* alias for digit pointers */
      tmpa = a->dp;
      tmpb = b->dp;
      tmpc = c->dp;

      /* set carry to zero */
      u = 0;
      for (i = 0; i < min; i++) {
         /* T[i] = A[i] - B[i] - U */
         *tmpc = (*tmpa++ - *tmpb++) - u;

         /* U = carry bit of T[i]
          * Note this saves performing an AND operation since
          * if a carry does occur it will propagate all the way to the
          * MSB.  As a result a single shift is enough to get the carry
          */
         u = *tmpc >> (((size_t)CHAR_BIT * sizeof(mp_digit)) - 1u);

         /* Clear carry from T[i] */
         *tmpc++ &= MP_MASK;
      }

      /* now copy higher words if any, e.g. if A has more digits than B  */
      for (; i < max; i++) {
         /* T[i] = A[i] - U */
         *tmpc = *tmpa++ - u;

         /* U = carry bit of T[i] */
         u = *tmpc >> (((size_t)CHAR_BIT * sizeof(mp_digit)) - 1u);

         /* Clear carry from T[i] */
         *tmpc++ &= MP_MASK;
      }

      /* clear digits above used (since we may not have grown result above) */
      for (i = c->used; i < olduse; i++) {
         *tmpc++ = 0;
      }
   }

   mp_clamp(c);
   return MP_OKAY;
}

/* high level multiplication (handles sign) */
int mp_mul(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     res, neg;
   neg = (a->sign == b->sign) ? MP_ZPOS : MP_NEG;

   /* use Toom-Cook? */
#ifdef BN_MP_TOOM_MUL_C
   if (MIN(a->used, b->used) >= TOOM_MUL_CUTOFF) {
      res = mp_toom_mul(a, b, c);
   } else
#endif
#ifdef BN_MP_KARATSUBA_MUL_C
      /* use Karatsuba? */
      if (MIN(a->used, b->used) >= KARATSUBA_MUL_CUTOFF) {
         res = mp_karatsuba_mul(a, b, c);
      } else
#endif
      {
         /* can we use the fast multiplier?
          *
          * The fast multiplier can be used if the output will
          * have less than MP_WARRAY digits and the number of
          * digits won't affect carry propagation
          */
         int     digs = a->used + b->used + 1;

#ifdef BN_FAST_S_MP_MUL_DIGS_C
         if ((digs < (int)MP_WARRAY) &&
             (MIN(a->used, b->used) <=
              (int)(1u << (((size_t)CHAR_BIT * sizeof(mp_word)) - (2u * (size_t)DIGIT_BIT))))) {
            res = fast_s_mp_mul_digs(a, b, c, digs);
         } else
#endif
         {
#ifdef BN_S_MP_MUL_DIGS_C
            res = s_mp_mul(a, b, c); /* uses s_mp_mul_digs */
#else
            res = MP_VAL;
#endif
         }
      }
   c->sign = (c->used > 0) ? neg : MP_ZPOS;
   return res;
}

/* clear one (frees)  */
void mp_clear(mp_int *a)
{
   int i;

   /* only do anything if a hasn't been freed previously */
   if (a->dp != NULL) {
      /* first zero the digits */
      for (i = 0; i < a->used; i++) {
         a->dp[i] = 0;
      }

      /* free ram */
      XFREE(a->dp);

      /* reset members to make debugging easier */
      a->dp    = NULL;
      a->alloc = a->used = 0;
      a->sign  = MP_ZPOS;
   }
}

/* hac 14.61, pp608 */
int mp_invmod(const mp_int *a, const mp_int *b, mp_int *c)
{
   /* b cannot be negative and has to be >1 */
   if ((b->sign == MP_NEG) || (mp_cmp_d(b, 1uL) != MP_GT)) {
      return MP_VAL;
   }
   return mp_invmod_slow(a, b, c);
}

/* c = a mod b, 0 <= c < b if b > 0, b < c <= 0 if b < 0 */
int mp_mod(const mp_int *a, const mp_int *b, mp_int *c)
{
   mp_int  t;
   int     res;

   if ((res = mp_init_size(&t, b->used)) != MP_OKAY) {
      return res;
   }

   if ((res = mp_div(a, b, NULL, &t)) != MP_OKAY) {
      mp_clear(&t);
      return res;
   }

   if ((mp_iszero(&t) != MP_NO) || (t.sign == b->sign)) {
      res = MP_OKAY;
      mp_exch(&t, c);
   } else {
      res = mp_add(b, &t, c);
   }

   mp_clear(&t);
   return res;
}

/* hac 14.61, pp608 */
int mp_invmod_slow(const mp_int *a, const mp_int *b, mp_int *c)
{
   mp_int  x, y, u, v, A, B, C, D;
   int     res;

   /* b cannot be negative */
   if ((b->sign == MP_NEG) || (mp_iszero(b) == MP_YES)) {
      return MP_VAL;
   }

   /* init temps */
   if ((res = mp_init_multi(&x, &y, &u, &v,
                            &A, &B, &C, &D, NULL)) != MP_OKAY) {
      return res;
   }

   /* x = a, y = b */
   if ((res = mp_mod(a, b, &x)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((res = mp_copy(b, &y)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* 2. [modified] if x,y are both even then return an error! */
   if ((mp_iseven(&x) == MP_YES) && (mp_iseven(&y) == MP_YES)) {
      res = MP_VAL;
      goto LBL_ERR;
   }

   /* 3. u=x, v=y, A=1, B=0, C=0,D=1 */
   if ((res = mp_copy(&x, &u)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((res = mp_copy(&y, &v)) != MP_OKAY) {
      goto LBL_ERR;
   }
   mp_set(&A, 1uL);
   mp_set(&D, 1uL);

top:
   /* 4.  while u is even do */
   while (mp_iseven(&u) == MP_YES) {
      /* 4.1 u = u/2 */
      if ((res = mp_div_2(&u, &u)) != MP_OKAY) {
         goto LBL_ERR;
      }
      /* 4.2 if A or B is odd then */
      if ((mp_isodd(&A) == MP_YES) || (mp_isodd(&B) == MP_YES)) {
         /* A = (A+y)/2, B = (B-x)/2 */
         if ((res = mp_add(&A, &y, &A)) != MP_OKAY) {
            goto LBL_ERR;
         }
         if ((res = mp_sub(&B, &x, &B)) != MP_OKAY) {
            goto LBL_ERR;
         }
      }
      /* A = A/2, B = B/2 */
      if ((res = mp_div_2(&A, &A)) != MP_OKAY) {
         goto LBL_ERR;
      }
      if ((res = mp_div_2(&B, &B)) != MP_OKAY) {
         goto LBL_ERR;
      }
   }

   /* 5.  while v is even do */
   while (mp_iseven(&v) == MP_YES) {
      /* 5.1 v = v/2 */
      if ((res = mp_div_2(&v, &v)) != MP_OKAY) {
         goto LBL_ERR;
      }
      /* 5.2 if C or D is odd then */
      if ((mp_isodd(&C) == MP_YES) || (mp_isodd(&D) == MP_YES)) {
         /* C = (C+y)/2, D = (D-x)/2 */
         if ((res = mp_add(&C, &y, &C)) != MP_OKAY) {
            goto LBL_ERR;
         }
         if ((res = mp_sub(&D, &x, &D)) != MP_OKAY) {
            goto LBL_ERR;
         }
      }
      /* C = C/2, D = D/2 */
      if ((res = mp_div_2(&C, &C)) != MP_OKAY) {
         goto LBL_ERR;
      }
      if ((res = mp_div_2(&D, &D)) != MP_OKAY) {
         goto LBL_ERR;
      }
   }

   /* 6.  if u >= v then */
   if (mp_cmp(&u, &v) != MP_LT) {
      /* u = u - v, A = A - C, B = B - D */
      if ((res = mp_sub(&u, &v, &u)) != MP_OKAY) {
         goto LBL_ERR;
      }

      if ((res = mp_sub(&A, &C, &A)) != MP_OKAY) {
         goto LBL_ERR;
      }

      if ((res = mp_sub(&B, &D, &B)) != MP_OKAY) {
         goto LBL_ERR;
      }
   } else {
      /* v - v - u, C = C - A, D = D - B */
      if ((res = mp_sub(&v, &u, &v)) != MP_OKAY) {
         goto LBL_ERR;
      }

      if ((res = mp_sub(&C, &A, &C)) != MP_OKAY) {
         goto LBL_ERR;
      }

      if ((res = mp_sub(&D, &B, &D)) != MP_OKAY) {
         goto LBL_ERR;
      }
   }

   /* if not zero goto step 4 */
   if (mp_iszero(&u) == MP_NO)
      goto top;

   /* now a = C, b = D, gcd == g*v */

   /* if v != 1 then there is no inverse */
   if (mp_cmp_d(&v, 1uL) != MP_EQ) {
      res = MP_VAL;
      goto LBL_ERR;
   }

   /* if its too low */
   while (mp_cmp_d(&C, 0uL) == MP_LT) {
      if ((res = mp_add(&C, b, &C)) != MP_OKAY) {
         goto LBL_ERR;
      }
   }

   /* too big */
   while (mp_cmp_mag(&C, b) != MP_LT) {
      if ((res = mp_sub(&C, b, &C)) != MP_OKAY) {
         goto LBL_ERR;
      }
   }

   /* C is now the inverse */
   mp_exch(&C, c);
   res = MP_OKAY;
LBL_ERR:
   mp_clear_multi(&x, &y, &u, &v, &A, &B, &C, &D, NULL);
   return res;
}

/* b = a/2 */
int mp_div_2(const mp_int *a, mp_int *b)
{
   int     x, res, oldused;

   /* copy */
   if (b->alloc < a->used) {
      if ((res = mp_grow(b, a->used)) != MP_OKAY) {
         return res;
      }
   }

   oldused = b->used;
   b->used = a->used;
   {
      mp_digit r, rr, *tmpa, *tmpb;

      /* source alias */
      tmpa = a->dp + b->used - 1;

      /* dest alias */
      tmpb = b->dp + b->used - 1;

      /* carry */
      r = 0;
      for (x = b->used - 1; x >= 0; x--) {
         /* get the carry for the next iteration */
         rr = *tmpa & 1u;

         /* shift the current digit, add in carry and store */
         *tmpb-- = (*tmpa-- >> 1) | (r << (DIGIT_BIT - 1));

         /* forward carry to next iteration */
         r = rr;
      }

      /* zero excess digits */
      tmpb = b->dp + b->used;
      for (x = b->used; x < oldused; x++) {
         *tmpb++ = 0;
      }
   }
   b->sign = a->sign;
   mp_clamp(b);
   return MP_OKAY;
}

/* set to a digit */
void mp_set(mp_int *a, mp_digit b)
{
   mp_zero(a);
   a->dp[0] = b & MP_MASK;
   a->used  = (a->dp[0] != 0u) ? 1 : 0;
}

/* init a new mp_int */
int mp_init(mp_int *a)
{
   int i;

   /* allocate memory required and clear it */
   a->dp = XMALLOC(sizeof(mp_digit) * (size_t)MP_PREC);
   if (a->dp == NULL) {
      return MP_MEM;
   }

   /* set the digits to zero */
   for (i = 0; i < MP_PREC; i++) {
      a->dp[i] = 0;
   }

   /* set the used to zero, allocated digits to the default precision
    * and sign to positive */
   a->used  = 0;
   a->alloc = MP_PREC;
   a->sign  = MP_ZPOS;

   return MP_OKAY;
}

/* swap the elements of two integers, for cases where you can't simply swap the
 * mp_int pointers around
 */
void mp_exch(mp_int *a, mp_int *b)
{
   mp_int  t;

   t  = *a;
   *a = *b;
   *b = t;
}

/* low level addition, based on HAC pp.594, Algorithm 14.7 */
int s_mp_add(const mp_int *a, const mp_int *b, mp_int *c)
{
   const mp_int *x;
   int     olduse, res, min, max;

   /* find sizes, we let |a| <= |b| which means we have to sort
    * them.  "x" will point to the input with the most digits
    */
   if (a->used > b->used) {
      min = b->used;
      max = a->used;
      x = a;
   } else {
      min = a->used;
      max = b->used;
      x = b;
   }

   /* init result */
   if (c->alloc < (max + 1)) {
      if ((res = mp_grow(c, max + 1)) != MP_OKAY) {
         return res;
      }
   }

   /* get old used digit count and set new one */
   olduse = c->used;
   c->used = max + 1;

   {
      mp_digit u, *tmpa, *tmpb, *tmpc;
      int i;

      /* alias for digit pointers */

      /* first input */
      tmpa = a->dp;

      /* second input */
      tmpb = b->dp;

      /* destination */
      tmpc = c->dp;

      /* zero the carry */
      u = 0;
      for (i = 0; i < min; i++) {
         /* Compute the sum at one digit, T[i] = A[i] + B[i] + U */
         *tmpc = *tmpa++ + *tmpb++ + u;

         /* U = carry bit of T[i] */
         u = *tmpc >> (mp_digit)DIGIT_BIT;

         /* take away carry bit from T[i] */
         *tmpc++ &= MP_MASK;
      }

      /* now copy higher words if any, that is in A+B
       * if A or B has more digits add those in
       */
      if (min != max) {
         for (; i < max; i++) {
            /* T[i] = X[i] + U */
            *tmpc = x->dp[i] + u;

            /* U = carry bit of T[i] */
            u = *tmpc >> (mp_digit)DIGIT_BIT;

            /* take away carry bit from T[i] */
            *tmpc++ &= MP_MASK;
         }
      }

      /* add carry */
      *tmpc++ = u;

      /* clear digits above oldused */
      for (i = c->used; i < olduse; i++) {
         *tmpc++ = 0;
      }
   }

   mp_clamp(c);
   return MP_OKAY;
}

/* shift right a certain amount of digits */
void mp_rshd(mp_int *a, int b)
{
   int     x;

   /* if b <= 0 then ignore it */
   if (b <= 0) {
      return;
   }

   /* if b > used then simply zero it and return */
   if (a->used <= b) {
      mp_zero(a);
      return;
   }

   {
      mp_digit *bottom, *top;

      /* shift the digits down */

      /* bottom */
      bottom = a->dp;

      /* top [offset into digits] */
      top = a->dp + b;

      /* this is implemented as a sliding window where
       * the window is b-digits long and digits from
       * the top of the window are copied to the bottom
       *
       * e.g.

       b-2 | b-1 | b0 | b1 | b2 | ... | bb |   ---->
                   /\                   |      ---->
                    \-------------------/      ---->
       */
      for (x = 0; x < (a->used - b); x++) {
         *bottom++ = *top++;
      }

      /* zero the top digits */
      for (; x < a->used; x++) {
         *bottom++ = 0;
      }
   }

   /* remove excess digits */
   a->used -= b;
}

const mp_digit ltm_prime_tab[] = {
   0x0002, 0x0003, 0x0005, 0x0007, 0x000B, 0x000D, 0x0011, 0x0013,
   0x0017, 0x001D, 0x001F, 0x0025, 0x0029, 0x002B, 0x002F, 0x0035,
   0x003B, 0x003D, 0x0043, 0x0047, 0x0049, 0x004F, 0x0053, 0x0059,
   0x0061, 0x0065, 0x0067, 0x006B, 0x006D, 0x0071, 0x007F,
};

/* performs a variable number of rounds of Miller-Rabin
 *
 * Probability of error after t rounds is no more than

 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
int mp_prime_is_prime(const mp_int *a, int t, int *result)
{
   mp_int  b;
   int     ix, err, res;

   /* default to no */
   *result = MP_NO;

   /* valid value of t? */
   if ((t <= 0) || (t > PRIME_SIZE)) {
      return MP_VAL;
   }

   /* is the input equal to one of the primes in the table? */
   for (ix = 0; ix < PRIME_SIZE; ix++) {
      if (mp_cmp_d(a, ltm_prime_tab[ix]) == MP_EQ) {
         *result = 1;
         return MP_OKAY;
      }
   }

   /* first perform trial division */
   if ((err = mp_prime_is_divisible(a, &res)) != MP_OKAY) {
      return err;
   }

   /* return if it was trivially divisible */
   if (res == MP_YES) {
      return MP_OKAY;
   }

   /* now perform the miller-rabin rounds */
   if ((err = mp_init(&b)) != MP_OKAY) {
      return err;
   }

   for (ix = 0; ix < t; ix++) {
      /* set the prime */
      mp_set(&b, ltm_prime_tab[ix]);

      if ((err = mp_prime_miller_rabin(a, &b, &res)) != MP_OKAY) {
         goto LBL_B;
      }

      if (res == MP_NO) {
         goto LBL_B;
      }
   }

   /* passed the test */
   *result = MP_YES;
LBL_B:
   mp_clear(&b);
   return err;
}

/* determines if an integers is divisible by one
 * of the first PRIME_SIZE primes or not
 *
 * sets result to 0 if not, 1 if yes
 */
int mp_prime_is_divisible(const mp_int *a, int *result)
{
   int     err, ix;
   mp_digit res;

   /* default to not */
   *result = MP_NO;

   for (ix = 0; ix < PRIME_SIZE; ix++) {
      /* what is a mod LBL_prime_tab[ix] */
      if ((err = mp_mod_d(a, ltm_prime_tab[ix], &res)) != MP_OKAY) {
         return err;
      }

      /* is the residue zero? */
      if (res == 0u) {
         *result = MP_YES;
         return MP_OKAY;
      }
   }

   return MP_OKAY;
}

/* Miller-Rabin test of "a" to the base of "b" as described in
 * HAC pp. 139 Algorithm 4.24
 *
 * Sets result to 0 if definitely composite or 1 if probably prime.
 * Randomly the chance of error is no more than 1/4 and often
 * very much lower.
 */
int mp_prime_miller_rabin(const mp_int *a, const mp_int *b, int *result)
{
   mp_int  n1, y, r;
   int     s, j, err;

   /* default */
   *result = MP_NO;

   /* ensure b > 1 */
   if (mp_cmp_d(b, 1uL) != MP_GT) {
      return MP_VAL;
   }

   /* get n1 = a - 1 */
   if ((err = mp_init_copy(&n1, a)) != MP_OKAY) {
      return err;
   }
   if ((err = mp_sub_d(&n1, 1uL, &n1)) != MP_OKAY) {
      goto LBL_N1;
   }

   /* set 2**s * r = n1 */
   if ((err = mp_init_copy(&r, &n1)) != MP_OKAY) {
      goto LBL_N1;
   }

   /* count the number of least significant bits
    * which are zero
    */
   s = mp_cnt_lsb(&r);

   /* now divide n - 1 by 2**s */
   if ((err = mp_div_2d(&r, s, &r, NULL)) != MP_OKAY) {
      goto LBL_R;
   }

   /* compute y = b**r mod a */
   if ((err = mp_init(&y)) != MP_OKAY) {
      goto LBL_R;
   }
   if ((err = mp_exptmod(b, &r, a, &y)) != MP_OKAY) {
      goto LBL_Y;
   }

   /* if y != 1 and y != n1 do */
   if ((mp_cmp_d(&y, 1uL) != MP_EQ) && (mp_cmp(&y, &n1) != MP_EQ)) {
      j = 1;
      /* while j <= s-1 and y != n1 */
      while ((j <= (s - 1)) && (mp_cmp(&y, &n1) != MP_EQ)) {
         if ((err = mp_sqrmod(&y, a, &y)) != MP_OKAY) {
            goto LBL_Y;
         }

         /* if y == 1 then composite */
         if (mp_cmp_d(&y, 1uL) == MP_EQ) {
            goto LBL_Y;
         }

         ++j;
      }

      /* if y != n1 then composite */
      if (mp_cmp(&y, &n1) != MP_EQ) {
         goto LBL_Y;
      }
   }

   /* probably prime now */
   *result = MP_YES;
LBL_Y:
   mp_clear(&y);
LBL_R:
   mp_clear(&r);
LBL_N1:
   mp_clear(&n1);
   return err;
}

/* c = a * a (mod b) */
int mp_sqrmod(const mp_int *a, const mp_int *b, mp_int *c)
{
   int     res;
   mp_int  t;

   if ((res = mp_init(&t)) != MP_OKAY) {
      return res;
   }

   if ((res = mp_sqr(a, &t)) != MP_OKAY) {
      mp_clear(&t);
      return res;
   }
   res = mp_mod(&t, b, c);
   mp_clear(&t);
   return res;
}
