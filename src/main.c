/* main.c — ASTEROIDS standalone for Neo Geo Pocket Color
 *
 * Proven Space Collection sub-game pattern:
 *   1 tile per entity, u16[8] with InstallTileSetAt()
 *   Pixel positions, tick-based movement, struct-of-arrays
 *
 * V2 features: 8-dir ship, UFO, warp, title screen, greyscale
 * Controls: L/R=rotate, Up=thrust, A=fire, B=warp
 */

#define CARTHDR_IMPL
#include "carthdr.h"
#include "ngpc.h"
#include "library.h"

/* ---- Tile data: u16[8] per tile, 1bpp ---- */

static const u16 ship_n[8]  = { 0x0100,0x0380,0x0380,0x06C0,0x06C0,0x0C60,0x0C60,0x0000 };
static const u16 ship_ne[8] = { 0x003E,0x001C,0x0034,0x0064,0x00C4,0x0180,0x0000,0x0000 };
static const u16 ship_e[8]  = { 0x0000,0x0100,0x0180,0x0FE0,0x0FE0,0x0180,0x0100,0x0000 };
static const u16 ship_se[8] = { 0x0000,0x0000,0x0180,0x00C4,0x0064,0x0034,0x001C,0x003E };
static const u16 ship_s[8]  = { 0x0000,0x0C60,0x0C60,0x06C0,0x06C0,0x0380,0x0380,0x0100 };
static const u16 ship_sw[8] = { 0x0000,0x0000,0x0180,0x2300,0x2600,0x2C00,0x3800,0x7C00 };
static const u16 ship_w[8]  = { 0x0000,0x0080,0x0180,0x07F0,0x07F0,0x0180,0x0080,0x0000 };
static const u16 ship_nw[8] = { 0x7C00,0x3800,0x2C00,0x2600,0x2300,0x0180,0x0000,0x0000 };

static const u16 thrst_n[8]  = { 0x0100,0x0380,0x0380,0x06C0,0x06C0,0x0C60,0x0C60,0x0100 };
static const u16 thrst_ne[8] = { 0x003E,0x001C,0x0034,0x0064,0x00C4,0x0180,0x0200,0x0000 };
static const u16 thrst_e[8]  = { 0x0000,0x0100,0x0180,0x0FE0,0x0FE0,0x0180,0x0100,0x0000 };
static const u16 thrst_se[8] = { 0x0000,0x0200,0x0180,0x00C4,0x0064,0x0034,0x001C,0x003E };
static const u16 thrst_s[8]  = { 0x0100,0x0C60,0x0C60,0x06C0,0x06C0,0x0380,0x0380,0x0100 };
static const u16 thrst_sw[8] = { 0x0000,0x0040,0x0180,0x2300,0x2600,0x2C00,0x3800,0x7C00 };
static const u16 thrst_w[8]  = { 0x0000,0x0080,0x0180,0x07F0,0x07F0,0x0180,0x0080,0x0000 };
static const u16 thrst_nw[8] = { 0x7C00,0x3800,0x2C00,0x2600,0x2300,0x0180,0x0040,0x0000 };

static const u16 rock_large[8] = { 0x07E0,0x1FF8,0x3FFC,0x7FFE,0x7FFE,0x3FFC,0x1FF8,0x07E0 };
static const u16 rock_med[8]   = { 0x0000,0x03C0,0x07E0,0x0FF0,0x0FF0,0x07E0,0x03C0,0x0000 };
static const u16 rock_small[8] = { 0x0000,0x0000,0x0180,0x03C0,0x03C0,0x0180,0x0000,0x0000 };
static const u16 bullet_t[8]   = { 0x0000,0x0000,0x0000,0x0180,0x0180,0x0000,0x0000,0x0000 };
static const u16 ufo_t[8]      = { 0x0000,0x03C0,0x07E0,0x1FF8,0x3FFC,0x0FF0,0x07E0,0x0000 };
static const u16 ufo_shot_t[8] = { 0x0000,0x0000,0x0000,0x0100,0x0100,0x0000,0x0000,0x0000 };
static const u16 star_t[8]     = { 0x0000,0x0000,0x0000,0x0100,0x0000,0x0000,0x0000,0x0000 };

