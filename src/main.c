/* main.c — Asteroids SNK Edition
 * Entry point, game loop, state machine
 * Entities on sprite plane, starfield on scroll plane 2
 */

#define CARTHDR_IMPL
#include "carthdr.h"
#include "game.h"
#include "tiles.h"
#include "screen.h"
#include "entities.h"
#include "sound.h"
#include "save.h"

static u8 rot_tick;

static void game_start(void) {
    u8 i;
    ClearScreen(SCR_1_PLANE);
    setup_palettes(); SysSetSystemFont(); install_tiles();
    init_entities();

    ent_type[0] = ENT_SHIP; ent_px[0] = 76; ent_py[0] = 72;
    ent_dir[0] = 0; ent_spd[0] = 3; ent_tick[0] = 0;
    ent_pal[0] = PAL_SHIP; ent_otx[0] = 255; ent_oty[0] = 255;

    ship_dir = 0; thrusting = 0; rot_tick = 0;
    spawn_grace = 90;
    score = 0; wave = 1; spawn_timer = 0;
    game_over = 0; alive = 1; warp_cooldown = 0;
    ufo_active = 0; ufo_timer = 100; ufo_fire_tmr = UFO_FIRE_RATE;

    /* Lives by difficulty */
    if (difficulty == DIFF_EASY) lives = 5;
    else if (difficulty == DIFF_NORMAL) lives = 3;
    else lives = 3;

    init_stars(); spawn_wave(); draw_hud();
}

static void game_update(void) {
    u8 i, j, bcount;
    if (game_over) return;
    if (warp_cooldown > 0) warp_cooldown--;
    if (pad_press & J_OPTION) {
        /* Clear all sprites before returning to title */
        for (i = 0; i < 64; i++) UnsetSprite(i);
        state = STATE_TITLE; skip = 10; draw_title(); return;
    }
    update_ufo();

    /* Parallax drift */
    scroll_stars();

    /* Ship respawn */
    if (!alive) {
        spawn_timer++;
        if (spawn_timer > 60 && lives > 0) {
            spawn_timer = 0;
            ent_type[0] = ENT_SHIP; ent_px[0] = 76; ent_py[0] = 72;
            ent_dir[0] = 0; ent_spd[0] = 3; ent_tick[0] = 0;
            ent_pal[0] = PAL_SHIP; ent_otx[0] = 255; ent_oty[0] = 255;
            ship_dir = 0; warp_cooldown = 0; alive = 1;
            spawn_grace = 90;
        }
    }

    /* Ship controls */
    if (alive && ent_type[0] == ENT_SHIP) {
        rot_tick++;
        if (rot_tick >= 8) {
            rot_tick = 0;
            if (pad_cur & J_LEFT) {
                if (ship_dir == 0) ship_dir = 7; else ship_dir = ship_dir - 1;
                ent_dir[0] = ship_dir;
            }
            if (pad_cur & J_RIGHT) {
                ship_dir = ship_dir + 1; if (ship_dir > 7) ship_dir = 0;
                ent_dir[0] = ship_dir;
            }
        }

        thrusting = 0;
        if (pad_cur & J_UP) thrusting = 1;

        move_ship();

        if (pad_press & J_A) {
            bcount = 0;
            for (i = 0; i < MAX_ENTS; i++) if (ent_type[i]==ENT_BULLET) bcount++;
            if (bcount < 4) { fire_bullet(); PlaySound(SND_FIRE); }
        }
        if ((pad_press & J_B) && alive) warp_ship();
    }

    /* Move non-ship entities */
    for (i = 1; i < MAX_ENTS; i++) {
        if (ent_type[i]==ENT_NONE) continue;
        move_ent(i);
        if (ent_type[i]==ENT_BULLET || ent_type[i]==ENT_USHOT) {
            if (ent_life[i] > 0) ent_life[i] = ent_life[i]-1;
            if (ent_life[i]==0) { erase_ent(i); ent_type[i]=ENT_NONE; continue; }
        }
    }

    /* Spawn grace: tick down, flash ship */
    if (spawn_grace > 0) spawn_grace--;

    /* Collisions: bullets vs rocks */
    for (i = 0; i < MAX_ENTS; i++) {
        if (ent_type[i]!=ENT_BULLET) continue;
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j]<ENT_ROCK_L || ent_type[j]>ENT_ROCK_S) continue;
            if (check_hit(i,j)) {
                erase_ent(i); ent_type[i]=ENT_NONE;
                destroy_rock(j); break;
            }
        }
    }
    /* Bullets vs UFO */
    if (ufo_active) {
        for (i = 0; i < MAX_ENTS; i++) {
            if (ent_type[i]!=ENT_BULLET) continue;
            if (check_hit(i, ufo_idx)) {
                erase_ent(i); ent_type[i]=ENT_NONE;
                erase_ent(ufo_idx); ent_type[ufo_idx]=ENT_NONE;
                score=score+200; ufo_active=0; ufo_timer=60+cheap_rand(80);
                PlaySound(SND_EXPLODE);
            }
        }
    }
    /* Ship vs rocks (skip during grace period) */
    if (alive && ent_type[0]==ENT_SHIP && spawn_grace==0) {
        for (j = 1; j < MAX_ENTS; j++) {
            if (ent_type[j]<ENT_ROCK_L || ent_type[j]>ENT_ROCK_S) continue;
            if (check_hit(0,j)) {
                spawn_explosion(ent_px[0], ent_py[0], PAL_SHIP);
                erase_ent(0); ent_type[0]=ENT_NONE; alive=0;
                PlaySound(SND_EXPLODE);
                if (lives>0) lives=lives-1; spawn_timer=0;
                if (lives==0) { game_over=1; state=STATE_OVER;
                    insert_high_score(score); save_high_scores();
                    PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
                    PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
                    skip=30; }
                break;
            }
        }
    }
    /* Ship vs UFO */
    if (alive && ufo_active && ent_type[0]==ENT_SHIP && spawn_grace==0) {
        if (check_hit(0, ufo_idx)) {
            spawn_explosion(ent_px[0], ent_py[0], PAL_SHIP);
            erase_ent(0); ent_type[0]=ENT_NONE;
            spawn_explosion(ent_px[ufo_idx], ent_py[ufo_idx], PAL_UFO);
            erase_ent(ufo_idx); ent_type[ufo_idx]=ENT_NONE;
            alive=0; ufo_active=0; PlaySound(SND_EXPLODE);
            if (lives>0) lives=lives-1; spawn_timer=0;
            if (lives==0) { game_over=1; state=STATE_OVER;
                insert_high_score(score); save_high_scores();
                PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
                PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
                skip=30; }
        }
    }
    /* UFO shots vs ship */
    if (alive && ent_type[0]==ENT_SHIP && spawn_grace==0) {
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j]!=ENT_USHOT) continue;
            if (check_hit(0,j)) {
                erase_ent(j); ent_type[j]=ENT_NONE;
                spawn_explosion(ent_px[0], ent_py[0], PAL_SHIP);
                erase_ent(0); ent_type[0]=ENT_NONE; alive=0;
                PlaySound(SND_EXPLODE);
                if (lives>0) lives=lives-1; spawn_timer=0;
                if (lives==0) { game_over=1; state=STATE_OVER;
                    insert_high_score(score); save_high_scores();
                    PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
                    PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
                    skip=30; }
                break;
            }
        }
    }

    /* Next wave */
    if (count_rocks()==0 && !game_over) { wave=wave+1; spawn_wave(); }

    /* Draw all entities (sprite plane — no erase needed, just update positions) */
    for (i = 0; i < 16; i++) {
        if (ent_type[i]!=ENT_NONE) draw_ent(i);
    }
    draw_hud();
}

