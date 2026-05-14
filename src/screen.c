/* screen.c — Title, HUD, scores, parallax starfield, palettes */
#include "screen.h"
#include "tiles.h"
#include "entities.h"

/* Stars on scroll plane 2 for parallax */
static u8 bg_scroll_x, bg_scroll_y;
static u8 bg_tick;

void setup_palettes(void) {
    SetBackgroundColour(RGB(0, 0, 0));
    SetPalette(SCR_1_PLANE, PAL_TEXT, 0, RGB(15,15,15), RGB(15,15,15), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_SHIP, 0, RGB(10,12,15), RGB(7,9,13), RGB(15,10,2));
    SetPalette(SCR_1_PLANE, PAL_ROCK1, 0, RGB(15,15,15), RGB(13,13,13), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_ROCK2, 0, RGB(12,12,12), RGB(10,10,10), RGB(14,14,14));
    SetPalette(SCR_1_PLANE, PAL_ROCK3, 0, RGB(9,9,9), RGB(7,7,7), RGB(11,11,11));
    SetPalette(SCR_1_PLANE, PAL_DIM, 0, RGB(4,4,4), RGB(3,3,3), RGB(5,5,5));
    SetPalette(SCR_1_PLANE, PAL_UFO, 0, RGB(6,15,6), RGB(4,12,4), RGB(8,15,8));
    SetPalette(SCR_1_PLANE, PAL_MARQUEE, 0, RGB(15,14,10), RGB(10,9,6), RGB(15,4,2));
    /* Scroll plane 2 star palettes */
    SetPalette(SCR_2_PLANE, 0, 0, RGB(3,3,4), RGB(2,2,3), RGB(5,5,6));
    SetPalette(SCR_2_PLANE, 1, 0, RGB(5,5,6), RGB(3,3,4), RGB(7,7,8));
}

void put_score_at(u8 x, u8 y, u16 val) {
    u16 v; v = val;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+4, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+3, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+2, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+1, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x, y, '0'+v%10);
}

void draw_hud(void) {
    put_score_at(0, 0, score);
    PrintString(SCR_1_PLANE, PAL_DIM, 7, 0, "LIVES:");
    PutTile(SCR_1_PLANE, PAL_TEXT, 13, 0, '0'+lives);
    PutTile(SCR_1_PLANE, PAL_DIM, 15, 0, 'W');
    PutTile(SCR_1_PLANE, PAL_TEXT, 16, 0, '0'+wave/10);
    PutTile(SCR_1_PLANE, PAL_TEXT, 17, 0, '0'+wave%10);
    if (warp_cooldown > 0) PutTile(SCR_1_PLANE, PAL_ROCK3, 19, 0, 'B');
    else PutTile(SCR_1_PLANE, PAL_SHIP, 19, 0, 'B');
}

/* ---- Parallax starfield on scroll plane 2 ---- */

void init_stars(void) {
    u8 x, y;
    /* Scatter star tiles across scroll plane 2's 32x32 tile map */
    ClearScreen(SCR_2_PLANE);
    for (y = 0; y < 24; y++) {
        for (x = 0; x < 32; x++) {
            if (cheap_rand(12) == 0)
                PutTile(SCR_2_PLANE, cheap_rand(2), x, y, T_STAR);
        }
    }
    bg_scroll_x = 0; bg_scroll_y = 0; bg_tick = 0;
}

void scroll_stars(void) {
    bg_tick++;
    if (bg_tick >= 3) {
        bg_tick = 0;
        bg_scroll_x++;
        if ((bg_scroll_x & 3) == 0) bg_scroll_y++;
        ShiftScroll(SCR_2_PLANE, bg_scroll_x, bg_scroll_y);
    }
}

void draw_stars(void) {
    /* Stars are on scroll plane 2 now, just call scroll */
    scroll_stars();
}

/* ---- Title ---- */

void draw_title(void) {
    u8 tx, ty;
    u16 base;
    ClearScreen(SCR_1_PLANE);
    ClearScreen(SCR_2_PLANE);
    setup_palettes(); SysSetSystemFont(); install_tiles();

    /* Marquee */
    for (ty = 0; ty < MARQUEE_H; ty++)
        for (tx = 0; tx < MARQUEE_W; tx++)
            PutTile(SCR_1_PLANE, PAL_MARQUEE, tx, ty+1, T_MARQUEE + ty*MARQUEE_W + tx);
    install_marquee();

    /* Ship NW thrust */
    base = T_THRST + 7 * 4;
    PutTile(SCR_1_PLANE, PAL_SHIP, 5, 8, base);
    PutTile(SCR_1_PLANE, PAL_SHIP, 6, 8, base+1);
    PutTile(SCR_1_PLANE, PAL_SHIP, 5, 9, base+2);
    PutTile(SCR_1_PLANE, PAL_SHIP, 6, 9, base+3);
    PutTile(SCR_1_PLANE, PAL_SHIP, 4, 7, T_BULLET);

    /* Alien */
    {
        u8 c, r;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                PutTile(SCR_1_PLANE, PAL_UFO, 14+c, 8+r, T_ALIEN + r*4 + c);
    }

    /* Rocks */
    PutTile(SCR_1_PLANE, PAL_ROCK1, 3, 10, T_ROCK_L);
    PutTile(SCR_1_PLANE, PAL_ROCK1, 4, 10, T_ROCK_L+1);
    PutTile(SCR_1_PLANE, PAL_ROCK1, 3, 11, T_ROCK_L+2);
    PutTile(SCR_1_PLANE, PAL_ROCK1, 4, 11, T_ROCK_L+3);
    PutTile(SCR_1_PLANE, PAL_ROCK2, 11, 11, T_ROCK_M);
    PutTile(SCR_1_PLANE, PAL_ROCK3, 8, 12, T_ROCK_S);

    /* Difficulty indicator */
    if (difficulty == DIFF_EASY) PrintString(SCR_1_PLANE, PAL_UFO, 6, 13, "< EASY >");
    else if (difficulty == DIFF_NORMAL) PrintString(SCR_1_PLANE, PAL_SHIP, 5, 13, "< NORMAL >");
    else PrintString(SCR_1_PLANE, PAL_ROCK1, 3, 13, "< OLD SCHOOL >");

    PrintString(SCR_1_PLANE, PAL_SHIP, 3, 15, "PRESS  A  START");
    PrintString(SCR_1_PLANE, PAL_DIM, 1, 16, "LR:DIFF U:THR B:WARP");
    PrintString(SCR_1_PLANE, PAL_DIM, 0, 17, "--------------------");
    PrintString(SCR_1_PLANE, PAL_DIM, 5, 18, "HI:");
    put_score_at(8, 18, high_scores[0]);
}

void draw_scores(void) {
    u8 i;
    ClearScreen(SCR_1_PLANE);
    setup_palettes(); SysSetSystemFont();
    PrintString(SCR_1_PLANE, PAL_SHIP, 3, 1, "HIGH  SCORES");
    PrintString(SCR_1_PLANE, PAL_DIM, 2, 2, "================");
    for (i = 0; i < 5; i++) {
        PutTile(SCR_1_PLANE, PAL_TEXT, 5, 4+i*2, '1'+i);
        PutTile(SCR_1_PLANE, PAL_TEXT, 6, 4+i*2, '.');
        put_score_at(8, 4+i*2, high_scores[i]);
    }
    PrintString(SCR_1_PLANE, PAL_ROCK2, 2, 16, "A:PLAY  OPT:TITLE");
}
