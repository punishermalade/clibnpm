/*
 * main.c
 *
 *  Created on: 22 Oct 2017
 *      Author: punisher
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "crypto.h"
#include "aes.h"
#include "sha.h"
#include "prng.h"
#include "des.h"

void print_byte_array(unsigned char *d, unsigned int s);
void aes_test();
void sha_test();
void sha_compact_test();
void prng_test();
void prng_easy_test();
void xor_test();
void aes_cbc_test();
void des_test();

int main(void)
{
	aes_test();
	sha_test();
	sha_compact_test();
	prng_test();
	prng_easy_test();
	xor_test();
	aes_cbc_test();
	des_test();
}

void print_byte_array(unsigned char *data, unsigned int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		printf("%02X", *data);
		data++;
	}
	printf("\n");

}

void aes_test()
{
	const unsigned int DATA_SIZE = 64;
	unsigned char key[16], data[DATA_SIZE];
	memset(key, 0, 16);
	memset(data, 0x22, DATA_SIZE);
	printf("aes original:  ");
	print_byte_array(data, DATA_SIZE);

	unsigned char out[DATA_SIZE];
	memset(out, 0, DATA_SIZE);

	unsigned char clear[DATA_SIZE];
	memset(clear, 0, DATA_SIZE);

	aes_key skey;
	aes_key_setup(key, 16, &skey);
	aes_ecb_encrypt(data, DATA_SIZE, out, &skey);
	printf("aes encrypted: ");
	print_byte_array(out, DATA_SIZE);

	aes_key dkey;
	aes_key_setup(key, 16, &dkey);
	aes_ecb_decrypt(out, DATA_SIZE, clear, &dkey);
	printf("aes decrypted: ");
	print_byte_array(clear, DATA_SIZE);
}

void sha_test()
{
	const unsigned int data_size = 3;
	/*
	const unsigned char data[data_size] = { 0x03, 0x55, 0x55, 0x53, 0xde, 0x21, 0x45, 0x66,
									0x33, 0x44, 0x55, 0x67, 0x99, 0x56, 0x77, 0xbe };
	*/
	const unsigned char *data = "abc";

	hash_state md;
	unsigned char out[64];
	sha512_init(&md);
	sha512_process(&md, data, data_size);
	sha512_done(&md, out);
	printf("sha512: ");
	print_byte_array(out, 64);

	unsigned char out1[32];
	sha256_init(&md);
	sha256_process(&md, data, data_size);
	sha256_done(&md, out1);
	printf("sha256: ");
	print_byte_array(out1, 32);

	unsigned char out2[20];
	sha1_init(&md);
	sha1_process(&md, data, data_size);
	sha1_done(&md, out2);
	printf("sha1: ");
	print_byte_array(out2, 20);

	unsigned char out3[16];
	md5_init(&md);
	md5_process(&md, data, data_size);
	md5_done(&md, out3);
	printf("md5: ");
	print_byte_array(out3, 16);

}

void sha_compact_test()
{
	const unsigned int datasize = 3;
	unsigned char *data = "abc";

	unsigned char out[64];
	sha512_hash(data, datasize, out);
	printf("SHA512:");
	print_byte_array(out, 64);

	unsigned char out1[32];
	sha256_hash(data, datasize, out1);
	printf("SHA256:");
	print_byte_array(out1, 32);

	unsigned char out2[20];
	sha1_hash(data, datasize, out2);
	printf("SHA1:");
	print_byte_array(out2, SHA1_HASH_SIZE);

	printf("%d\n", SHA1_HASH_SIZE);

	unsigned char out3[16];
	md5_hash(data, datasize, out3);
	printf("MD5:");
	print_byte_array(out3, MD5_HASH_SIZE);
}

void prng_test()
{
	prng_state md;
	fortuna_start(&md);

	unsigned char random[32];
	memset(random, 0x34, 32);
	fortuna_add_entropy(random, 32, &md);

	fortuna_ready(&md);

	const unsigned int loops = 10;
	int i;

	unsigned char out[16];

	for (i = 0; i < loops; i++)
	{
		fortuna_read(out, 16, &md);
		print_byte_array(out, 16);
	}

	fortuna_done(&md);
}

void prng_easy_test()
{
	unsigned char myRandomData[50];
	memset(myRandomData, 0x32, 50);

	int loops = 10, i = 0;
	prng_state ps;
	prng_create(&ps, myRandomData, 50);
	unsigned char out[16];

	for (i = 0; i < loops; i++)
	{
		prng_read(&ps, out, 16, 1);
		print_byte_array(out, 16);
	}

	prng_close(&ps);
}

void xor_test()
{
	unsigned char result[16];
	unsigned char dataA[16] = { 0x11, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char dataB[16] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };

	XOR(dataA, 16, dataB, 16, result);
	print_byte_array(dataA, 16);
	print_byte_array(dataB, 16);
	print_byte_array(result, 16);
}

void aes_cbc_test()
{
	unsigned char aesKey[16] = { 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22 };
	unsigned char iv[16] = { 0x11, 0x21, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char data[64];
	unsigned char result[64];
	unsigned char encrypt[64];

	memset(data, 0x42, 64);
	memset(result, 0, 64);

	aes_key key;
	aes_key_setup(aesKey, 16, &key);
	aes_cbc_encrypt(data, iv, 64, result, &key);

	printf("Original: ");
	print_byte_array(data, 64);
	printf("Encrypted: ");
	print_byte_array(result, 64);

	memcpy(encrypt, result, 64);
	aes_cbc_decrypt(encrypt, iv, 64, result, &key);
	printf("Decrypted: ");
	print_byte_array(result, 64);

	unsigned char iv2[16] = { 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	aes_cbc_encrypt(data, iv2, 64, result, &key);
	print_byte_array(result, 64);

}

void des_test()
{
	unsigned char desKey[24];
	unsigned char data[8];
	unsigned char output[8];

	memset(desKey, 0x22, 24);
	memset(data, 0x54, 8);
	memset(output, 0x00, 8);

	des3_key key;
	des3_setup(desKey, 24, &key);
	des3_encrypt(data, output, &key);

	printf("DES3 plain: ");
	print_byte_array(data, 8);
	printf("DES3 cipher: ");
	print_byte_array(output, 8);

	des3_decrypt(output, data, &key);
	printf("DES3 plain: ");
	print_byte_array(data, 8);
}
