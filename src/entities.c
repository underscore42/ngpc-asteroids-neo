/* entities.c — Entity pool, movement, collision, rocks, UFO, bullets */
#include "entities.h"
#include "save.h"
#include "screen.h"
#include "tiles.h"

/* Ship state (local to entity system) */
static u8 ship_vx_add, ship_vx_sub, ship_vy_add, ship_vy_sub;
static u8 drift_timer, rot_tick, fire_tick;
static u8 thrust_snd_tick;

/* Ship tile masks: bit0=TL, bit1=TR, bit2=BL, bit3=BR */
static const u8 ship_mask[8] = { 0x0D,0x0F,0x0F,0x0F,0x07,0x0F,0x0F,0x0F };
static const u8 thrst_mask[8] = { 0x0D,0x0F,0x0F,0x0F,0x07,0x0F,0x0F,0x0F };

void init_entities(void) {
    u8 i;
    for (i = 0; i < MAX_ENTS; i++) {
        ent_type[i] = ENT_NONE; ent_otx[i] = 255; ent_oty[i] = 255; ent_pal[i] = 0;
    }
    ship_vx_add = 0; ship_vx_sub = 0;
    ship_vy_add = 0; ship_vy_sub = 0;
    drift_timer = 0; rot_tick = 0; fire_tick = 0;
    thrust_snd_tick = 0;
}

void erase_ent(u8 i) {
    u8 tx, ty, mask;
    tx = ent_otx[i]; ty = ent_oty[i];
    if (tx >= 20 || ty >= 19) return;
    if (ent_type[i] == ENT_SHIP) {
        if (thrusting) mask = thrst_mask[ent_dir[i]];
        else mask = ship_mask[ent_dir[i]];
        if (mask & 1) PutTile(SCR_1_PLANE, 0, tx, ty, ' ');
        if ((mask & 2) && tx+1 < 20) PutTile(SCR_1_PLANE, 0, tx+1, ty, ' ');
        if ((mask & 4) && ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx, ty+1, ' ');
        if ((mask & 8) && tx+1 < 20 && ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx+1, ty+1, ' ');
    } else if (ent_type[i] == ENT_UFO) {
        u8 c, r;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                if (tx+c < 20 && ty+r < 19)
                    PutTile(SCR_1_PLANE, 0, tx+c, ty+r, ' ');
    } else if (ent_type[i] == ENT_ROCK_L) {
        PutTile(SCR_1_PLANE, 0, tx, ty, ' ');
        if (tx+1 < 20) PutTile(SCR_1_PLANE, 0, tx+1, ty, ' ');
        if (ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx, ty+1, ' ');
        if (tx+1 < 20 && ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx+1, ty+1, ' ');
    } else {
        PutTile(SCR_1_PLANE, 0, tx, ty, ' ');
    }
}

void draw_ent(u8 i) {
    u8 tx, ty, mask;
    u16 base;
    tx = ent_px[i] >> 3; ty = ent_py[i] >> 3;
    if (tx >= 20) tx = 19;
    if (ty >= 19) ty = 18;

    if (ent_type[i] == ENT_SHIP) {
        if (tx >= 19) tx = 18; if (ty >= 18) ty = 17;
        if (thrusting) { base = T_THRST + (u16)ent_dir[i] * 4; mask = thrst_mask[ent_dir[i]]; }
        else { base = T_SHIP + (u16)ent_dir[i] * 4; mask = ship_mask[ent_dir[i]]; }
        if (mask & 1) PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, base);
        if (mask & 2) PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty, base+1);
        if (mask & 4) PutTile(SCR_1_PLANE, ent_pal[i], tx, ty+1, base+2);
        if (mask & 8) PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty+1, base+3);
    } else if (ent_type[i] == ENT_UFO) {
        u8 c, r;
        if (tx >= 17) tx = 16; if (ty >= 18) ty = 17;
        base = T_ALIEN;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                PutTile(SCR_1_PLANE, ent_pal[i], tx+c, ty+r, base + r*4 + c);
    } else if (ent_type[i] == ENT_ROCK_L) {
        if (tx >= 19) tx = 18; if (ty >= 18) ty = 17;
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_L);
        PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty, T_ROCK_L+1);
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty+1, T_ROCK_L+2);
        PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty+1, T_ROCK_L+3);
    } else if (ent_type[i] == ENT_ROCK_M) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_M);
    } else if (ent_type[i] == ENT_ROCK_S) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_S);
    } else if (ent_type[i] == ENT_BULLET) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_BULLET);
    } else if (ent_type[i] == ENT_USHOT) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_USHOT);
    }
    ent_otx[i] = tx; ent_oty[i] = ty;
}