/* ---- Tile indices (after system font at 0..143) ---- */
#define T_SHIP     144
#define T_THRST    152
#define T_ROCK_L   160
#define T_ROCK_M   161
#define T_ROCK_S   162
#define T_BULLET   163
#define T_UFO      164
#define T_USHOT    165
#define T_STAR     166

static void install_tiles(void) {
    InstallTileSetAt((const unsigned short (*)[8])ship_n,  8, T_SHIP);
    InstallTileSetAt((const unsigned short (*)[8])ship_ne, 8, T_SHIP+1);
    InstallTileSetAt((const unsigned short (*)[8])ship_e,  8, T_SHIP+2);
    InstallTileSetAt((const unsigned short (*)[8])ship_se, 8, T_SHIP+3);
    InstallTileSetAt((const unsigned short (*)[8])ship_s,  8, T_SHIP+4);
    InstallTileSetAt((const unsigned short (*)[8])ship_sw, 8, T_SHIP+5);
    InstallTileSetAt((const unsigned short (*)[8])ship_w,  8, T_SHIP+6);
    InstallTileSetAt((const unsigned short (*)[8])ship_nw, 8, T_SHIP+7);
    InstallTileSetAt((const unsigned short (*)[8])thrst_n,  8, T_THRST);
    InstallTileSetAt((const unsigned short (*)[8])thrst_ne, 8, T_THRST+1);
    InstallTileSetAt((const unsigned short (*)[8])thrst_e,  8, T_THRST+2);
    InstallTileSetAt((const unsigned short (*)[8])thrst_se, 8, T_THRST+3);
    InstallTileSetAt((const unsigned short (*)[8])thrst_s,  8, T_THRST+4);
    InstallTileSetAt((const unsigned short (*)[8])thrst_sw, 8, T_THRST+5);
    InstallTileSetAt((const unsigned short (*)[8])thrst_w,  8, T_THRST+6);
    InstallTileSetAt((const unsigned short (*)[8])thrst_nw, 8, T_THRST+7);
    InstallTileSetAt((const unsigned short (*)[8])rock_large, 8, T_ROCK_L);
    InstallTileSetAt((const unsigned short (*)[8])rock_med,   8, T_ROCK_M);
    InstallTileSetAt((const unsigned short (*)[8])rock_small, 8, T_ROCK_S);
    InstallTileSetAt((const unsigned short (*)[8])bullet_t,   8, T_BULLET);
    InstallTileSetAt((const unsigned short (*)[8])ufo_t,      8, T_UFO);
    InstallTileSetAt((const unsigned short (*)[8])ufo_shot_t, 8, T_USHOT);
    InstallTileSetAt((const unsigned short (*)[8])star_t,     8, T_STAR);
}

/* ---- Constants ---- */
#define MAX_ENTS   32
#define ENT_NONE    0
#define ENT_SHIP    1
#define ENT_ROCK_L  2
#define ENT_ROCK_M  3
#define ENT_ROCK_S  4
#define ENT_BULLET  5
#define ENT_UFO     6
#define ENT_USHOT   7

#define STATE_TITLE  0
#define STATE_GAME   1
#define STATE_OVER   2
#define STATE_SCORES 3

#define PAL_TEXT  0
#define PAL_SHIP  1
#define PAL_ROCK1 2
#define PAL_ROCK2 3
#define PAL_ROCK3 4
#define PAL_DIM   5
#define PAL_UFO   6

#define UFO_FIRE_RATE 90
#define WARP_COOLDOWN 90

static const u8 dx_add[8] = {0,1,1,1,0,0,0,0};
static const u8 dx_sub[8] = {0,0,0,0,0,1,1,1};
static const u8 dy_add[8] = {0,0,0,1,1,1,0,0};
static const u8 dy_sub[8] = {1,1,0,0,0,0,0,1};

/* ---- Entity pool ---- */
static u8 ent_type[MAX_ENTS];
static u8 ent_px[MAX_ENTS];
static u8 ent_py[MAX_ENTS];
static u8 ent_dir[MAX_ENTS];
static u8 ent_spd[MAX_ENTS];
static u8 ent_tick[MAX_ENTS];
static u8 ent_life[MAX_ENTS];
static u8 ent_otx[MAX_ENTS];
static u8 ent_oty[MAX_ENTS];
static u8 ent_pal[MAX_ENTS];

