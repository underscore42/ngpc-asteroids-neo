/* entities.c — Entity pool with SPRITE PLANE rendering
 * All game entities use hardware sprites for true transparency.
 * Scroll planes reserved for background/HUD.
 */
#include "entities.h"
#include "save.h"
#include "screen.h"
#include "tiles.h"

static u8 ship_vx_add, ship_vx_sub, ship_vy_add, ship_vy_sub;
static u8 drift_timer, thrust_snd_tick;

static const u8 ship_mask[8] = { 0x0D,0x0F,0x0F,0x0F,0x07,0x0F,0x0F,0x0F };
static const u8 thrst_mask[8] = { 0x0D,0x0F,0x0F,0x0F,0x07,0x0F,0x0F,0x0F };

/* Sprite allocation: entity index -> base sprite number
 * ent 0 (ship): sprites 0-3  (2x2)
 * ent 1..15:    sprites 4 + (i-1)*4 = 4..63
 * Max 16 entities using sprites (ent 0..15)
 */
#define SPR_PER_ENT 4

static u8 ent_spr(u8 i) {
    return i * SPR_PER_ENT;
}

/* Sprite palettes (separate from scroll palettes) */
void setup_sprite_palettes(void) {
    SetPalette(SPRITE_PLANE, PAL_SHIP, 0, RGB(10,12,15), RGB(7,9,13), RGB(15,10,2));
    SetPalette(SPRITE_PLANE, PAL_ROCK1, 0, RGB(15,15,15), RGB(13,13,13), RGB(15,15,15));
    SetPalette(SPRITE_PLANE, PAL_ROCK2, 0, RGB(12,12,12), RGB(10,10,10), RGB(14,14,14));
    SetPalette(SPRITE_PLANE, PAL_ROCK3, 0, RGB(9,9,9), RGB(7,7,7), RGB(11,11,11));
    SetPalette(SPRITE_PLANE, PAL_UFO, 0, RGB(6,15,6), RGB(4,12,4), RGB(8,15,8));
    SetPalette(SPRITE_PLANE, PAL_TEXT, 0, RGB(15,15,15), RGB(15,15,15), RGB(15,15,15));
}

void init_entities(void) {
    u8 i;
    for (i = 0; i < MAX_ENTS; i++) {
        ent_type[i] = ENT_NONE; ent_otx[i] = 255; ent_oty[i] = 255; ent_pal[i] = 0;
    }
    for (i = 0; i < 64; i++) UnsetSprite(i);
    ship_vx_add = 0; ship_vx_sub = 0;
    ship_vy_add = 0; ship_vy_sub = 0;
    drift_timer = 0; thrust_snd_tick = 0;
    setup_sprite_palettes();
}

void erase_ent(u8 i) {
    u8 base, s;
    if (i >= 16) return;
    base = ent_spr(i);
    for (s = 0; s < SPR_PER_ENT; s++) UnsetSprite(base + s);
    /* UFO needs 8 sprites */
    if (ent_type[i] == ENT_UFO && i < 15) {
        for (s = 0; s < SPR_PER_ENT; s++) UnsetSprite(base + SPR_PER_ENT + s);
    }
}