void move_ent(u8 i) {
    u8 d;
    ent_tick[i] = ent_tick[i] + 1;
    if (ent_tick[i] < ent_spd[i]) return;
    ent_tick[i] = 0;
    d = ent_dir[i];
    if (dx_add[d]) { ent_px[i]=ent_px[i]+1; if (ent_px[i]>=160) ent_px[i]=0; }
    if (dx_sub[d]) { if (ent_px[i]==0) ent_px[i]=159; else ent_px[i]=ent_px[i]-1; }
    if (dy_add[d]) { ent_py[i]=ent_py[i]+1; if (ent_py[i]>=152) ent_py[i]=8; }
    if (dy_sub[d]) { if (ent_py[i]<=8) ent_py[i]=151; else ent_py[i]=ent_py[i]-1; }
}

u8 check_hit(u8 a, u8 b) {
    u8 ax, ay, bx, by, aw, ah, bw, bh;
    ax = ent_px[a] >> 3; ay = ent_py[a] >> 3;
    bx = ent_px[b] >> 3; by = ent_py[b] >> 3;
    if (ent_type[a] == ENT_SHIP) { aw = 2; ah = 2; }
    else if (ent_type[a] == ENT_UFO) { aw = 4; ah = 2; }
    else if (ent_type[a] == ENT_ROCK_L) { aw = 2; ah = 2; }
    else { aw = 1; ah = 1; }
    if (ent_type[b] == ENT_SHIP) { bw = 2; bh = 2; }
    else if (ent_type[b] == ENT_UFO) { bw = 4; bh = 2; }
    else if (ent_type[b] == ENT_ROCK_L) { bw = 2; bh = 2; }
    else { bw = 1; bh = 1; }
    if (ax+aw <= bx || bx+bw <= ax) return 0;
    if (ay+ah <= by || by+bh <= ay) return 0;
    return 1;
}

/* ---- Rocks ---- */

void spawn_rock(u8 type, u8 px, u8 py) {
    u8 idx;
    idx = find_free(); if (idx == 255) return;
    ent_type[idx] = type; ent_px[idx] = px; ent_py[idx] = py;
    ent_dir[idx] = cheap_rand(8); ent_tick[idx] = 0;
    ent_otx[idx] = 255; ent_oty[idx] = 255; ent_life[idx] = 0;
    if (type == ENT_ROCK_L) { ent_spd[idx] = 6; ent_pal[idx] = PAL_ROCK1; }
    else if (type == ENT_ROCK_M) { ent_spd[idx] = 4; ent_pal[idx] = PAL_ROCK2; }
    else { ent_spd[idx] = 3; ent_pal[idx] = PAL_ROCK3; }
}

void spawn_wave(void) {
    u8 count, i;
    count = 3 + wave; if (count > 8) count = 8;
    for (i = 0; i < count; i++) {
        if (cheap_rand(2) == 0)
            spawn_rock(ENT_ROCK_L, cheap_rand(160), cheap_rand(2)==0 ? 12 : 140);
        else
            spawn_rock(ENT_ROCK_L, cheap_rand(2)==0 ? 4 : 152, 12+cheap_rand(128));
    }
}

void destroy_rock(u8 idx) {
    u8 t, px, py;
    t = ent_type[idx]; px = ent_px[idx]; py = ent_py[idx];
    if (t == ENT_ROCK_L) score = score + 20;
    else if (t == ENT_ROCK_M) score = score + 50;
    else score = score + 100;
    erase_ent(idx); ent_type[idx] = ENT_NONE;
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
    idx = find_free(); if (idx == 255) return;
    ent_type[idx] = ENT_BULLET;
    ent_px[idx] = ent_px[0]; ent_py[idx] = ent_py[0];
    ent_dir[idx] = ship_dir;
    ent_spd[idx] = 1; ent_tick[idx] = 0;
    ent_life[idx] = 40; ent_pal[idx] = PAL_SHIP;
    ent_otx[idx] = 255; ent_oty[idx] = 255;
}

/* ---- UFO ---- */

void spawn_ufo(void) {
    u8 idx, side;
    if (ufo_active) return;
    idx = find_free(); if (idx == 255) return;
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
    idx = find_free(); if (idx == 255) return;
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

/* ---- Warp ---- */

static void do_game_over(void) {
    game_over = 1; state = STATE_OVER;
    insert_high_score(score);
    save_high_scores();
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
