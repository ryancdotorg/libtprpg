/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <netinet/in.h> /* for ntohl/htonl */

#include "tprpg.h"

/* Integer square root by Halleck's method */
static inline int32_t isqrt32(uint32_t x) {
  uint32_t squaredbit;
  uint32_t remainder;
  uint32_t root;

   if (x<1) return 0;
  
   /* We really want to load the binary constant 01 00 00 ... 00, 
    *   (There must be an even number of zeros to the right
    *    of the single one bit, and the one bit as far to the
    *    left as can be done within that constraint)
    */
   remainder = x>>2;
   squaredbit = 1<<30;

   /* Form bits of the answer. */
   remainder = x;  root = 0;
   while (squaredbit > 0) {
     if (remainder >= (squaredbit | root)) {
         remainder -= (squaredbit | root);
         root >>= 1; root |= squaredbit;
     } else {
         root >>= 1;
     }
     squaredbit >>= 2; 
   }

   return root;
}

/* quick-and-dirty parameter selection */
static inline void tprpg_sel_ab(uint32_t k, uint32_t *a, uint32_t *b) {
  uint32_t x, y;
#ifdef TPRPG_SMALL_DOM_HACK
  if (k < 256) {
    x = 1;
    while (x * x < k)
      x <<= 1;
    *a = *b = x;
    /* fprintf(stderr, "a: %3u b: %3u\n", *a, *b); */
    return;
  }
#endif
  /* start with integer square root */
  x = y = isqrt32(k);
  /* We're done if k was a perfect square */
  if (x * y == k) { *a = x; *b = y; return; }
  /* here, x * y < k. increase y and possibly x so that x * y > k */
  if (x * ++y < k) x++;
  /* adjust x and y until x * y <= k... */
  while (x * y > k) { x--; y++; }
  /* then copy out the previous values to a and b so a * b >= k */
  *a = x + 1; *b = y - 1;
  /* fprintf(stderr, "a: %3u b: %3u\n", *a, *b); */
}

#define XTEA64_RND(V,I) ((((V << 4) ^ (V >> 5)) + V) ^ (sum + key[(I) & 3]))
#define XTEA32_RND(V,I) ((((V << 2) ^ (V >> 3)) + V) ^ (sum + key[(I) & 3]))
#define DELTA           0x9e3779b9
#define NUM_CYCLES      16
#define NUM_ROUNDS      (NUM_CYCLES*2)
#define KSA_ROUNDS      10
#define FEISTEL_RNDS    16

/* Modified RC4 KSA - second loop */
void tprpg_reseed(tprpg_ctx *ctx, const uint8_t *key, const size_t key_sz) {
  uint32_t *key_ptr = (uint32_t *)(ctx->keys);
  uint8_t *S = (uint8_t *)(ctx->keys);
  int i; uint8_t temp;

  /* Convert keys from native byte order to little endian */
  for (i=0; i < 64; i++)
    key_ptr[i] = htonl(key_ptr[i]);

  for (i = 0; i < 256; i++) {
    ctx->j = (ctx->j + S[i] + key[i % key_sz]) % 256;
    temp = S[i];
    S[i] = S[ctx->j];
    S[ctx->j] = temp;
  }

  /* Convert keys to native byte order for XTEA */
  for (i=0; i < 64; i++)
    key_ptr[i] = ntohl(key_ptr[i]);
}

/* Modified RC4 KSA */
void tprpg_setkey(tprpg_ctx *ctx, const uint8_t *key, const size_t key_sz) {
  uint32_t *key_ptr = (uint32_t *)(ctx->keys);
  uint8_t *S = (uint8_t *)(ctx->keys);
  int r, i; uint8_t temp;

  ctx->j = 0;
  for (i = 0; i < 256; i++)
    S[i] = i;
  /* Repeating the second loop of the RC4 KSA
   * a few times supposedly improves security */
  for (r = 0; r < KSA_ROUNDS; r++) {
    for (i = 0; i < 256; i++) {
      ctx->j = (ctx->j + S[i] + key[i % key_sz]) & 255;
      temp = S[i];
      S[i] = S[ctx->j];
      S[ctx->j] = temp;
    }
  }

  /* Convert keys to native byte order for XTEA */
  for (i=0; i < 64; i++)
    key_ptr[i] = ntohl(key_ptr[i]);

  ctx->last_k = 0;
  ctx->state  = TPRPG_KEYED;
}


/* XTEA modified for uint32 in/out */
static uint32_t _tprpg_xtea32(const uint32_t m, const uint32_t key[4]) {
  uint32_t i, sum=0;
  uint16_t l, r;

  l = m >> 16;    /* upper 16 bits */
  r = m & 0xffff; /* lower 16 bits */
  for (i=0; i < NUM_CYCLES; i++) {
    /* odd round */
    l += XTEA32_RND(r, sum);
    sum += DELTA;
    /* even round */
    r += XTEA32_RND(l, sum>>11);
  }
  return ((uint32_t)l << 16) + r;
}

#if 0
/* XTEA modified for uint64 in/out */
static uint64_t _tprpg_xtea64(const uint64_t m, const uint32_t key[4]) {
  uint32_t i, sum=0;
  uint32_t l, r;

  l = m >> 32;        /* upper 32 bits */
  r = m & 0xffffffff; /* lower 32 bits */
  for (i=0; i < NUM_CYCLES; i++) {
    /* odd round */
    l += XTEA64_RND(r, sum);
    sum += DELTA;
    /* even round */
    r += XTEA64_RND(l, sum>>11);
  }
  return ((uint64_t)l << 32) + r;
}
#endif

/* Generalized Feistel cipher using modified XTEA as round function */
uint32_t tprpg(tprpg_ctx *ctx, const uint32_t m, const uint32_t k) {
  if (ctx->state != TPRPG_KEYED)
    return UINT32_MAX; /* return blatently bogus results */

  uint32_t l, r, t;
  uint32_t i, c;

  if (ctx->last_k != k) { /* regenerate a and b if k has changed */
    tprpg_sel_ab(k, &(ctx->a), &(ctx->b));
    ctx->last_k = k;
  }

  l = m % ctx->a;
  r = m / ctx->a;
  for (i=0; i < FEISTEL_RNDS; i++) {
    t = (l + _tprpg_xtea32(r, (ctx->keys)[i&15])) % (i & 1 ? ctx->b : ctx->a);
    l = r; r = t;
  }
  c = ctx->a * r + l;
  if (c < k) {
    return c;
  } else {
    return tprpg(ctx, c, k);
  }
}


/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