/* ---- State ---- */
static u8  ship_dir, thrusting, rot_tick, fire_tick;
static u8  ship_vx_add, ship_vx_sub, ship_vy_add, ship_vy_sub;
static u8  drift_timer, alive;
static u8  state, skip, lives, wave, spawn_timer, game_over;
static u16 score;
static u8  warp_cooldown;
static u8  ufo_active, ufo_timer, ufo_fire_tmr, ufo_idx;
static u8  star_x[16], star_y[16];
static u16 high_scores[5];
static u8  rand_seed;
static u8  pad_cur, pad_prev, pad_press;

/* ---- RNG ---- */
static u8 cheap_rand(u8 max) {
    rand_seed = rand_seed + 7;
    rand_seed = rand_seed ^ (rand_seed << 3);
    rand_seed = rand_seed ^ (rand_seed >> 1);
    if (max == 0) return 0;
    return rand_seed % max;
}

/* ---- Entity helpers ---- */
static u8 find_free(void) {
    u8 i;
    for (i = 1; i < MAX_ENTS; i++) {
        if (ent_type[i] == ENT_NONE) return i;
    }
    return 255;
}

static void erase_ent(u8 i) {
    if (ent_otx[i] < 20 && ent_oty[i] < 19)
        PutTile(SCR_1_PLANE, 0, ent_otx[i], ent_oty[i], ' ');
}

static u16 ent_tile(u8 i) {
    u8 t;
    t = ent_type[i];
    if (t == ENT_SHIP) return thrusting ? T_THRST + ent_dir[i] : T_SHIP + ent_dir[i];
    if (t == ENT_ROCK_L) return T_ROCK_L;
    if (t == ENT_ROCK_M) return T_ROCK_M;
    if (t == ENT_ROCK_S) return T_ROCK_S;
    if (t == ENT_BULLET) return T_BULLET;
    if (t == ENT_UFO) return T_UFO;
    if (t == ENT_USHOT) return T_USHOT;
    return ' ';
}

static void draw_ent(u8 i) {
    u8 tx, ty;
    tx = ent_px[i] >> 3;
    ty = ent_py[i] >> 3;
    if (tx >= 20) tx = 19;
    if (ty >= 19) ty = 18;
    PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, ent_tile(i));
    ent_otx[i] = tx;
    ent_oty[i] = ty;
}

static void move_ent(u8 i) {
    u8 d;
    ent_tick[i] = ent_tick[i] + 1;
    if (ent_tick[i] < ent_spd[i]) return;
    ent_tick[i] = 0;
    d = ent_dir[i];
    if (dx_add[d]) { ent_px[i] = ent_px[i] + 1; if (ent_px[i] >= 160) ent_px[i] = 0; }
    if (dx_sub[d]) { if (ent_px[i] == 0) ent_px[i] = 159; else ent_px[i] = ent_px[i] - 1; }
    if (dy_add[d]) { ent_py[i] = ent_py[i] + 1; if (ent_py[i] >= 152) ent_py[i] = 8; }
    if (dy_sub[d]) { if (ent_py[i] <= 8) ent_py[i] = 151; else ent_py[i] = ent_py[i] - 1; }
}

/* ---- Rocks ---- */
static void spawn_rock(u8 type, u8 px, u8 py) {
    u8 idx;
    idx = find_free();
    if (idx == 255) return;
    ent_type[idx] = type;
    ent_px[idx] = px;
    ent_py[idx] = py;
    ent_dir[idx] = cheap_rand(8);
    ent_tick[idx] = 0;
    ent_otx[idx] = 255;
    ent_oty[idx] = 255;
    ent_life[idx] = 0;
    if (type == ENT_ROCK_L) { ent_spd[idx] = 6; ent_pal[idx] = PAL_ROCK1; }
    else if (type == ENT_ROCK_M) { ent_spd[idx] = 4; ent_pal[idx] = PAL_ROCK2; }
    else { ent_spd[idx] = 3; ent_pal[idx] = PAL_ROCK3; }
}

