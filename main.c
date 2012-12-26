/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "tprpg.h"
#include "print_hex.h"
#include "randombytes.h"

int main(int argc, char **argv) {
  uint32_t i, n, r, c, max;

  uint8_t key[256];

  tprpg_ctx ctx;

  if (argc == 2) {
    max = atoi(argv[1]);
    n   = max;
    c   = 1;
  } else if (argc == 3) {
    max = atoi(argv[1]);
    n   = atoi(argv[2]);
    c   = 1;
  } else if (argc == 4) {
    max = atoi(argv[1]);
    n   = atoi(argv[2]);
    c   = atoi(argv[3]);
  } else {
    fprintf(stderr, "wrong number of arguments\n");
    return 1;
  }

  randombytes(key, sizeof(key));
  tprpg_setkey(&ctx, key, 256);
  while (c--) {
    randombytes(key, 16);
    tprpg_reseed(&ctx, key, 16);
    for (i = 0;;) {
      r = tprpg(&ctx, i, max);
      printf("%2u", r);
      i++;
      if (i >= n) break;
      printf(" ");
    }
    printf("\n");
  }

  return 0;
}

/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
