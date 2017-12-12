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

#include <string.h>
#include "aes.h"
#include "aes_tab.c"

static ulong32 setup_mix(ulong32 temp)
{
   return (Te4_3[BYTE(temp, 2)]) ^
          (Te4_2[BYTE(temp, 1)]) ^
          (Te4_1[BYTE(temp, 0)]) ^
          (Te4_0[BYTE(temp, 3)]);
}

int aes_key_setup(unsigned char *key, unsigned int keylen, aes_key *skey)
{
	int i;
	unsigned int temp, *rk;
	unsigned int *rrk;

	/* cannot work with null pointers */
	if (skey == NULL || key == NULL)
	{
		return CRYPTO_INVALID_STRUCT;
	}

	/* keylen must be a specific size */
	if (keylen != 16 && keylen != 24 && keylen != 32)
	{
	   return CRYPTO_INVALID_KEYSIZE;
	}

	/* number of rounds calculated from the keysize */
	skey->num_rounds = (10 + ((keylen/8)-2)*2);

	/* setup the forward key */
	i                 = 0;
	rk                = skey->eK;
	LOAD32H(rk[0], key     );
	LOAD32H(rk[1], key +  4);
	LOAD32H(rk[2], key +  8);
	LOAD32H(rk[3], key + 12);
	if (keylen == 16)
	{
		for (;;) {
			temp  = rk[3];
			rk[4] = rk[0] ^ setup_mix(temp) ^ rcon[i];
			rk[5] = rk[1] ^ rk[4];
			rk[6] = rk[2] ^ rk[5];
			rk[7] = rk[3] ^ rk[6];
			if (++i == 10) {
			   break;
			}
			rk += 4;
		}
	}
	else if (keylen == 24)
	{
		LOAD32H(rk[4], key + 16);
		LOAD32H(rk[5], key + 20);
		for (;;) {
			temp = rk[5];
			rk[ 6] = rk[ 0] ^ setup_mix(temp) ^ rcon[i];
			rk[ 7] = rk[ 1] ^ rk[ 6];
			rk[ 8] = rk[ 2] ^ rk[ 7];
			rk[ 9] = rk[ 3] ^ rk[ 8];
			if (++i == 8) {
				break;
			}
			rk[10] = rk[ 4] ^ rk[ 9];
			rk[11] = rk[ 5] ^ rk[10];
			rk += 6;
		}
	}
	else if (keylen == 32)
	{
		LOAD32H(rk[4], key + 16);
		LOAD32H(rk[5], key + 20);
		LOAD32H(rk[6], key + 24);
		LOAD32H(rk[7], key + 28);
		for (;;) {
			temp = rk[7];
			rk[ 8] = rk[ 0] ^ setup_mix(temp) ^ rcon[i];
			rk[ 9] = rk[ 1] ^ rk[ 8];
			rk[10] = rk[ 2] ^ rk[ 9];
			rk[11] = rk[ 3] ^ rk[10];
			if (++i == 7)
			{
				break;
			}
			temp = rk[11];
			rk[12] = rk[ 4] ^ setup_mix(RORc(temp, 8));
			rk[13] = rk[ 5] ^ rk[12];
			rk[14] = rk[ 6] ^ rk[13];
			rk[15] = rk[ 7] ^ rk[14];
			rk += 8;
		}
	}

	 /* setup the inverse key now */
	rk   = skey->dK;
	rrk  = skey->eK + (28 + keylen) - 4;

	/* apply the inverse MixColumn transform to all round keys but the first and the last: */
	/* copy first */
	*rk++ = *rrk++;
	*rk++ = *rrk++;
	*rk++ = *rrk++;
	*rk   = *rrk;
	rk -= 3; rrk -= 3;

	for (i = 1; i < skey->num_rounds; i++) {
		rrk -= 4;
		rk  += 4;
		temp = rrk[0];
		rk[0] =
			Tks0[BYTE(temp, 3)] ^
			Tks1[BYTE(temp, 2)] ^
			Tks2[BYTE(temp, 1)] ^
			Tks3[BYTE(temp, 0)];
		temp = rrk[1];
		rk[1] =
			Tks0[BYTE(temp, 3)] ^
			Tks1[BYTE(temp, 2)] ^
			Tks2[BYTE(temp, 1)] ^
			Tks3[BYTE(temp, 0)];
		temp = rrk[2];
		rk[2] =
			Tks0[BYTE(temp, 3)] ^
			Tks1[BYTE(temp, 2)] ^
			Tks2[BYTE(temp, 1)] ^
			Tks3[BYTE(temp, 0)];
		temp = rrk[3];
		rk[3] =
			Tks0[BYTE(temp, 3)] ^
			Tks1[BYTE(temp, 2)] ^
			Tks2[BYTE(temp, 1)] ^
			Tks3[BYTE(temp, 0)];
	}

	/* copy last */
	rrk -= 4;
	rk  += 4;
	*rk++ = *rrk++;
	*rk++ = *rrk++;
	*rk++ = *rrk++;
	*rk   = *rrk;

	return CRYPTO_OK;
}