static void spawn_wave(void) {
    u8 count, i;
    count = 3 + wave;
    if (count > 8) count = 8;
    for (i = 0; i < count; i++) {
        if (cheap_rand(2) == 0)
            spawn_rock(ENT_ROCK_L, cheap_rand(160), cheap_rand(2) == 0 ? 12 : 140);
        else
            spawn_rock(ENT_ROCK_L, cheap_rand(2) == 0 ? 4 : 152, 12 + cheap_rand(128));
    }
}

static void destroy_rock(u8 idx) {
    u8 t, px, py;
    t = ent_type[idx]; px = ent_px[idx]; py = ent_py[idx];
    if (t == ENT_ROCK_L) score = score + 20;
    else if (t == ENT_ROCK_M) score = score + 50;
    else score = score + 100;
    erase_ent(idx);
    ent_type[idx] = ENT_NONE;
    if (t == ENT_ROCK_L) { spawn_rock(ENT_ROCK_M, px+4, py); spawn_rock(ENT_ROCK_M, px, py+4); }
    else if (t == ENT_ROCK_M) { spawn_rock(ENT_ROCK_S, px+2, py); spawn_rock(ENT_ROCK_S, px, py+2); }
}

static u8 count_rocks(void) {
    u8 i, c;
    c = 0;
    for (i = 0; i < MAX_ENTS; i++)
        if (ent_type[i] >= ENT_ROCK_L && ent_type[i] <= ENT_ROCK_S) c++;
    return c;
}

/* ---- Bullets ---- */
static void fire_bullet(void) {
    u8 idx;
    idx = find_free();
    if (idx == 255) return;
    ent_type[idx] = ENT_BULLET;
    ent_px[idx] = ent_px[0]; ent_py[idx] = ent_py[0];
    ent_dir[idx] = ship_dir;
    ent_spd[idx] = 1; ent_tick[idx] = 0;
    ent_life[idx] = 40; ent_pal[idx] = PAL_SHIP;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
}

/* ---- UFO ---- */
static void spawn_ufo(void) {
    u8 idx, side;
    if (ufo_active) return;
    idx = find_free();
    if (idx == 255) return;
    ufo_active = 1; ufo_idx = idx;
    ufo_fire_tmr = UFO_FIRE_RATE;
    ent_type[idx] = ENT_UFO; ent_pal[idx] = PAL_UFO;
    ent_tick[idx] = 0; ent_otx[idx] = 255; ent_oty[idx] = 255;
    side = cheap_rand(2);
    if (side) { ent_px[idx] = 0; ent_dir[idx] = 2; }
    else { ent_px[idx] = 155; ent_dir[idx] = 6; }
    ent_py[idx] = 30 + cheap_rand(90);
    ent_spd[idx] = 3;
}

static void ufo_fire(void) {
    u8 idx, aim_dir;
    if (!ufo_active || !alive) return;
    idx = find_free();
    if (idx == 255) return;
    ent_type[idx] = ENT_USHOT;
    ent_px[idx] = ent_px[ufo_idx]; ent_py[idx] = ent_py[ufo_idx];
    ent_spd[idx] = 2; ent_tick[idx] = 0;
    ent_life[idx] = 60; ent_pal[idx] = PAL_UFO;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
    if (ent_px[0] > ent_px[ufo_idx] + 16) {
        if (ent_py[0] + 16 < ent_py[ufo_idx]) aim_dir = 1;
        else if (ent_py[0] > ent_py[ufo_idx] + 16) aim_dir = 3;
        else aim_dir = 2;
    } else if (ent_px[0] + 16 < ent_px[ufo_idx]) {
        if (ent_py[0] + 16 < ent_py[ufo_idx]) aim_dir = 7;
        else if (ent_py[0] > ent_py[ufo_idx] + 16) aim_dir = 5;
        else aim_dir = 6;
    } else {
        if (ent_py[0] + 16 < ent_py[ufo_idx]) aim_dir = 0;
        else aim_dir = 4;
    }
    if (cheap_rand(3) == 0) aim_dir = (aim_dir + 1) & 7;
    else if (cheap_rand(3) == 0) aim_dir = (aim_dir + 7) & 7;
    ent_dir[idx] = aim_dir;
}

