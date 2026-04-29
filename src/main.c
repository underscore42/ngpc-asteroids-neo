#include "ngpc.h"
#define CARTHDR_IMPL
#include "carthdr.h"
#include "library.h"

#define STATE_MENU   0
#define STATE_LANDER 1

static u8 pad_cur   = 0;
static u8 pad_prev  = 0;
static u8 pad_press = 0;
static u8 state     = STATE_MENU;
static u8 menu_cursor = 0;
static u8 skip = 0;

/* Lander state */
static u8 lx;
static u8 ly;
static u8 lfuel;
static u8 ldone;
static u8 grav_tick;
static u8 thrust_tick;
static u8 drift_tick;
static u8 fall_speed;
static u8 angle;
static u8 drift_dir;    /* 0=none, 1=left, 2=right */

#define PAD_LEFT   8
#define PAD_RIGHT  11
#define GROUND_ROW 17
#define TOP_ROW    4
#define ANG_LEFT   0
#define ANG_CENTER 1
#define ANG_RIGHT  2

static void read_input(void)
{
    pad_prev  = pad_cur;
    pad_cur   = JOYPAD & 0x7F;
    pad_press = pad_cur & ~pad_prev;
}

static void show_menu(void)
{
    ClearScreen(SCR_1_PLANE);
    PrintString(SCR_1_PLANE, 0, 2, 1, "SPACE COLLECTION");
    PrintString(SCR_1_PLANE, 0, 4, 5, "LUNAR LANDER");
    PrintString(SCR_1_PLANE, 0, 4, 7, "SPACEWAR");
    PrintString(SCR_1_PLANE, 0, 4, 9, "LUNAR BUGGY");
    PrintString(SCR_1_PLANE, 0, 4, 11, "ASTEROIDS");
    PrintString(SCR_1_PLANE, 0, 4, 15, "A:SELECT");
    PrintString(SCR_1_PLANE, 1, 2, 5 + menu_cursor * 2, ">");
}

static u8 lander_char(void)
{
    if (angle == ANG_LEFT) return '<';
    if (angle == ANG_RIGHT) return '>';
    return 'V';
}

static void draw_hud(void)
{
    u8 alt;
    if (ly < GROUND_ROW) {
        alt = GROUND_ROW - ly;
    } else {
        alt = 0;
    }
    PutTile(SCR_1_PLANE, 0, 5, 0, '0' + alt / 10);
    PutTile(SCR_1_PLANE, 0, 6, 0, '0' + alt % 10);
    PutTile(SCR_1_PLANE, 0, 5, 1, '0' + fall_speed / 10);
    PutTile(SCR_1_PLANE, 0, 6, 1, '0' + fall_speed % 10);
    PutTile(SCR_1_PLANE, 0, 5, 2, '0' + lfuel / 10);
    PutTile(SCR_1_PLANE, 0, 6, 2, '0' + lfuel % 10);
    PutTile(SCR_1_PLANE, 0, 5, 3, lander_char());
}

static void show_lander(void)
{
    u8 x;

    lx = 10;
    ly = 4;
    lfuel = 99;
    ldone = 0;
    grav_tick = 0;
    thrust_tick = 0;
    drift_tick = 0;
    fall_speed = 0;
    angle = ANG_CENTER;
    drift_dir = 0;

    ClearScreen(SCR_1_PLANE);

    for (x = 0; x < 20; x++) {
        if (x >= PAD_LEFT && x <= PAD_RIGHT) {
            PutTile(SCR_1_PLANE, 1, x, 17, '=');
            PutTile(SCR_1_PLANE, 1, x, 18, '=');
        } else {
            PutTile(SCR_1_PLANE, 0, x, 17, '#');
            PutTile(SCR_1_PLANE, 0, x, 18, '#');
        }
    }

    PrintString(SCR_1_PLANE, 0, 0, 0, "ALT:");
    PrintString(SCR_1_PLANE, 0, 0, 1, "SPD:");
    PrintString(SCR_1_PLANE, 0, 0, 2, "FUEL:");
    PrintString(SCR_1_PLANE, 0, 0, 3, "ANG:");
    draw_hud();
    PutTile(SCR_1_PLANE, 0, lx, ly, lander_char());
}