int aes_block_encrypt(unsigned char *pt, unsigned char *ct, aes_key *key)
{
	ulong32 s0, s1, s2, s3, t0, t1, t2, t3, *rk;
	int Nr, r;

	Nr = key->num_rounds;
	rk = key->eK;

	/*
	 * map BYTE array block to cipher state
	 * and add initial round key:
	 */
	LOAD32H(s0, pt      ); s0 ^= rk[0];
	LOAD32H(s1, pt  +  4); s1 ^= rk[1];
	LOAD32H(s2, pt  +  8); s2 ^= rk[2];
	LOAD32H(s3, pt  + 12); s3 ^= rk[3];

	/*
	 * Nr - 1 full rounds:
	 */
	r = Nr >> 1;
	for (;;) {
		t0 =
			Te0(BYTE(s0, 3)) ^
			Te1(BYTE(s1, 2)) ^
			Te2(BYTE(s2, 1)) ^
			Te3(BYTE(s3, 0)) ^
			rk[4];
		t1 =
			Te0(BYTE(s1, 3)) ^
			Te1(BYTE(s2, 2)) ^
			Te2(BYTE(s3, 1)) ^
			Te3(BYTE(s0, 0)) ^
			rk[5];
		t2 =
			Te0(BYTE(s2, 3)) ^
			Te1(BYTE(s3, 2)) ^
			Te2(BYTE(s0, 1)) ^
			Te3(BYTE(s1, 0)) ^
			rk[6];
		t3 =
			Te0(BYTE(s3, 3)) ^
			Te1(BYTE(s0, 2)) ^
			Te2(BYTE(s1, 1)) ^
			Te3(BYTE(s2, 0)) ^
			rk[7];

		rk += 8;
		if (--r == 0) {
			break;
		}

		s0 =
			Te0(BYTE(t0, 3)) ^
			Te1(BYTE(t1, 2)) ^
			Te2(BYTE(t2, 1)) ^
			Te3(BYTE(t3, 0)) ^
			rk[0];
		s1 =
			Te0(BYTE(t1, 3)) ^
			Te1(BYTE(t2, 2)) ^
			Te2(BYTE(t3, 1)) ^
			Te3(BYTE(t0, 0)) ^
			rk[1];
		s2 =
			Te0(BYTE(t2, 3)) ^
			Te1(BYTE(t3, 2)) ^
			Te2(BYTE(t0, 1)) ^
			Te3(BYTE(t1, 0)) ^
			rk[2];
		s3 =
			Te0(BYTE(t3, 3)) ^
			Te1(BYTE(t0, 2)) ^
			Te2(BYTE(t1, 1)) ^
			Te3(BYTE(t2, 0)) ^
			rk[3];
	}

	/*
	 * apply last round and
	 * map cipher state to BYTE array block:
	 */
	s0 =
		(Te4_3[BYTE(t0, 3)]) ^
		(Te4_2[BYTE(t1, 2)]) ^
		(Te4_1[BYTE(t2, 1)]) ^
		(Te4_0[BYTE(t3, 0)]) ^
		rk[0];
	STORE32H(s0, ct);
	s1 =
		(Te4_3[BYTE(t1, 3)]) ^
		(Te4_2[BYTE(t2, 2)]) ^
		(Te4_1[BYTE(t3, 1)]) ^
		(Te4_0[BYTE(t0, 0)]) ^
		rk[1];
	STORE32H(s1, ct+4);
	s2 =
		(Te4_3[BYTE(t2, 3)]) ^
		(Te4_2[BYTE(t3, 2)]) ^
		(Te4_1[BYTE(t0, 1)]) ^
		(Te4_0[BYTE(t1, 0)]) ^
		rk[2];
	STORE32H(s2, ct+8);
	s3 =
		(Te4_3[BYTE(t3, 3)]) ^
		(Te4_2[BYTE(t0, 2)]) ^
		(Te4_1[BYTE(t1, 1)]) ^
		(Te4_0[BYTE(t2, 0)]) ^
		rk[3];
	STORE32H(s3, ct+12);

	return CRYPTO_OK;

}

