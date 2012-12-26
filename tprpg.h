#ifndef __TPRPG_H_
#define __TPRPG_H_

/* arbitrary magic number */
#define TPRPG_KEYED 2827962856

typedef struct tprpg_ctx_s {
  uint32_t keys[16][4];
  uint32_t a;
  uint32_t b;
  uint32_t last_k;
  uint32_t state;
  uint8_t  j;
} tprpg_ctx;

void tprpg_setkey(tprpg_ctx *ctx, const uint8_t *key, size_t key_sz);
void tprpg_reseed(tprpg_ctx *ctx, const uint8_t *key, size_t key_sz);

uint32_t tprpg(tprpg_ctx *ctx, const uint32_t m, const uint32_t k);

#endif
/* 
 * vim: ts=2 sw=2 et ai si bg=dark
 */