static void update_lander(void)
{
    u8 on_pad;

    if (ldone) {
        if (pad_press & J_B) {
            show_lander();
            skip = 10;
        }
        return;
    }

    /* Rotation */
    if (pad_press & J_LEFT) {
        PutTile(SCR_1_PLANE, 0, lx, ly, ' ');
        if (angle > ANG_LEFT) angle = angle - 1;
    }
    if (pad_press & J_RIGHT) {
        PutTile(SCR_1_PLANE, 0, lx, ly, ' ');
        if (angle < ANG_RIGHT) angle = angle + 1;
    }

    /* Gravity: very slow, accelerates gently */
    grav_tick++;
    if (grav_tick >= 30) {
        grav_tick = 0;
        if (fall_speed < 15) fall_speed = fall_speed + 1;
        if (ly < GROUND_ROW - 1) {
            PutTile(SCR_1_PLANE, 0, lx, ly, ' ');
            ly = ly + 1;
        }
    }

    /* Thrust (A): vectored based on angle */
    if ((pad_cur & J_A) && lfuel > 0) {
        thrust_tick++;
        if (thrust_tick >= 6) {
            thrust_tick = 0;
            PutTile(SCR_1_PLANE, 0, lx, ly, ' ');

            /* Vertical component: always push up */
            if (angle == ANG_CENTER) {
                if (ly > TOP_ROW) ly = ly - 1;
            }
            /* Angled: less vertical, more horizontal */
            if (angle == ANG_LEFT || angle == ANG_RIGHT) {
                /* Still some upward push */
                if (grav_tick > 10 && ly > TOP_ROW) ly = ly - 1;
            }

            /* Horizontal component */
            if (angle == ANG_LEFT) {
                drift_dir = 1;
            } else if (angle == ANG_RIGHT) {
                drift_dir = 2;
            }

            if (fall_speed > 0) fall_speed = fall_speed - 1;
            if (lfuel > 0) lfuel = lfuel - 1;
        }
    } else {
        thrust_tick = 0;
    }

    /* Apply horizontal drift */
    if (drift_dir > 0) {
        drift_tick++;
        if (drift_tick >= 8) {
            drift_tick = 0;
            PutTile(SCR_1_PLANE, 0, lx, ly, ' ');
            if (drift_dir == 1 && lx > 0) lx = lx - 1;
            if (drift_dir == 2 && lx < 19) lx = lx + 1;
            /* Drift decays when not thrusting tilted */
            if (!((pad_cur & J_A) && lfuel > 0)) {
                drift_dir = 0;
            }
        }
    }

    /* Landing check */
    if (ly >= GROUND_ROW - 1) {
        ly = GROUND_ROW - 1;
        ldone = 1;
        on_pad = (lx >= PAD_LEFT && lx <= PAD_RIGHT) ? 1 : 0;

        if (on_pad && fall_speed <= 3 && angle == ANG_CENTER) {
            PutTile(SCR_1_PLANE, 0, lx, ly, 'V');
            PrintString(SCR_1_PLANE, 1, 5, 9, "LANDED!");
        } else if (on_pad && angle != ANG_CENTER) {
            PutTile(SCR_1_PLANE, 0, lx, ly, 'X');
            PrintString(SCR_1_PLANE, 0, 4, 9, "NOT LEVEL!");
        } else if (on_pad) {
            PutTile(SCR_1_PLANE, 0, lx, ly, 'X');
            PrintString(SCR_1_PLANE, 0, 5, 9, "TOO FAST!");
        } else {
            PutTile(SCR_1_PLANE, 0, lx, ly, 'X');
            PrintString(SCR_1_PLANE, 0, 4, 9, "MISSED PAD!");
        }
        PrintString(SCR_1_PLANE, 0, 2, 11, "B:RETRY  OPT:MENU");
        return;
    }

    PutTile(SCR_1_PLANE, 0, lx, ly, lander_char());
    draw_hud();
}

void main(void)
{
    InitNGPC();
    SysSetSystemFont();
    SetBackgroundColour(RGB(0, 0, 0));
    SetPalette(SCR_1_PLANE, 0, 0,
        RGB(15,15,15), RGB(15,15,15), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, 1, 0,
        RGB(0,15,0), RGB(0,10,0), RGB(0,15,0));

    state = STATE_MENU;
    skip = 10;
    show_menu();

    while (1) {
        WaitVsync();
        read_input();

        if (skip > 0) {
            skip--;
            continue;
        }

        if (state == STATE_MENU) {
            if (pad_press & J_DOWN) {
                menu_cursor++;
                if (menu_cursor >= 4) menu_cursor = 0;
                show_menu();
            }
            if (pad_press & J_UP) {
                if (menu_cursor == 0) menu_cursor = 3;
                else menu_cursor--;
                show_menu();
            }
            if (pad_press & J_A) {
                state = STATE_LANDER;
                skip = 10;
                show_lander();
            }
        } else if (state == STATE_LANDER) {
            update_lander();
            if (pad_press & J_OPTION) {
                state = STATE_MENU;
                skip = 10;
                show_menu();
            }
        }
    }
}