int aes_block_decrypt(unsigned char *ct, unsigned char *pt, aes_key *key)
{
	ulong32 s0, s1, s2, s3, t0, t1, t2, t3, *rk;
	int Nr, r;

	Nr = key->num_rounds;
	rk = key->dK;

	/*
	 * map BYTE array block to cipher state
	 * and add initial round key:
	 */
	LOAD32H(s0, ct      ); s0 ^= rk[0];
	LOAD32H(s1, ct  +  4); s1 ^= rk[1];
	LOAD32H(s2, ct  +  8); s2 ^= rk[2];
	LOAD32H(s3, ct  + 12); s3 ^= rk[3];

	/*
	 * Nr - 1 full rounds:
	 */
	r = Nr >> 1;
	for (;;) {

		t0 =
			Td0(BYTE(s0, 3)) ^
			Td1(BYTE(s3, 2)) ^
			Td2(BYTE(s2, 1)) ^
			Td3(BYTE(s1, 0)) ^
			rk[4];
		t1 =
			Td0(BYTE(s1, 3)) ^
			Td1(BYTE(s0, 2)) ^
			Td2(BYTE(s3, 1)) ^
			Td3(BYTE(s2, 0)) ^
			rk[5];
		t2 =
			Td0(BYTE(s2, 3)) ^
			Td1(BYTE(s1, 2)) ^
			Td2(BYTE(s0, 1)) ^
			Td3(BYTE(s3, 0)) ^
			rk[6];
		t3 =
			Td0(BYTE(s3, 3)) ^
			Td1(BYTE(s2, 2)) ^
			Td2(BYTE(s1, 1)) ^
			Td3(BYTE(s0, 0)) ^
			rk[7];

		rk += 8;
		if (--r == 0) {
			break;
		}


		s0 =
			Td0(BYTE(t0, 3)) ^
			Td1(BYTE(t3, 2)) ^
			Td2(BYTE(t2, 1)) ^
			Td3(BYTE(t1, 0)) ^
			rk[0];
		s1 =
			Td0(BYTE(t1, 3)) ^
			Td1(BYTE(t0, 2)) ^
			Td2(BYTE(t3, 1)) ^
			Td3(BYTE(t2, 0)) ^
			rk[1];
		s2 =
			Td0(BYTE(t2, 3)) ^
			Td1(BYTE(t1, 2)) ^
			Td2(BYTE(t0, 1)) ^
			Td3(BYTE(t3, 0)) ^
			rk[2];
		s3 =
			Td0(BYTE(t3, 3)) ^
			Td1(BYTE(t2, 2)) ^
			Td2(BYTE(t1, 1)) ^
			Td3(BYTE(t0, 0)) ^
			rk[3];
	}
	/*
	 * apply last round and
	 * map cipher state to BYTE array block:
	 */
	s0 =
		(Td4[BYTE(t0, 3)] & 0xff000000) ^
		(Td4[BYTE(t3, 2)] & 0x00ff0000) ^
		(Td4[BYTE(t2, 1)] & 0x0000ff00) ^
		(Td4[BYTE(t1, 0)] & 0x000000ff) ^
		rk[0];
	STORE32H(s0, pt);
	s1 =
		(Td4[BYTE(t1, 3)] & 0xff000000) ^
		(Td4[BYTE(t0, 2)] & 0x00ff0000) ^
		(Td4[BYTE(t3, 1)] & 0x0000ff00) ^
		(Td4[BYTE(t2, 0)] & 0x000000ff) ^
		rk[1];
	STORE32H(s1, pt+4);
	s2 =
		(Td4[BYTE(t2, 3)] & 0xff000000) ^
		(Td4[BYTE(t1, 2)] & 0x00ff0000) ^
		(Td4[BYTE(t0, 1)] & 0x0000ff00) ^
		(Td4[BYTE(t3, 0)] & 0x000000ff) ^
		rk[2];
	STORE32H(s2, pt+8);
	s3 =
		(Td4[BYTE(t3, 3)] & 0xff000000) ^
		(Td4[BYTE(t2, 2)] & 0x00ff0000) ^
		(Td4[BYTE(t1, 1)] & 0x0000ff00) ^
		(Td4[BYTE(t0, 0)] & 0x000000ff) ^
		rk[3];
	STORE32H(s3, pt+12);

	return CRYPTO_OK;
}

