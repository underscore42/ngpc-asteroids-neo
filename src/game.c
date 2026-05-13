/* game.c — Shared state and utilities */
#include "game.h"

/* Movement tables */
const u8 dx_add[8] = {0,1,1,1,0,0,0,0};
const u8 dx_sub[8] = {0,0,0,0,0,1,1,1};
const u8 dy_add[8] = {0,0,0,1,1,1,0,0};
const u8 dy_sub[8] = {1,1,0,0,0,0,0,1};

/* Entity pool */
u8 ent_type[MAX_ENTS];
u8 ent_px[MAX_ENTS];
u8 ent_py[MAX_ENTS];
u8 ent_dir[MAX_ENTS];
u8 ent_spd[MAX_ENTS];
u8 ent_tick[MAX_ENTS];
u8 ent_life[MAX_ENTS];
u8 ent_otx[MAX_ENTS];
u8 ent_oty[MAX_ENTS];
u8 ent_pal[MAX_ENTS];

/* Game state */
u8  state, skip, lives, wave, spawn_timer, game_over, alive;
u16 score;
u8  ship_dir, thrusting, warp_cooldown;
u8  ufo_active, ufo_timer, ufo_fire_tmr, ufo_idx;
u16 high_scores[5];
u8  pad_cur, pad_prev, pad_press;
u8  rand_seed;

/* RNG */
u8 cheap_rand(u8 max) {
    rand_seed = rand_seed + 7;
    rand_seed = rand_seed ^ (rand_seed << 3);
    rand_seed = rand_seed ^ (rand_seed >> 1);
    if (max == 0) return 0;
    return rand_seed % max;
}

/* Find free entity slot */
u8 find_free(void) {
    u8 i;
    for (i = 1; i < MAX_ENTS; i++) if (ent_type[i] == ENT_NONE) return i;
    return 255;
}
