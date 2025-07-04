/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain

   Test Vectors (from FIPS PUB 180-1)
   "abc"
   A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
   84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
   A million repetitions of "a"
   34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

/* #define SHA1HANDSOFF * Copies data before messing with it. */

#ifdef HAVE_CONFIG_H
# include "tdconfig.h"
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#include "sha1.h"
#ifdef _WIN32
# include <windows.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

static void sha1_transform (unsigned int state[5], unsigned char buffer[64]);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

# define blk0(i) (block->l[i] = htonl (block->l[i]))

#define blk(i) (block->l[i & 15] = rol (block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] \
    ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0 + R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R1(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk (i) + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R2(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); w = rol(w, 30);
#define R4(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); w = rol(w, 30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

static void sha1_transform (unsigned int state[5], unsigned char buffer[64])
{
	unsigned int a, b, c, d, e;

	typedef union {
		unsigned char c[64];
		unsigned int l[16];
	} CHAR64LONG16;

	CHAR64LONG16 * block;

#ifdef SHA1HANDSOFF
	static unsigned char workspace[64];
	block = (CHAR64LONG16 *)workspace;
	memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16 *)buffer;
#endif

	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	/* Wipe variables */
	a = b = c = d = e = 0;
}


/* sha1_init - Initialize new context */

void sha1_init(SHA1_CTX * context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void sha1_update(SHA1_CTX * context, unsigned char * data, size_t len)
{
	unsigned int i, j;

	j = (context->count[0] >> 3) & 63;

	if ((context->count[0] += (unsigned int)len << 3) < (len << 3))
		context->count[1]++;

	context->count[1] += (unsigned int)(len >> 29);

	if ((j + len) > 63)
	{
		memcpy (&context->buffer[j], data, (i = 64 - j));
		sha1_transform (context->state, context->buffer);

		for ( ; i + 63 < len; i += 64)
		{
			memcpy (context->buffer, &data[i], 64);
			sha1_transform (context->state, context->buffer);
		}

		j = 0;
	}
	else
	{
		i = 0;
	}

	memcpy (&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void sha1_final(unsigned char digest[20], SHA1_CTX * context)
{
	unsigned int i, j;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++)
	{
		finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
					>> ((3 - (i & 3)) * 8) ) & 255);  /* Endian independent */
	}

	sha1_update (context, (unsigned char *)"\200", 1);

	while ((context->count[0] & 504) != 448)
	{
		sha1_update (context, (unsigned char *)"\0", 1);
	}

	sha1_update (context, finalcount, 8);  /* Should cause a sha1_transform() */

	for (i = 0; i < 20; i++)
	{
		digest[i] = (unsigned char)
			((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}

	/* Wipe variables */
	i = j = 0;
	memset(context->buffer, 0, 64);
	memset(context->state, 0, 20);
	memset(context->count, 0, 8);
	memset(&finalcount, 0, 8);

#ifdef SHA1HANDSOFF  /* make sha1_transform overwrite it's own static vars */
	sha1_transform(context->state, context->buffer);
#endif
}