int aes_ecb_encrypt(unsigned char *data, unsigned int len, unsigned char *out, aes_key *key)
{
	unsigned int i;
	unsigned int num_block;
	unsigned char temp[16];

	CONDITION_CHECK((len % 16 == 0), CRYPTO_INVALID_DATA_SIZE)

	num_block = len / 16;

	for (i = 0; i < num_block; i++)
	{
		aes_block_encrypt(data, temp, key);
		XMEMCPY(out, temp, 16);
		out += 16;
	}

	return CRYPTO_OK;
}

int aes_ecb_decrypt(unsigned char *data, unsigned int len, unsigned char *out, aes_key *key)
{
	unsigned int i;
	unsigned int num_block;
	unsigned char temp[16];

	CONDITION_CHECK((len % 16 == 0), CRYPTO_INVALID_DATA_SIZE)

	num_block = len / 16;

	for (i = 0; i < num_block; i++)
	{
		aes_block_decrypt(data, temp, key);
		XMEMCPY(out, temp, 16);
		out += 16;
	}

	return CRYPTO_OK;
}

int aes_cbc_encrypt(unsigned char *data, unsigned char *iv, unsigned int len, unsigned char *out, aes_key *key)
{
	int i;
	unsigned int num_block;
	unsigned char temp[16];
	unsigned char tempXor[16];
	unsigned char tempIv[16];

	CONDITION_CHECK((len > 0), CRYPTO_INVALID_DATA_SIZE);
	CONDITION_CHECK((len % 16 == 0), CRYPTO_INVALID_DATA_SIZE)

	// init the block number and first IV
	num_block = len / 16;
	XMEMCPY(tempIv, iv, 16);

	for (i = 0; i < num_block; i++)
	{
		// encrypt 16 bytes of data
		XOR(data, 16, tempIv, 16, tempXor);
		aes_block_encrypt(tempXor, temp, key);

		// add this to output, move pointer
		XMEMCPY(out, temp, 16);
		data += 16;
		out += 16;

		// setting up the next IV
		XMEMCPY(tempIv, temp, 16);
	}

	// cleanup
	ZEROMEM(temp, 16);
	ZEROMEM(tempXor, 16);
	ZEROMEM(tempIv, 16);

	return CRYPTO_OK;
}

int aes_cbc_decrypt(unsigned char *data, unsigned char *iv, unsigned int len, unsigned char *out, aes_key *key)
{
	int i;
	unsigned int num_block;
	unsigned char temp[16];
	unsigned char tempIv[16];
	unsigned char tempXor[16];

	CONDITION_CHECK((len > 0), CRYPTO_INVALID_DATA_SIZE);
	CONDITION_CHECK((len % 16 == 0), CRYPTO_INVALID_DATA_SIZE);

	num_block = len / 16;
	XMEMCPY(tempIv, iv, 16);

	for (i = 0; i < num_block; i++)
	{
		// decrypt 16 bytes and xor it
		aes_block_decrypt(data, temp, key);
		XOR(temp, 16, tempIv, 16, tempXor);

		// copy the result
		XMEMCPY(out, tempXor, 16);
		out += 16;

		// get the next IV
		XMEMCPY(tempIv, data, 16);
		data += 16;
	}

	ZEROMEM(temp, 16);
	ZEROMEM(tempIv, 16);
	ZEROMEM(tempXor, 16);

	return CRYPTO_OK;
}
