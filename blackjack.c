/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <termios.h>
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

#define BET_INCREMENT 10

static const char *card_suites[] = { "\xe2\x99\xa3", "\xe2\x99\xa6", "\xe2\x99\xa5", "\xe2\x99\xa0" };
static const char *card_values[] = {
  "A", "2", "3", "4", "5", "6", "7",
  "8", "9", "10", "J", "Q", "K" };
static const int card_num[] = {
  1011, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 10, 10, 10 };

typedef struct {
  unsigned char str[10];
  uint16_t n;
  uint8_t v;
  uint8_t s;
} card_t;

card_t deal_card(tprpg_ctx *ctx, uint32_t n) {
  int c;
  card_t card;

  c = tprpg(ctx, n, 52);
  card.v = (c>>2)%13;
  card.n = card_num[card.v];
  card.s = c&3;
  sprintf(card.str, "%2s%s", card_values[card.v], card_suites[card.s]);
  return card;
}

int main(int argc, char **argv) {
  unsigned int i;

  int cash = 500;
  int bet  = BET_INCREMENT;

  unsigned char bet_str[16];
  unsigned char cash_str[16];

  uint8_t key[256];

  tprpg_ctx ctx;

  struct termios term_orig, term_new;

  setvbuf(stdout, NULL, _IONBF, 0);
  tcgetattr(0, &term_orig);
  term_new = term_orig;
  term_new.c_lflag &= ~(ICANON|ECHO);
  term_new.c_cc[VMIN]  = 1;
  term_new.c_cc[VTIME] = 2;
  tcsetattr(0, TCSANOW, &term_new);

  if (argc > 1) {
    memset(key, 0, sizeof(key));
    strncpy(key, argv[1], sizeof(key)-1);
  } else {
    randombytes(key, sizeof(key));
  }

  tprpg_setkey(&ctx, key, 256);

  card_t dealer_cards[16];
  card_t player_cards[16];

  int deck_pos = 0;

  for (;;) {
    int c = 0;

    unsigned int dealer_p = 0;
    unsigned int player_p = 0;
    unsigned int dealer_t = 0;
    unsigned int player_t = 0;

    /* shuffle if the deck is getting low */
    if (deck_pos > 40) {
      printf("Shuffling...\n");
      tprpg_reseed(&ctx, "Deck shuffling string constant of doom!!!one", 42);
      deck_pos = 0;
    }

    /* Display bankroll, take bet */
    sprintf(cash_str, "$%d", cash);
    for (;;) {
      sprintf(bet_str, "$%d", bet);
      printf("\033[0G\033[2KCash: %7s Bet: %7s Choose (q)uit, (d)eal", cash_str, bet_str);
      if (cash - BET_INCREMENT >= bet) printf(", (i)ncrease bet");
      if (bet > BET_INCREMENT) printf(", (r)educe bet");
      printf(" ");
      c = getchar();
      if (c == 'd') break;
      switch (c) {
        case 'q':
          printf("\n");
          tcsetattr(0, TCSANOW, &term_orig);
          return 0;
        case 'i':
          if (cash - BET_INCREMENT >= bet)
            bet += BET_INCREMENT;
          break;
        case 'r':
          if (bet > BET_INCREMENT)
            bet -= BET_INCREMENT;
          break;
      }
    }
    printf("\n");
    cash -= bet;

    dealer_cards[dealer_p++] = deal_card(&ctx, deck_pos++);
    dealer_cards[dealer_p++] = deal_card(&ctx, deck_pos++);

    printf("Dealer: ??? %s\n", dealer_cards[1].str);
    
    player_cards[player_p++] = deal_card(&ctx, deck_pos++);
    player_cards[player_p++] = deal_card(&ctx, deck_pos++);
   
    for (;;) {
      printf("\033[0G\033[2KPlayer:");
      player_t = 0;
      for (i = 0; i < player_p; i++) {
        printf(" %s", player_cards[i].str);
        player_t += player_cards[i].n;
      }
      /* If over 21, switch aces from 11 to 1 */
      while (player_t > 1000 && ((player_t % 1000) > 21)) player_t -= 1010;
      player_t %= 1000;
      printf(" (%u)", player_t);
      if (player_t > 21) {
        printf(" BUST!\033[0G\033[1A\033[2KDealer: %s %s\033[1B\n", dealer_cards[0].str, dealer_cards[1].str);
        break;
      } else if (player_t == 21 && player_p == 2) {
        printf(" BLACKJACK!\033[0G\033[1A\033[2KDealer: %s %s\033[1B\n", dealer_cards[0].str, dealer_cards[1].str);
        cash += bet * 3;
        break;
      } else if (dealer_t && dealer_t < player_t) {
        printf("\033[1B\n");
        cash += bet * 2;
        break;
      } else if (dealer_t && dealer_t == player_t) {
        printf("\033[1B\n");
        cash += bet;
        break;
      } else if (dealer_t > 21) {
        printf("\033[1B\n");
        cash += bet * 2;
        break;
      } else if (dealer_t && dealer_t > player_t) {
        printf("\033[1B\n");
        break;
      } else {
        printf(" Choose (h)it or (s)tand ");
      }
      for (;;) {
        c = getchar();
        if (c == 'h') {
          player_cards[player_p++] = deal_card(&ctx, deck_pos++);
          break;
        } else if (c == 's') {
          for (;;) {
            dealer_t = 0;
            printf("\033[0G\033[1A\033[2KDealer:");
            for (i = 0; i < dealer_p; i++) {
              printf(" %s", dealer_cards[i].str);
              dealer_t += dealer_cards[i].n;
            }
            while (dealer_t > 1000 && ((dealer_t % 1000) > 21)) dealer_t -= 1010;
            dealer_t %= 1000;
            if (dealer_t < 17) {
              dealer_cards[dealer_p++] = deal_card(&ctx, deck_pos++);
            } else {
              break;
            }
          }
          printf(" (%u)\n", dealer_t);
          break;
        }
      }
    }
  }
  tcsetattr(0, TCSANOW, &term_orig);
  return 0;
}

/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
