/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "tprpg.h"
#include "print_hex.h"
#include "randombytes.h"

/* static const char *card_suites[] = { "C", "D", "H", "S" }; */
static const char *card_suites[] = { "\xe2\x99\xa3", "\xe2\x99\xa6", "\xe2\x99\xa5", "\xe2\x99\xa0" };
static const char *card_values[] = {
  "A", "2", "3", "4", "5", "6", "7",
  "8", "9", "10", "J", "Q", "K" };
static const int card_num[] = {
  1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 10, 10, 10 };

int main(int argc, char **argv) {
  uint32_t c;
  int i;

  uint8_t key[256];

  tprpg_ctx ctx;

  if (argc > 1) {
    memset(key, 0, sizeof(key));
    strncpy(key, argv[1], sizeof(key)-1);
  } else {
    randombytes(key, sizeof(key));
  }

  tprpg_setkey(&ctx, key, 256);
  for (i = 0; i < 52; i++) {
    c = tprpg(&ctx, i, 52);
    printf("%2s%s", card_values[(c>>2)%13], card_suites[c&3]);
    if ((i & 3) == 3) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
  
  return 0;
}

/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
