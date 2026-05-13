/* game.h — Asteroids SNK Edition: shared types and constants */
#ifndef GAME_H
#define GAME_H

#include "ngpc.h"
#include "library.h"

/* ---- Game states ---- */
#define STATE_TITLE  0
#define STATE_GAME   1
#define STATE_OVER   2
#define STATE_SCORES 3

/* ---- Entity types ---- */
#define MAX_ENTS   32
#define ENT_NONE    0
#define ENT_SHIP    1
#define ENT_ROCK_L  2
#define ENT_ROCK_M  3
#define ENT_ROCK_S  4
#define ENT_BULLET  5
#define ENT_UFO     6
#define ENT_USHOT   7

/* ---- Palettes ---- */
#define PAL_TEXT    0
#define PAL_SHIP    1
#define PAL_ROCK1   2
#define PAL_ROCK2   3
#define PAL_ROCK3   4
#define PAL_DIM     5
#define PAL_UFO     6
#define PAL_MARQUEE 7

/* ---- Sound IDs (1-based for PlaySound) ---- */
#define SND_FIRE     1
#define SND_THRUST   2
#define SND_WARP     3
#define SND_EXPLODE  4
#define SND_UFO_HUM  5
#define SND_UFO_FIRE 6

/* ---- Gameplay constants ---- */
#define UFO_FIRE_RATE 90
#define WARP_COOLDOWN 90

/* ---- Tile indices ---- */
#define T_SHIP     144
#define T_THRST    176
#define T_ALIEN    208
#define T_ROCK_L   216
#define T_ROCK_M   217
#define T_ROCK_S   218
#define T_BULLET   219
#define T_USHOT    220
#define T_STAR     221
#define T_MARQUEE  300
#define MARQUEE_W  20
#define MARQUEE_H  5

/* ---- Movement tables ---- */
extern const u8 dx_add[8];
extern const u8 dx_sub[8];
extern const u8 dy_add[8];
extern const u8 dy_sub[8];

/* ---- Entity pool (shared across modules) ---- */
extern u8 ent_type[MAX_ENTS];
extern u8 ent_px[MAX_ENTS];
extern u8 ent_py[MAX_ENTS];
extern u8 ent_dir[MAX_ENTS];
extern u8 ent_spd[MAX_ENTS];
extern u8 ent_tick[MAX_ENTS];
extern u8 ent_life[MAX_ENTS];
extern u8 ent_otx[MAX_ENTS];
extern u8 ent_oty[MAX_ENTS];
extern u8 ent_pal[MAX_ENTS];

/* ---- Game state (shared) ---- */
extern u8  state, skip, lives, wave, spawn_timer, game_over, alive;
extern u16 score;
extern u8  ship_dir, thrusting, warp_cooldown;
extern u8  ufo_active, ufo_timer, ufo_fire_tmr, ufo_idx;
extern u16 high_scores[5];
extern u8  pad_cur, pad_prev, pad_press;
extern u8  rand_seed;

/* ---- Shared utilities ---- */
u8 cheap_rand(u8 max);
u8 find_free(void);

#endif