static void update_ufo(void) {
    if (!ufo_active) {
        if (wave >= 3) {
            if (ufo_timer > 0) ufo_timer--;
            else { spawn_ufo(); ufo_timer = 150 + cheap_rand(128); }
        }
        return;
    }
    if ((ent_dir[ufo_idx] == 2 && ent_px[ufo_idx] > 158) ||
        (ent_dir[ufo_idx] == 6 && ent_px[ufo_idx] < 2)) {
        erase_ent(ufo_idx); ent_type[ufo_idx] = ENT_NONE;
        ufo_active = 0; ufo_timer = 100 + cheap_rand(128); return;
    }
    if (ufo_fire_tmr > 0) ufo_fire_tmr--;
    else {
        ufo_fire();
        ufo_fire_tmr = UFO_FIRE_RATE - (wave * 5);
        if (ufo_fire_tmr < 30) ufo_fire_tmr = 30;
    }
}

/* ---- Forward decl ---- */
static void insert_high_score(u16 s);
static void draw_title(void);

/* ---- Game over ---- */
static void do_game_over(void) {
    game_over = 1; state = STATE_OVER;
    insert_high_score(score);
    PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
    PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
    skip = 30;
}

/* ---- Warp ---- */
static void warp_ship(void) {
    if (warp_cooldown > 0) return;
    erase_ent(0);
    ent_px[0] = 16 + cheap_rand(128);
    ent_py[0] = 16 + cheap_rand(112);
    ship_vx_add = 0; ship_vx_sub = 0;
    ship_vy_add = 0; ship_vy_sub = 0;
    drift_timer = 0;
    ent_otx[0] = 255; ent_oty[0] = 255;
    warp_cooldown = WARP_COOLDOWN;
    if (cheap_rand(10) == 0) {
        ent_type[0] = ENT_NONE; alive = 0;
        if (lives > 0) lives = lives - 1;
        spawn_timer = 0;
        if (lives == 0) do_game_over();
    }
}

/* ---- HUD ---- */
static void put_score_at(u8 x, u8 y, u16 val) {
    u16 v; v = val;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+4, y, '0' + v % 10); v = v / 10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+3, y, '0' + v % 10); v = v / 10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+2, y, '0' + v % 10); v = v / 10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+1, y, '0' + v % 10); v = v / 10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x,   y, '0' + v % 10);
}

static void draw_hud(void) {
    put_score_at(0, 0, score);
    PrintString(SCR_1_PLANE, PAL_DIM, 7, 0, "LIVES:");
    PutTile(SCR_1_PLANE, PAL_TEXT, 13, 0, '0' + lives);
    PutTile(SCR_1_PLANE, PAL_DIM, 15, 0, 'W');
    PutTile(SCR_1_PLANE, PAL_TEXT, 16, 0, '0' + wave / 10);
    PutTile(SCR_1_PLANE, PAL_TEXT, 17, 0, '0' + wave % 10);
    if (warp_cooldown > 0) PutTile(SCR_1_PLANE, PAL_ROCK3, 19, 0, 'B');
    else PutTile(SCR_1_PLANE, PAL_SHIP, 19, 0, 'B');
}

/* ---- Stars ---- */
static void init_stars(void) {
    u8 i;
    for (i = 0; i < 16; i++) { star_x[i] = cheap_rand(20); star_y[i] = 1 + cheap_rand(17); }
}

static void draw_stars(void) {
    u8 i;
    for (i = 0; i < 16; i++) PutTile(SCR_1_PLANE, PAL_DIM, star_x[i], star_y[i], T_STAR);
}

/* ---- Palette setup ---- */
static void setup_palettes(void) {
    SetBackgroundColour(RGB(0, 0, 0));
    SetPalette(SCR_1_PLANE, PAL_TEXT, 0, RGB(15,15,15), RGB(15,15,15), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_SHIP, 0, RGB(0,15,0), RGB(0,10,0), RGB(0,15,0));
    SetPalette(SCR_1_PLANE, PAL_ROCK1, 0, RGB(13,13,13), RGB(11,11,11), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_ROCK2, 0, RGB(9,9,9), RGB(7,7,7), RGB(11,11,11));
    SetPalette(SCR_1_PLANE, PAL_ROCK3, 0, RGB(6,6,6), RGB(4,4,4), RGB(8,8,8));
    SetPalette(SCR_1_PLANE, PAL_DIM, 0, RGB(4,4,4), RGB(3,3,3), RGB(5,5,5));
    SetPalette(SCR_1_PLANE, PAL_UFO, 0, RGB(10,12,10), RGB(8,10,8), RGB(12,14,12));
}