void draw_ent(u8 i) {
    u8 px, py, base;
    u16 tb;
    if (i >= 16) return;
    px = ent_px[i]; py = ent_py[i];
    base = ent_spr(i);

    if (ent_type[i] == ENT_SHIP) {
        /* Flash during spawn grace */
        if (spawn_grace > 0 && (spawn_grace & 4)) {
            UnsetSprite(base); UnsetSprite(base+1);
            UnsetSprite(base+2); UnsetSprite(base+3);
            ent_otx[i] = px >> 3; ent_oty[i] = py >> 3;
            return;
        }
        if (thrusting) tb = T_THRST + (u16)ent_dir[i] * 4;
        else tb = T_SHIP + (u16)ent_dir[i] * 4;
        SetSprite(base,   tb,   0, px,   py,   ent_pal[i]);
        SetSprite(base+1, tb+1, 0, px+8, py,   ent_pal[i]);
        SetSprite(base+2, tb+2, 0, px,   py+8, ent_pal[i]);
        SetSprite(base+3, tb+3, 0, px+8, py+8, ent_pal[i]);
    } else if (ent_type[i] == ENT_UFO) {
        u8 c, r;
        tb = T_ALIEN;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                SetSprite(base + r*4 + c, tb + r*4 + c, 0,
                          px + c*8, py + r*8, ent_pal[i]);
    } else if (ent_type[i] == ENT_ROCK_L) {
        tb = T_ROCK_L;
        SetSprite(base,   tb,   0, px,   py,   ent_pal[i]);
        SetSprite(base+1, tb+1, 0, px+8, py,   ent_pal[i]);
        SetSprite(base+2, tb+2, 0, px,   py+8, ent_pal[i]);
        SetSprite(base+3, tb+3, 0, px+8, py+8, ent_pal[i]);
    } else if (ent_type[i] == ENT_ROCK_M) {
        SetSprite(base, T_ROCK_M, 0, px, py, ent_pal[i]);
    } else if (ent_type[i] == ENT_ROCK_S) {
        SetSprite(base, T_ROCK_S, 0, px, py, ent_pal[i]);
    } else if (ent_type[i] == ENT_BULLET) {
        SetSprite(base, T_BULLET, 0, px, py, ent_pal[i]);
    } else if (ent_type[i] == ENT_USHOT) {
        SetSprite(base, T_USHOT, 0, px, py, ent_pal[i]);
    } else if (ent_type[i] == ENT_EXPLODE) {
        /* Expanding burst: 4 sub-sprites move outward over ent_life */
        u8 spread;
        u16 frame;
        spread = 20 - ent_life[i];
        if (spread > 16) spread = 16;
        frame = T_EXPLODE + (spread >> 2);
        if (frame > T_EXPLODE + 3) frame = T_EXPLODE + 3;
        SetSprite(base,   frame, 0, px - spread, py - spread, ent_pal[i]);
        SetSprite(base+1, frame, 0, px + spread, py - spread, ent_pal[i]);
        SetSprite(base+2, frame, 0, px - spread, py + spread, ent_pal[i]);
        SetSprite(base+3, frame, 0, px + spread, py + spread, ent_pal[i]);
    }
    ent_otx[i] = px >> 3;
    ent_oty[i] = py >> 3;
}

void move_ent(u8 i) {
    u8 d;
    /* Explosions: just tick down, don't move */
    if (ent_type[i] == ENT_EXPLODE) {
        if (ent_life[i] > 0) ent_life[i] = ent_life[i] - 1;
        if (ent_life[i] == 0) { erase_ent(i); ent_type[i] = ENT_NONE; }
        return;
    }
    ent_tick[i] = ent_tick[i] + 1;
    if (ent_tick[i] < ent_spd[i]) return;
    ent_tick[i] = 0;
    d = ent_dir[i];
    if (dx_add[d]) { ent_px[i]=ent_px[i]+1; if (ent_px[i]>=160) ent_px[i]=0; }
    if (dx_sub[d]) { if (ent_px[i]==0) ent_px[i]=159; else ent_px[i]=ent_px[i]-1; }
    if (dy_add[d]) { ent_py[i]=ent_py[i]+1; if (ent_py[i]>=152) ent_py[i]=8; }
    if (dy_sub[d]) { if (ent_py[i]<=8) ent_py[i]=151; else ent_py[i]=ent_py[i]-1; }
}

/* Pixel-based collision with smaller hitboxes */
u8 check_hit(u8 a, u8 b) {
    u8 ax, ay, bx, by, aw, ah, bw, bh, shrink;
    ax = ent_px[a]; ay = ent_py[a];
    bx = ent_px[b]; by = ent_py[b];
    /* Sizes in pixels */
    if (ent_type[a] == ENT_SHIP) { aw = 14; ah = 14; }
    else if (ent_type[a] == ENT_UFO) { aw = 30; ah = 14; }
    else if (ent_type[a] == ENT_ROCK_L) { aw = 14; ah = 14; }
    else if (ent_type[a] == ENT_ROCK_M) { aw = 7; ah = 7; }
    else { aw = 5; ah = 5; }
    if (ent_type[b] == ENT_SHIP) { bw = 14; bh = 14; }
    else if (ent_type[b] == ENT_UFO) { bw = 30; bh = 14; }
    else if (ent_type[b] == ENT_ROCK_L) { bw = 14; bh = 14; }
    else if (ent_type[b] == ENT_ROCK_M) { bw = 7; bh = 7; }
    else { bw = 5; bh = 5; }
    /* Shrink hitboxes by difficulty */
    shrink = difficulty * 2;
    if (aw > shrink + 4) { ax = ax + shrink; aw = aw - shrink - shrink; }
    if (ah > shrink + 4) { ay = ay + shrink; ah = ah - shrink - shrink; }
    if (bw > shrink + 4) { bx = bx + shrink; bw = bw - shrink - shrink; }
    if (bh > shrink + 4) { by = by + shrink; bh = bh - shrink - shrink; }
    /* AABB overlap */
    if (ax + aw <= bx || bx + bw <= ax) return 0;
    if (ay + ah <= by || by + bh <= ay) return 0;
    return 1;
}