/* ---- Entry point ---- */

void main(void) {
    InitNGPC(); SysSetSystemFont(); install_tiles(); setup_palettes();
    sound_init();
    load_high_scores();
    difficulty = DIFF_NORMAL;

    state = STATE_TITLE; skip = 10;
    pad_cur = 0; pad_prev = 0; rand_seed = 42;
    ufo_active = 0; ufo_timer = 100;
    draw_title();

    while (1) {
        WaitVsync();
        pad_prev = pad_cur; pad_cur = JOYPAD & 0x7F;
        pad_press = pad_cur & ~pad_prev;
        if (skip > 0) { skip--; continue; }

        if (state==STATE_TITLE) {
            rand_seed = rand_seed + VBCounter;
            /* L/R to change difficulty */
            if (pad_press & J_LEFT) {
                if (difficulty > 0) difficulty = difficulty - 1;
                draw_title();
            }
            if (pad_press & J_RIGHT) {
                if (difficulty < 2) difficulty = difficulty + 1;
                draw_title();
            }
            if (pad_press & J_A) { state=STATE_GAME; skip=10; game_start(); }
            if (pad_press & J_OPTION) { state=STATE_SCORES; skip=10; draw_scores(); }
        } else if (state==STATE_GAME) {
            game_update();
        } else if (state==STATE_OVER) {
            if (pad_press & J_A) { state=STATE_GAME; skip=10; game_start(); }
            if (pad_press & J_OPTION) { state=STATE_SCORES; skip=10; draw_scores(); }
        } else if (state==STATE_SCORES) {
            if (pad_press & J_A) { state=STATE_GAME; skip=10; game_start(); }
            if (pad_press & J_OPTION) { state=STATE_TITLE; skip=10; draw_title(); }
        }
    }
}