/* ---- Title ---- */
static void draw_title(void) {
    ClearScreen(SCR_1_PLANE);
    setup_palettes();
    
    SysSetSystemFont();
    install_tiles();
    PrintString(SCR_1_PLANE, PAL_DIM,  0, 0, "--------------------");
    PrintString(SCR_1_PLANE, PAL_SHIP, 1, 2, "A S T E R O I D S");
    PrintString(SCR_1_PLANE, PAL_DIM,  2, 3, "================");
    PutTile(SCR_1_PLANE, PAL_SHIP, 10, 5, T_SHIP);
    PutTile(SCR_1_PLANE, PAL_SHIP, 10, 4, T_BULLET);
    PutTile(SCR_1_PLANE, PAL_ROCK1, 3, 6, T_ROCK_L);
    PutTile(SCR_1_PLANE, PAL_ROCK2, 16, 7, T_ROCK_M);
    PutTile(SCR_1_PLANE, PAL_ROCK3, 7, 8, T_ROCK_S);
    PutTile(SCR_1_PLANE, PAL_ROCK1, 14, 5, T_ROCK_L);
    PutTile(SCR_1_PLANE, PAL_UFO, 5, 10, T_UFO);
    PrintString(SCR_1_PLANE, PAL_ROCK2, 3, 12, "NEO GEO POCKET");
    PrintString(SCR_1_PLANE, PAL_SHIP, 3, 15, "PRESS  A  START");
    PrintString(SCR_1_PLANE, PAL_DIM,  1, 16, "LR:ROT U:THR B:WARP");
    PrintString(SCR_1_PLANE, PAL_DIM,  0, 17, "--------------------");
    PrintString(SCR_1_PLANE, PAL_DIM,  5, 18, "HI:");
    put_score_at(8, 18, high_scores[0]);
}

/* ---- High scores ---- */
static void insert_high_score(u16 s) {
    u8 i, j;
    for (i = 0; i < 5; i++) {
        if (s > high_scores[i]) {
            j = 4;
            while (j > i) { high_scores[j] = high_scores[j-1]; j--; }
            high_scores[i] = s; return;
        }
    }
}

static void draw_scores(void) {
    u8 i;
    ClearScreen(SCR_1_PLANE);
    setup_palettes();
    SysSetSystemFont();
    PrintString(SCR_1_PLANE, PAL_SHIP, 3, 1, "HIGH  SCORES");
    PrintString(SCR_1_PLANE, PAL_DIM,  2, 2, "================");
    for (i = 0; i < 5; i++) {
        PutTile(SCR_1_PLANE, PAL_TEXT, 5, 4+i*2, '1'+i);
        PutTile(SCR_1_PLANE, PAL_TEXT, 6, 4+i*2, '.');
        put_score_at(8, 4+i*2, high_scores[i]);
    }
    PrintString(SCR_1_PLANE, PAL_ROCK2, 2, 16, "A:PLAY  OPT:TITLE");
}

/* ---- Game init ---- */
static void game_start(void) {
    u8 i;
    ClearScreen(SCR_1_PLANE);
    setup_palettes();
    SysSetSystemFont();
    install_tiles();
    for (i = 0; i < MAX_ENTS; i++) { ent_type[i] = ENT_NONE; ent_otx[i] = 255; ent_oty[i] = 255; ent_pal[i] = 0; }
    ent_type[0] = ENT_SHIP; ent_px[0] = 80; ent_py[0] = 76;
    ent_dir[0] = 0; ent_spd[0] = 3; ent_tick[0] = 0;
    ent_pal[0] = PAL_SHIP; ent_otx[0] = 255; ent_oty[0] = 255;
    ship_dir = 0; ship_vx_add = 0; ship_vx_sub = 0;
    ship_vy_add = 0; ship_vy_sub = 0; drift_timer = 0;
    thrusting = 0; rot_tick = 0; fire_tick = 0;
    score = 0; lives = 3; wave = 1; spawn_timer = 0;
    game_over = 0; alive = 1; warp_cooldown = 0;
    ufo_active = 0; ufo_timer = 200; ufo_fire_tmr = UFO_FIRE_RATE;
    init_stars(); draw_stars(); spawn_wave(); draw_hud();
}