/* ---- Ship movement with momentum/drift ---- */
void move_ship(void) {
    /* Thrust: set drift velocity to facing direction */
    if (thrusting) {
        ship_vx_add = dx_add[ship_dir];
        ship_vx_sub = dx_sub[ship_dir];
        ship_vy_add = dy_add[ship_dir];
        ship_vy_sub = dy_sub[ship_dir];
    }
    /* Apply drift */
    drift_timer++;
    if (drift_timer >= 3) {
        drift_timer = 0;
        if (ship_vx_add) { ent_px[0]=ent_px[0]+1; if (ent_px[0]>=160) ent_px[0]=0; }
        if (ship_vx_sub) { if (ent_px[0]==0) ent_px[0]=159; else ent_px[0]=ent_px[0]-1; }
        if (ship_vy_add) { ent_py[0]=ent_py[0]+1; if (ent_py[0]>=152) ent_py[0]=8; }
        if (ship_vy_sub) { if (ent_py[0]<=8) ent_py[0]=151; else ent_py[0]=ent_py[0]-1; }
    }
}

/* ---- Rocks ---- */

void spawn_rock(u8 type, u8 px, u8 py) {
    u8 idx;
    idx = find_free(); if (idx == 255 || idx >= 16) return;
    ent_type[idx] = type; ent_px[idx] = px; ent_py[idx] = py;
    ent_dir[idx] = cheap_rand(8); ent_tick[idx] = 0;
    ent_otx[idx] = 255; ent_oty[idx] = 255; ent_life[idx] = 0;
    if (type == ENT_ROCK_L) { ent_spd[idx] = 6; ent_pal[idx] = PAL_ROCK1; }
    else if (type == ENT_ROCK_M) { ent_spd[idx] = 4; ent_pal[idx] = PAL_ROCK2; }
    else { ent_spd[idx] = 3; ent_pal[idx] = PAL_ROCK3; }
}

/* Spawn explosion effect at position */
void spawn_explosion(u8 px, u8 py, u8 pal) {
    u8 idx;
    idx = find_free(); if (idx == 255 || idx >= 16) return;
    ent_type[idx] = ENT_EXPLODE;
    ent_px[idx] = px; ent_py[idx] = py;
    ent_life[idx] = 20; ent_pal[idx] = pal;
    ent_spd[idx] = 1; ent_tick[idx] = 0;
    ent_dir[idx] = 0;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
}

void spawn_wave(void) {
    u8 count, i;
    /* Fewer rocks early, scales with wave */
    if (difficulty == 0) count = 2 + wave;
    else if (difficulty == 1) count = 3 + wave;
    else count = 4 + wave;
    if (count > 7) count = 7;
    for (i = 0; i < count; i++) {
        if (cheap_rand(2) == 0)
            spawn_rock(ENT_ROCK_L, cheap_rand(140), cheap_rand(2)==0 ? 12 : 136);
        else
            spawn_rock(ENT_ROCK_L, cheap_rand(2)==0 ? 4 : 148, 16+cheap_rand(120));
    }
}

void destroy_rock(u8 idx) {
    u8 t, px, py;
    t = ent_type[idx]; px = ent_px[idx]; py = ent_py[idx];
    if (t == ENT_ROCK_L) score = score + 20;
    else if (t == ENT_ROCK_M) score = score + 50;
    else score = score + 100;
    erase_ent(idx); ent_type[idx] = ENT_NONE;
    spawn_explosion(px, py, PAL_ROCK1);
    if (t == ENT_ROCK_L) { spawn_rock(ENT_ROCK_M, px+4, py); spawn_rock(ENT_ROCK_M, px, py+4); }
    else if (t == ENT_ROCK_M) { spawn_rock(ENT_ROCK_S, px+2, py); spawn_rock(ENT_ROCK_S, px, py+2); }
}

u8 count_rocks(void) {
    u8 i, c; c = 0;
    for (i = 0; i < MAX_ENTS; i++)
        if (ent_type[i] >= ENT_ROCK_L && ent_type[i] <= ENT_ROCK_S) c++;
    return c;
}

/* ---- Bullets ---- */

void fire_bullet(void) {
    u8 idx;
    idx = find_free(); if (idx == 255 || idx >= 16) return;
    ent_type[idx] = ENT_BULLET;
    ent_px[idx] = ent_px[0]; ent_py[idx] = ent_py[0];
    ent_dir[idx] = ship_dir;
    ent_spd[idx] = 1; ent_tick[idx] = 0;
    ent_life[idx] = 60; ent_pal[idx] = PAL_SHIP;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
}

/* ---- UFO ---- */

void spawn_ufo(void) {
    u8 idx, side;
    if (ufo_active) return;
    idx = find_free(); if (idx == 255 || idx >= 15) return;
    ufo_active = 1; ufo_idx = idx; ufo_fire_tmr = UFO_FIRE_RATE;
    ent_type[idx] = ENT_UFO; ent_pal[idx] = PAL_UFO;
    ent_tick[idx] = 0; ent_otx[idx] = 255; ent_oty[idx] = 255;
    side = cheap_rand(2);
    if (side) { ent_px[idx] = 0; ent_dir[idx] = 2; }
    else { ent_px[idx] = 128; ent_dir[idx] = 6; }
    ent_py[idx] = 20 + cheap_rand(100); ent_spd[idx] = 3;
}

static void ufo_fire(void) {
    u8 idx, aim_dir;
    if (!ufo_active || !alive) return;
    idx = find_free(); if (idx == 255 || idx >= 16) return;
    ent_type[idx] = ENT_USHOT;
    ent_px[idx] = ent_px[ufo_idx]; ent_py[idx] = ent_py[ufo_idx];
    ent_spd[idx] = 2; ent_tick[idx] = 0;
    ent_life[idx] = 60; ent_pal[idx] = PAL_UFO;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
    if (ent_px[0] > ent_px[ufo_idx]+16) {
        if (ent_py[0]+16 < ent_py[ufo_idx]) aim_dir = 1;
        else if (ent_py[0] > ent_py[ufo_idx]+16) aim_dir = 3;
        else aim_dir = 2;
    } else if (ent_px[0]+16 < ent_px[ufo_idx]) {
        if (ent_py[0]+16 < ent_py[ufo_idx]) aim_dir = 7;
        else if (ent_py[0] > ent_py[ufo_idx]+16) aim_dir = 5;
        else aim_dir = 6;
    } else {
        if (ent_py[0]+16 < ent_py[ufo_idx]) aim_dir = 0;
        else aim_dir = 4;
    }
    if (cheap_rand(3)==0) aim_dir = (aim_dir+1) & 7;
    else if (cheap_rand(3)==0) aim_dir = (aim_dir+7) & 7;
    ent_dir[idx] = aim_dir;
}

void update_ufo(void) {
    if (!ufo_active) {
        if (ufo_timer > 0) ufo_timer--;
        else { spawn_ufo(); ufo_timer = 100 + cheap_rand(100); }
        return;
    }
    if ((ent_dir[ufo_idx]==2 && ent_px[ufo_idx] > 155) ||
        (ent_dir[ufo_idx]==6 && ent_px[ufo_idx] < 3)) {
        erase_ent(ufo_idx); ent_type[ufo_idx] = ENT_NONE;
        ufo_active = 0; ufo_timer = 60 + cheap_rand(80); return;
    }
    if (ufo_fire_tmr > 0) ufo_fire_tmr--;
    else {
        ufo_fire(); PlaySound(SND_UFO_FIRE);
        ufo_fire_tmr = UFO_FIRE_RATE - (wave * 5);
        if (ufo_fire_tmr < 30) ufo_fire_tmr = 30;
    }
}

/* ---- Warp / Game Over ---- */

static void do_game_over(void) {
    game_over = 1; state = STATE_OVER;
    spawn_explosion(ent_px[0], ent_py[0], PAL_SHIP);
    insert_high_score(score); save_high_scores();
    PlaySound(SND_EXPLODE);
    PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
    PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
    skip = 30;
}

void warp_ship(void) {
    if (warp_cooldown > 0) return;
    erase_ent(0);
    ent_px[0] = 16+cheap_rand(128); ent_py[0] = 16+cheap_rand(112);
    ship_vx_add = 0; ship_vx_sub = 0; ship_vy_add = 0; ship_vy_sub = 0;
    drift_timer = 0; ent_otx[0] = 255; ent_oty[0] = 255;
    warp_cooldown = WARP_COOLDOWN;
    PlaySound(SND_WARP);
    if (cheap_rand(10)==0) {
        ent_type[0] = ENT_NONE; alive = 0;
        if (lives > 0) lives = lives - 1; spawn_timer = 0;
        if (lives == 0) do_game_over();
    }
}