/* ---- Game update ---- */
static void game_update(void) {
    u8 i, j, bcount;
    if (game_over) return;
    if (warp_cooldown > 0) warp_cooldown--;

    if (pad_press & J_OPTION) {
        state = STATE_TITLE; skip = 10; draw_title(); return;
    }

    update_ufo();

    if (!alive) {
        spawn_timer++;
        if (spawn_timer > 60 && lives > 0) {
            spawn_timer = 0;
            ent_type[0] = ENT_SHIP; ent_px[0] = 80; ent_py[0] = 76;
            ent_dir[0] = 0; ent_spd[0] = 3; ent_tick[0] = 0;
            ent_pal[0] = PAL_SHIP; ent_otx[0] = 255; ent_oty[0] = 255;
            ship_dir = 0; ship_vx_add = 0; ship_vx_sub = 0;
            ship_vy_add = 0; ship_vy_sub = 0; drift_timer = 0;
            warp_cooldown = 0; alive = 1;
        }
    }

    if (alive && ent_type[0] == ENT_SHIP) {
        rot_tick++;
        if (rot_tick >= 8) {
            rot_tick = 0;
            if (pad_cur & J_LEFT) { erase_ent(0); if (ship_dir == 0) ship_dir = 7; else ship_dir = ship_dir - 1; ent_dir[0] = ship_dir; }
            if (pad_cur & J_RIGHT) { erase_ent(0); ship_dir = ship_dir + 1; if (ship_dir > 7) ship_dir = 0; ent_dir[0] = ship_dir; }
        }
        thrusting = 0;
        if (pad_cur & J_UP) {
            thrusting = 1;
            ship_vx_add = dx_add[ship_dir]; ship_vx_sub = dx_sub[ship_dir];
            ship_vy_add = dy_add[ship_dir]; ship_vy_sub = dy_sub[ship_dir];
        }
        drift_timer++;
        if (drift_timer >= 3) {
            drift_timer = 0; erase_ent(0);
            if (ship_vx_add) { ent_px[0] = ent_px[0]+1; if (ent_px[0] >= 160) ent_px[0] = 0; }
            if (ship_vx_sub) { if (ent_px[0] == 0) ent_px[0] = 159; else ent_px[0] = ent_px[0]-1; }
            if (ship_vy_add) { ent_py[0] = ent_py[0]+1; if (ent_py[0] >= 152) ent_py[0] = 8; }
            if (ship_vy_sub) { if (ent_py[0] <= 8) ent_py[0] = 151; else ent_py[0] = ent_py[0]-1; }
        }
        fire_tick++;
        if ((pad_press & J_A) && fire_tick >= 6) {
            bcount = 0;
            for (i = 0; i < MAX_ENTS; i++) if (ent_type[i] == ENT_BULLET) bcount++;
            if (bcount < 4) { fire_bullet(); fire_tick = 0; }
        }
        if ((pad_press & J_B) && alive) warp_ship();
    }

    for (i = 1; i < MAX_ENTS; i++) {
        if (ent_type[i] == ENT_NONE) continue;
        erase_ent(i); move_ent(i);
        if (ent_type[i] == ENT_BULLET || ent_type[i] == ENT_USHOT) {
            if (ent_life[i] > 0) ent_life[i] = ent_life[i] - 1;
            if (ent_life[i] == 0) { erase_ent(i); ent_type[i] = ENT_NONE; continue; }
        }
    }

    /* Collision: bullets vs rocks */
    for (i = 0; i < MAX_ENTS; i++) {
        if (ent_type[i] != ENT_BULLET) continue;
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j] < ENT_ROCK_L || ent_type[j] > ENT_ROCK_S) continue;
            if ((ent_px[i]>>3)==(ent_px[j]>>3) && (ent_py[i]>>3)==(ent_py[j]>>3)) {
                erase_ent(i); ent_type[i] = ENT_NONE; destroy_rock(j); break;
            }
        }
    }

    /* Collision: bullets vs UFO */
    if (ufo_active) {
        for (i = 0; i < MAX_ENTS; i++) {
            if (ent_type[i] != ENT_BULLET) continue;
            if ((ent_px[i]>>3)==(ent_px[ufo_idx]>>3) && (ent_py[i]>>3)==(ent_py[ufo_idx]>>3)) {
                erase_ent(i); ent_type[i] = ENT_NONE;
                erase_ent(ufo_idx); ent_type[ufo_idx] = ENT_NONE;
                score = score + 200; ufo_active = 0; ufo_timer = 100 + cheap_rand(128);
            }
        }
    }

    /* Collision: ship vs rocks */
    if (alive && ent_type[0] == ENT_SHIP) {
        for (j = 1; j < MAX_ENTS; j++) {
            if (ent_type[j] < ENT_ROCK_L || ent_type[j] > ENT_ROCK_S) continue;
            if ((ent_px[0]>>3)==(ent_px[j]>>3) && (ent_py[0]>>3)==(ent_py[j]>>3)) {
                erase_ent(0); ent_type[0] = ENT_NONE; alive = 0;
                if (lives > 0) lives = lives - 1; spawn_timer = 0;
                if (lives == 0) do_game_over(); break;
            }
        }
    }

    /* Collision: ship vs UFO */
    if (alive && ufo_active && ent_type[0] == ENT_SHIP) {
        if ((ent_px[0]>>3)==(ent_px[ufo_idx]>>3) && (ent_py[0]>>3)==(ent_py[ufo_idx]>>3)) {
            erase_ent(0); ent_type[0] = ENT_NONE;
            erase_ent(ufo_idx); ent_type[ufo_idx] = ENT_NONE;
            alive = 0; ufo_active = 0;
            if (lives > 0) lives = lives - 1; spawn_timer = 0;
            if (lives == 0) do_game_over();
        }
    }

    /* Collision: UFO shots vs ship */
    if (alive && ent_type[0] == ENT_SHIP) {
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j] != ENT_USHOT) continue;
            if ((ent_px[0]>>3)==(ent_px[j]>>3) && (ent_py[0]>>3)==(ent_py[j]>>3)) {
                erase_ent(j); ent_type[j] = ENT_NONE;
                erase_ent(0); ent_type[0] = ENT_NONE; alive = 0;
                if (lives > 0) lives = lives - 1; spawn_timer = 0;
                if (lives == 0) do_game_over(); break;
            }
        }
    }

    if (count_rocks() == 0 && !game_over) { wave = wave + 1; draw_stars(); spawn_wave(); }

    for (i = 0; i < MAX_ENTS; i++) {
        if (ent_type[i] != ENT_NONE) {
            if (ent_type[i] == ENT_SHIP) erase_ent(i);
            draw_ent(i);
        }
    }
    draw_hud();
}

/* ---- Main ---- */
void main(void) {
    InitNGPC();
    SysSetSystemFont();
    install_tiles();
    setup_palettes();

    high_scores[0] = 1000; high_scores[1] = 800;
    high_scores[2] = 600; high_scores[3] = 400; high_scores[4] = 200;
    state = STATE_TITLE; skip = 10;
    pad_cur = 0; pad_prev = 0; rand_seed = 42;
    ufo_active = 0; ufo_timer = 200;
    draw_title();

    while (1) {
        WaitVsync();
        pad_prev = pad_cur;
        pad_cur = JOYPAD & 0x7F;
        pad_press = pad_cur & ~pad_prev;
        if (skip > 0) { skip--; continue; }

        if (state == STATE_TITLE) {
            rand_seed = rand_seed + VBCounter;
            if (pad_press & J_A) { state = STATE_GAME; skip = 10; game_start(); }
            if (pad_press & J_OPTION) { state = STATE_SCORES; skip = 10; draw_scores(); }
        } else if (state == STATE_GAME) {
            game_update();
        } else if (state == STATE_OVER) {
            if (pad_press & J_A) { state = STATE_GAME; skip = 10; game_start(); }
            if (pad_press & J_OPTION) { state = STATE_SCORES; skip = 10; draw_scores(); }
        } else if (state == STATE_SCORES) {
            if (pad_press & J_A) { state = STATE_GAME; skip = 10; game_start(); }
            if (pad_press & J_OPTION) { state = STATE_TITLE; skip = 10; draw_title(); }
        }
    }
}
