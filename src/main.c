/* main.c — ASTEROIDS standalone for Neo Geo Pocket Color
 *
 * 16x16 ship (2x2 tiles), 32x16 alien (4x2 tiles)
 * Packed 2bpp tile format via InstallTileSetAt
 * Flash high score save
 * UFO appears every wave
 */

#define CARTHDR_IMPL
#include "carthdr.h"
#include "ngpc.h"
#include "library.h"

/* ---- Sound effects ---- */
static const SOUNDEFFECT game_sounds[] = {
    { 1, 3, 0, 0x0180, 0x0020, 1, 0, 0x0100, 0x0200, 15, 3, 1, 0, 0, 15 },
    { 2, 4, 0, 0x0040, 0x0010, 2, 0, 0x0020, 0x0100, 12, 2, 1, 0, 0, 15 },
    { 0, 6, 0, 0x0200, 0x0040, 1, 0, 0x0040, 0x0300, 14, 1, 2, 0, 0, 15 },
    { 2, 8, 0, 0x0030, 0x0008, 2, 0, 0x0010, 0x0080, 15, 1, 2, 0, 0, 15 },
    { 0, 10, 1, 0x0100, 0x0010, 3, 1, 0x00C0, 0x0140, 8, 1, 3, 1, 4, 10 },
    { 1, 3, 0, 0x0140, 0x0030, 1, 0, 0x0080, 0x0200, 13, 2, 1, 0, 0, 15 }
};
#define SND_FIRE    1
#define SND_THRUST  2
#define SND_WARP    3
#define SND_EXPLODE 4
#define SND_UFO_HUM 5
#define SND_UFO_FIRE 6
static u8 sound_installed;
static u8 thrust_snd_tick;

/* ---- Ship 16x16: 8 dirs x 4 tiles ---- */
static const u16 ship_n[4][8] = {
    { 0x0020, 0x0098, 0x0098, 0x0098, 0x0064, 0x0264, 0x0246, 0x0189 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x0909, 0x0901, 0x0601, 0x2602, 0x2555, 0x16AA, 0x1000, 0x0000 },
    { 0x0000, 0x8000, 0x8000, 0x4000, 0x4000, 0x6000, 0x9000, 0x0000 },
};
static const u16 ship_ne[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x0002, 0x0025, 0x0058, 0x0960, 0x9600 },
    { 0x0000, 0x0280, 0x2500, 0x5600, 0xA400, 0x1800, 0x9000, 0x6000 },
    { 0x6502, 0x0149, 0x0055, 0x0014, 0x0018, 0x0020, 0x0000, 0x0000 },
    { 0x4000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_e[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x8000, 0x9680, 0x2656, 0x2429, 0x2400 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5A00, 0x9540 },
    { 0x2429, 0x255A, 0x5600, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x5A00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_se[4][8] = {
    { 0x0018, 0x0024, 0x0016, 0x0069, 0x8981, 0x5602, 0x2580, 0x0160 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x4000, 0x6000, 0x9000 },
    { 0x0096, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x1400, 0x6400, 0x9500, 0x0980, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_s[4][8] = {
    { 0x0000, 0x1800, 0x0555, 0x05AA, 0x0602, 0x0902, 0x0981, 0x0189 },
    { 0x0000, 0xA000, 0x6000, 0x6000, 0x4000, 0x4000, 0x8000, 0x8000 },
    { 0x0249, 0x0246, 0x0064, 0x0054, 0x0098, 0x0098, 0x0020, 0x0000 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_sw[4][8] = {
    { 0x0000, 0x0006, 0x0025, 0x0019, 0x0090, 0x0260, 0x0240, 0x0182 },
    { 0x0000, 0x0000, 0x0000, 0x4000, 0x5000, 0x1640, 0x2580, 0x5800 },
    { 0x0509, 0x0696, 0x1560, 0x9600, 0x9800, 0x0000, 0x0000, 0x0000 },
    { 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_w[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0295, 0x95A8, 0x95A0 },
    { 0x0000, 0x0000, 0x0000, 0x0A60, 0x5580, 0xA900, 0x0900, 0x0900 },
    { 0x0296, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x8900, 0x5500, 0x2960, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 ship_nw[4][8] = {
    { 0x0000, 0xA000, 0x9400, 0x1580, 0x2658, 0x0625, 0x0901, 0x0180 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x5000, 0x9600 },
    { 0x0240, 0x0060, 0x009A, 0x0015, 0x0026, 0x0006, 0x0000, 0x0000 },
    { 0x2540, 0x9880, 0x6000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000 },
};

/* ---- Thrust 16x16: 8 dirs x 4 tiles ---- */
static const u16 thrst_n[4][8] = {
    { 0x0020, 0x0098, 0x0098, 0x0098, 0x0064, 0x0264, 0x0246, 0x0189 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x0909, 0x0901, 0x0601, 0x0602, 0x2555, 0x1FFF, 0x1000, 0x0000 },
    { 0x0000, 0x8000, 0x8000, 0x4000, 0x4000, 0x6000, 0xA000, 0x0000 },
};
static const u16 thrst_ne[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x0002, 0x0025, 0x0058, 0x0960, 0x9600 },
    { 0x0000, 0x0280, 0x2500, 0x5600, 0xA400, 0x1802, 0x9003, 0x6002 },
    { 0xFE02, 0x3F49, 0x30D6, 0x3FD4, 0x0318, 0x0020, 0x0000, 0x0000 },
    { 0x4003, 0x8002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_e[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x8000, 0x9680, 0x2656, 0xF429, 0xF400 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5A00, 0x9540 },
    { 0xF429, 0xF55A, 0x5600, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x5A00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_se[4][8] = {
    { 0x0018, 0x0324, 0x3FD6, 0x3E69, 0xBD81, 0x5602, 0x2580, 0x0160 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x4000, 0x6000, 0x9000 },
    { 0x0096, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x1400, 0x6400, 0x9500, 0x0980, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_s[4][8] = {
    { 0x0000, 0x183C, 0x25FF, 0x05AA, 0x0602, 0x0902, 0x0981, 0x0189 },
    { 0x0000, 0xA000, 0x6000, 0x6000, 0x4000, 0x4000, 0x8000, 0x8000 },
    { 0x0249, 0x0246, 0x0064, 0x0054, 0x0098, 0x0098, 0x0020, 0x0000 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_sw[4][8] = {
    { 0x0000, 0x0006, 0x0027, 0x0019, 0x0090, 0x0260, 0x0240, 0x0182 },
    { 0x0000, 0x0000, 0xC000, 0xFC00, 0x7800, 0x1640, 0x2580, 0x5800 },
    { 0x0509, 0x0696, 0x1560, 0x9600, 0x9800, 0x0000, 0x0000, 0x0000 },
    { 0x6000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_w[4][8] = {
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0295, 0x95A8, 0x95A0 },
    { 0x0000, 0x0000, 0x0000, 0x0A60, 0x5580, 0xA900, 0x0BC0, 0x0BC0 },
    { 0x0296, 0x0009, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0x8BC0, 0x5500, 0x2960, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};
static const u16 thrst_nw[4][8] = {
    { 0x0000, 0xA000, 0x9400, 0x9580, 0x2658, 0x0625, 0x0901, 0x0180 },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x5000, 0x9600 },
    { 0x0240, 0x0060, 0x009A, 0x0015, 0x0026, 0x0006, 0x0000, 0x0000 },
    { 0x2540, 0x9C80, 0xFC00, 0xFC00, 0x3000, 0x0000, 0x0000, 0x0000 },
};

/* ---- Alien 32x16: 4x2 tiles ---- */
static const u16 alien_ship[8][8] = {
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0032, 0x001A },
    { 0x0000, 0x00EA, 0x00DA, 0x00EF, 0x0EBF, 0x39AA, 0xAFFF, 0xF000 },
    { 0x0000, 0xC000, 0x4000, 0xB000, 0xE800, 0xAB00, 0xFFAC, 0x00FA },
    { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7000 },
    { 0x0016, 0x0016, 0x003E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
    { 0xAAAA, 0xBFFF, 0xB000, 0xAFFF, 0x3AAA, 0x03FF, 0x0000, 0x0000 },
    { 0xAAA5, 0xFFE9, 0x00EA, 0xFEB0, 0xABC0, 0xFC00, 0x0000, 0x0000 },
    { 0x7000, 0x7000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};


/* ---- Small sprites (8x8) ---- */
static const u16 rock_large[8] = { 0x07E0,0x1FF8,0x3FFC,0x7FFE,0x7FFE,0x3FFC,0x1FF8,0x07E0 };
static const u16 rock_med[8]   = { 0x0000,0x03C0,0x07E0,0x0FF0,0x0FF0,0x07E0,0x03C0,0x0000 };
static const u16 rock_small[8] = { 0x0000,0x0000,0x0180,0x03C0,0x03C0,0x0180,0x0000,0x0000 };
static const u16 bullet_t[8]   = { 0x0000,0x0000,0x0000,0x0180,0x0180,0x0000,0x0000,0x0000 };
static const u16 ufo_shot_t[8] = { 0x0000,0x0000,0x0000,0x0100,0x0100,0x0000,0x0000,0x0000 };
static const u16 star_t[8]     = { 0x0000,0x0000,0x0000,0x0100,0x0000,0x0000,0x0000,0x0000 };

#define MARQUEE_W  20
#define MARQUEE_H  5
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

/* Marquee: 160x40px = 20x5 = 100 tiles */
static const u16 mq_0[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0005, 0x0005 };
static const u16 mq_1[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5500, 0x5500, 0x5544, 0x5550 };
static const u16 mq_2[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0005, 0x0055, 0x0055, 0x0055 };
static const u16 mq_3[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5550, 0x5554, 0x5554, 0x0014 };
static const u16 mq_4[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x1555, 0x1555, 0x1555, 0x1005 };
static const u16 mq_5[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5554, 0x5555, 0x5555, 0x5400 };
static const u16 mq_6[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x1555, 0x1555, 0x0555, 0x0550 };
static const u16 mq_7[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5500, 0x5500, 0x5544, 0x0000 };
static const u16 mq_8[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5555, 0x5555, 0x5555, 0x5541 };
static const u16 mq_9[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x4000, 0x5400, 0x5500, 0x5540 };
static const u16 mq_10[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0055, 0x0555, 0x1555, 0x1550 };
static const u16 mq_11[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5400, 0x5540, 0x5550, 0x1554 };
static const u16 mq_12[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0554, 0x0554, 0x0154, 0x0154 };
static const u16 mq_13[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0555, 0x0555, 0x1155, 0x1155 };
static const u16 mq_14[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x5400, 0x5550, 0x5555, 0x1555 };
static const u16 mq_15[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0005, 0x0015, 0x0055, 0x4055 };
static const u16 mq_16[8] = { 0x0000, 0x0000, 0x0000, 0x0010, 0x5554, 0x5550, 0x5554, 0x4010 };
static const u16 mq_17[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0050, 0x0154, 0x4554, 0x1555 };
static const u16 mq_20[8] = { 0x0005, 0x0005, 0x0005, 0x0005, 0x0015, 0x0015, 0x0015, 0x0015 };
static const u16 mq_21[8] = { 0x5550, 0x5154, 0x5154, 0x5055, 0x5055, 0x4055, 0x5555, 0x5555 };
static const u16 mq_22[8] = { 0x0055, 0x4055, 0x0015, 0x1015, 0x0001, 0x4400, 0x5000, 0x5000 };
static const u16 mq_23[8] = { 0x0554, 0x4400, 0x5540, 0x5554, 0x5555, 0x4555, 0x0515, 0x0015 };
static const u16 mq_24[8] = { 0x1055, 0x5001, 0x0001, 0x0000, 0x4000, 0x4000, 0x5100, 0x5000 };
static const u16 mq_25[8] = { 0x5415, 0x5410, 0x5510, 0x5500, 0x5504, 0x5504, 0x5504, 0x5544 };
static const u16 mq_26[8] = { 0x5550, 0x0550, 0x0555, 0x0155, 0x0155, 0x0154, 0x0154, 0x0155 };
static const u16 mq_27[8] = { 0x5555, 0x4000, 0x5540, 0x5540, 0x5541, 0x0001, 0x1555, 0x1000 };
static const u16 mq_28[8] = { 0x1541, 0x1541, 0x1541, 0x1555, 0x1555, 0x0555, 0x0550, 0x0550 };
static const u16 mq_29[8] = { 0x5540, 0x1541, 0x5541, 0x5501, 0x5405, 0x5554, 0x5540, 0x5551 };
static const u16 mq_30[8] = { 0x1540, 0x5541, 0x5541, 0x1541, 0x1541, 0x1551, 0x1550, 0x0554 };
static const u16 mq_31[8] = { 0x5555, 0x4055, 0x0055, 0x0055, 0x0015, 0x0015, 0x0015, 0x4055 };
static const u16 mq_32[8] = { 0x0154, 0x0155, 0x4455, 0x4055, 0x4155, 0x4155, 0x4155, 0x4115 };
static const u16 mq_33[8] = { 0x1154, 0x1155, 0x0055, 0x0455, 0x0455, 0x0455, 0x4455, 0x4015 };
static const u16 mq_34[8] = { 0x0455, 0x1455, 0x0005, 0x0405, 0x0405, 0x0405, 0x4405, 0x4005 };
static const u16 mq_35[8] = { 0x4055, 0x5115, 0x5015, 0x5445, 0x5401, 0x5410, 0x5410, 0x5410 };
static const u16 mq_36[8] = { 0x0154, 0x4555, 0x5555, 0x5555, 0x5555, 0x0555, 0x1515, 0x1555 };
static const u16 mq_37[8] = { 0x5555, 0x5555, 0x5555, 0x5555, 0x0554, 0x0550, 0x0154, 0x0555 };
static const u16 mq_38[8] = { 0x4000, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_40[8] = { 0x0015, 0x0015, 0x0015, 0x0015, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_41[8] = { 0x5555, 0x4001, 0x4155, 0x0100, 0x0500, 0x5400, 0x0000, 0x0000 };
static const u16 mq_42[8] = { 0x5445, 0x5405, 0x5515, 0x5501, 0x0004, 0x0154, 0x0000, 0x0000 };
static const u16 mq_43[8] = { 0x0015, 0x5555, 0x5555, 0x5555, 0x0000, 0x0555, 0x0000, 0x0000 };
static const u16 mq_44[8] = { 0x5040, 0x5040, 0x4040, 0x0140, 0x0500, 0x5405, 0x0555, 0x1554 };
static const u16 mq_45[8] = { 0x1540, 0x1541, 0x1541, 0x1541, 0x0001, 0x0055, 0x0000, 0x0000 };
static const u16 mq_46[8] = { 0x0055, 0x0055, 0x0055, 0x0055, 0x0000, 0x0001, 0x0000, 0x0000 };
static const u16 mq_47[8] = { 0x0000, 0x5554, 0x5554, 0x5554, 0x0000, 0x5555, 0x0000, 0x0000 };
static const u16 mq_48[8] = { 0x0550, 0x0554, 0x0154, 0x1154, 0x1000, 0x4005, 0x0000, 0x0000 };
static const u16 mq_49[8] = { 0x4554, 0x4155, 0x0055, 0x1055, 0x1000, 0x5001, 0x0000, 0x0000 };
static const u16 mq_50[8] = { 0x0155, 0x0155, 0x0055, 0x4405, 0x0001, 0x5500, 0x0000, 0x0000 };
static const u16 mq_51[8] = { 0x0055, 0x5555, 0x5555, 0x5550, 0x0000, 0x1555, 0x0000, 0x0000 };
static const u16 mq_52[8] = { 0x4115, 0x0115, 0x0515, 0x0415, 0x5400, 0x4000, 0x0000, 0x0000 };
static const u16 mq_53[8] = { 0x4115, 0x4115, 0x4115, 0x5115, 0x0000, 0x5540, 0x0000, 0x0000 };
static const u16 mq_54[8] = { 0x4155, 0x5555, 0x5555, 0x5554, 0x0000, 0x5555, 0x0000, 0x0000 };
static const u16 mq_55[8] = { 0x5015, 0x5055, 0x4045, 0x0140, 0x1500, 0x5000, 0x0000, 0x0000 };
static const u16 mq_56[8] = { 0x5555, 0x5555, 0x5555, 0x5555, 0x1000, 0x0155, 0x0000, 0x0000 };
static const u16 mq_57[8] = { 0x4555, 0x4155, 0x4150, 0x4140, 0x0105, 0x5501, 0x0005, 0x0001 };
static const u16 mq_58[8] = { 0x4000, 0x0545, 0x0155, 0x0055, 0x4551, 0x5515, 0x1154, 0x4515 };
static const u16 mq_59[8] = { 0x0000, 0x0000, 0x4400, 0x5500, 0x4440, 0x5510, 0x4154, 0x5544 };
static const u16 mq_61[8] = { 0x0000, 0x5555, 0x0000, 0x0055, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_62[8] = { 0x0000, 0x5555, 0x0000, 0x4555, 0x0155, 0x0555, 0x1555, 0x0005 };
static const u16 mq_63[8] = { 0x0005, 0x5555, 0x1554, 0x5500, 0x5000, 0x0001, 0x0005, 0x4015 };
static const u16 mq_64[8] = { 0x5550, 0x5555, 0x1540, 0x5555, 0x5400, 0x5000, 0x4000, 0x0000 };
static const u16 mq_65[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_66[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_67[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_68[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_69[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_70[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_71[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_72[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_73[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_74[8] = { 0x0000, 0x5555, 0x0000, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_75[8] = { 0x0000, 0x0155, 0x0145, 0x5545, 0x0145, 0x0145, 0x0141, 0x0000 };
static const u16 mq_76[8] = { 0x0000, 0x0554, 0x1514, 0x5554, 0x5500, 0x4554, 0x0150, 0x0000 };
static const u16 mq_77[8] = { 0x0001, 0x1551, 0x5450, 0x5454, 0x5454, 0x1550, 0x0540, 0x0000 };
static const u16 mq_78[8] = { 0x5151, 0x4555, 0x1455, 0x1455, 0x0555, 0x0515, 0x0155, 0x1145 };
static const u16 mq_79[8] = { 0x4554, 0x5554, 0x4500, 0x5100, 0x5540, 0x1450, 0x5544, 0x5554 };
static const u16 mq_82[8] = { 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_83[8] = { 0x5054, 0x5450, 0x5550, 0x1540, 0x1500, 0x1400, 0x1000, 0x0000 };
static const u16 mq_98[8] = { 0x0545, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
static const u16 mq_99[8] = { 0x1450, 0x5540, 0x0400, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

static void install_marquee(void) {
    InstallTileSetAt((const unsigned short (*)[8])mq_0, 8, T_MARQUEE + 0);
    InstallTileSetAt((const unsigned short (*)[8])mq_1, 8, T_MARQUEE + 1);
    InstallTileSetAt((const unsigned short (*)[8])mq_2, 8, T_MARQUEE + 2);
    InstallTileSetAt((const unsigned short (*)[8])mq_3, 8, T_MARQUEE + 3);
    InstallTileSetAt((const unsigned short (*)[8])mq_4, 8, T_MARQUEE + 4);
    InstallTileSetAt((const unsigned short (*)[8])mq_5, 8, T_MARQUEE + 5);
    InstallTileSetAt((const unsigned short (*)[8])mq_6, 8, T_MARQUEE + 6);
    InstallTileSetAt((const unsigned short (*)[8])mq_7, 8, T_MARQUEE + 7);
    InstallTileSetAt((const unsigned short (*)[8])mq_8, 8, T_MARQUEE + 8);
    InstallTileSetAt((const unsigned short (*)[8])mq_9, 8, T_MARQUEE + 9);
    InstallTileSetAt((const unsigned short (*)[8])mq_10, 8, T_MARQUEE + 10);
    InstallTileSetAt((const unsigned short (*)[8])mq_11, 8, T_MARQUEE + 11);
    InstallTileSetAt((const unsigned short (*)[8])mq_12, 8, T_MARQUEE + 12);
    InstallTileSetAt((const unsigned short (*)[8])mq_13, 8, T_MARQUEE + 13);
    InstallTileSetAt((const unsigned short (*)[8])mq_14, 8, T_MARQUEE + 14);
    InstallTileSetAt((const unsigned short (*)[8])mq_15, 8, T_MARQUEE + 15);
    InstallTileSetAt((const unsigned short (*)[8])mq_16, 8, T_MARQUEE + 16);
    InstallTileSetAt((const unsigned short (*)[8])mq_17, 8, T_MARQUEE + 17);
    InstallTileSetAt((const unsigned short (*)[8])mq_20, 8, T_MARQUEE + 20);
    InstallTileSetAt((const unsigned short (*)[8])mq_21, 8, T_MARQUEE + 21);
    InstallTileSetAt((const unsigned short (*)[8])mq_22, 8, T_MARQUEE + 22);
    InstallTileSetAt((const unsigned short (*)[8])mq_23, 8, T_MARQUEE + 23);
    InstallTileSetAt((const unsigned short (*)[8])mq_24, 8, T_MARQUEE + 24);
    InstallTileSetAt((const unsigned short (*)[8])mq_25, 8, T_MARQUEE + 25);
    InstallTileSetAt((const unsigned short (*)[8])mq_26, 8, T_MARQUEE + 26);
    InstallTileSetAt((const unsigned short (*)[8])mq_27, 8, T_MARQUEE + 27);
    InstallTileSetAt((const unsigned short (*)[8])mq_28, 8, T_MARQUEE + 28);
    InstallTileSetAt((const unsigned short (*)[8])mq_29, 8, T_MARQUEE + 29);
    InstallTileSetAt((const unsigned short (*)[8])mq_30, 8, T_MARQUEE + 30);
    InstallTileSetAt((const unsigned short (*)[8])mq_31, 8, T_MARQUEE + 31);
    InstallTileSetAt((const unsigned short (*)[8])mq_32, 8, T_MARQUEE + 32);
    InstallTileSetAt((const unsigned short (*)[8])mq_33, 8, T_MARQUEE + 33);
    InstallTileSetAt((const unsigned short (*)[8])mq_34, 8, T_MARQUEE + 34);
    InstallTileSetAt((const unsigned short (*)[8])mq_35, 8, T_MARQUEE + 35);
    InstallTileSetAt((const unsigned short (*)[8])mq_36, 8, T_MARQUEE + 36);
    InstallTileSetAt((const unsigned short (*)[8])mq_37, 8, T_MARQUEE + 37);
    InstallTileSetAt((const unsigned short (*)[8])mq_38, 8, T_MARQUEE + 38);
    InstallTileSetAt((const unsigned short (*)[8])mq_40, 8, T_MARQUEE + 40);
    InstallTileSetAt((const unsigned short (*)[8])mq_41, 8, T_MARQUEE + 41);
    InstallTileSetAt((const unsigned short (*)[8])mq_42, 8, T_MARQUEE + 42);
    InstallTileSetAt((const unsigned short (*)[8])mq_43, 8, T_MARQUEE + 43);
    InstallTileSetAt((const unsigned short (*)[8])mq_44, 8, T_MARQUEE + 44);
    InstallTileSetAt((const unsigned short (*)[8])mq_45, 8, T_MARQUEE + 45);
    InstallTileSetAt((const unsigned short (*)[8])mq_46, 8, T_MARQUEE + 46);
    InstallTileSetAt((const unsigned short (*)[8])mq_47, 8, T_MARQUEE + 47);
    InstallTileSetAt((const unsigned short (*)[8])mq_48, 8, T_MARQUEE + 48);
    InstallTileSetAt((const unsigned short (*)[8])mq_49, 8, T_MARQUEE + 49);
    InstallTileSetAt((const unsigned short (*)[8])mq_50, 8, T_MARQUEE + 50);
    InstallTileSetAt((const unsigned short (*)[8])mq_51, 8, T_MARQUEE + 51);
    InstallTileSetAt((const unsigned short (*)[8])mq_52, 8, T_MARQUEE + 52);
    InstallTileSetAt((const unsigned short (*)[8])mq_53, 8, T_MARQUEE + 53);
    InstallTileSetAt((const unsigned short (*)[8])mq_54, 8, T_MARQUEE + 54);
    InstallTileSetAt((const unsigned short (*)[8])mq_55, 8, T_MARQUEE + 55);
    InstallTileSetAt((const unsigned short (*)[8])mq_56, 8, T_MARQUEE + 56);
    InstallTileSetAt((const unsigned short (*)[8])mq_57, 8, T_MARQUEE + 57);
    InstallTileSetAt((const unsigned short (*)[8])mq_58, 8, T_MARQUEE + 58);
    InstallTileSetAt((const unsigned short (*)[8])mq_59, 8, T_MARQUEE + 59);
    InstallTileSetAt((const unsigned short (*)[8])mq_61, 8, T_MARQUEE + 61);
    InstallTileSetAt((const unsigned short (*)[8])mq_62, 8, T_MARQUEE + 62);
    InstallTileSetAt((const unsigned short (*)[8])mq_63, 8, T_MARQUEE + 63);
    InstallTileSetAt((const unsigned short (*)[8])mq_64, 8, T_MARQUEE + 64);
    InstallTileSetAt((const unsigned short (*)[8])mq_65, 8, T_MARQUEE + 65);
    InstallTileSetAt((const unsigned short (*)[8])mq_66, 8, T_MARQUEE + 66);
    InstallTileSetAt((const unsigned short (*)[8])mq_67, 8, T_MARQUEE + 67);
    InstallTileSetAt((const unsigned short (*)[8])mq_68, 8, T_MARQUEE + 68);
    InstallTileSetAt((const unsigned short (*)[8])mq_69, 8, T_MARQUEE + 69);
    InstallTileSetAt((const unsigned short (*)[8])mq_70, 8, T_MARQUEE + 70);
    InstallTileSetAt((const unsigned short (*)[8])mq_71, 8, T_MARQUEE + 71);
    InstallTileSetAt((const unsigned short (*)[8])mq_72, 8, T_MARQUEE + 72);
    InstallTileSetAt((const unsigned short (*)[8])mq_73, 8, T_MARQUEE + 73);
    InstallTileSetAt((const unsigned short (*)[8])mq_74, 8, T_MARQUEE + 74);
    InstallTileSetAt((const unsigned short (*)[8])mq_75, 8, T_MARQUEE + 75);
    InstallTileSetAt((const unsigned short (*)[8])mq_76, 8, T_MARQUEE + 76);
    InstallTileSetAt((const unsigned short (*)[8])mq_77, 8, T_MARQUEE + 77);
    InstallTileSetAt((const unsigned short (*)[8])mq_78, 8, T_MARQUEE + 78);
    InstallTileSetAt((const unsigned short (*)[8])mq_79, 8, T_MARQUEE + 79);
    InstallTileSetAt((const unsigned short (*)[8])mq_82, 8, T_MARQUEE + 82);
    InstallTileSetAt((const unsigned short (*)[8])mq_83, 8, T_MARQUEE + 83);
    InstallTileSetAt((const unsigned short (*)[8])mq_98, 8, T_MARQUEE + 98);
    InstallTileSetAt((const unsigned short (*)[8])mq_99, 8, T_MARQUEE + 99);
}

static void install_tiles(void) {
    u8 i;
    /* Ship normal: 8 dirs x 4 tiles */
    InstallTileSetAt((const unsigned short (*)[8])ship_n,  32, T_SHIP);
    InstallTileSetAt((const unsigned short (*)[8])ship_ne, 32, T_SHIP+4);
    InstallTileSetAt((const unsigned short (*)[8])ship_e,  32, T_SHIP+8);
    InstallTileSetAt((const unsigned short (*)[8])ship_se, 32, T_SHIP+12);
    InstallTileSetAt((const unsigned short (*)[8])ship_s,  32, T_SHIP+16);
    InstallTileSetAt((const unsigned short (*)[8])ship_sw, 32, T_SHIP+20);
    InstallTileSetAt((const unsigned short (*)[8])ship_w,  32, T_SHIP+24);
    InstallTileSetAt((const unsigned short (*)[8])ship_nw, 32, T_SHIP+28);
    /* Ship thrust */
    InstallTileSetAt((const unsigned short (*)[8])thrst_n,  32, T_THRST);
    InstallTileSetAt((const unsigned short (*)[8])thrst_ne, 32, T_THRST+4);
    InstallTileSetAt((const unsigned short (*)[8])thrst_e,  32, T_THRST+8);
    InstallTileSetAt((const unsigned short (*)[8])thrst_se, 32, T_THRST+12);
    InstallTileSetAt((const unsigned short (*)[8])thrst_s,  32, T_THRST+16);
    InstallTileSetAt((const unsigned short (*)[8])thrst_sw, 32, T_THRST+20);
    InstallTileSetAt((const unsigned short (*)[8])thrst_w,  32, T_THRST+24);
    InstallTileSetAt((const unsigned short (*)[8])thrst_nw, 32, T_THRST+28);
    /* Alien */
    InstallTileSetAt((const unsigned short (*)[8])alien_ship, 64, T_ALIEN);
    /* Small sprites */
    InstallTileSetAt((const unsigned short (*)[8])rock_large, 8, T_ROCK_L);
    InstallTileSetAt((const unsigned short (*)[8])rock_med,   8, T_ROCK_M);
    InstallTileSetAt((const unsigned short (*)[8])rock_small, 8, T_ROCK_S);
    InstallTileSetAt((const unsigned short (*)[8])bullet_t,   8, T_BULLET);
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
#define PAL_MARQUEE 7

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

/* ---- Flash save structure ---- */
#define SAVE_MAGIC 0xA5A5
static u16 save_buf[8];

static void load_high_scores(void) {
    GetSavedData((void*)save_buf);
    if (save_buf[0] == SAVE_MAGIC) {
        high_scores[0] = save_buf[1];
        high_scores[1] = save_buf[2];
        high_scores[2] = save_buf[3];
        high_scores[3] = save_buf[4];
        high_scores[4] = save_buf[5];
    } else {
        high_scores[0] = 1000;
        high_scores[1] = 800;
        high_scores[2] = 600;
        high_scores[3] = 400;
        high_scores[4] = 200;
    }
}

static void save_high_scores(void) {
    save_buf[0] = SAVE_MAGIC;
    save_buf[1] = high_scores[0];
    save_buf[2] = high_scores[1];
    save_buf[3] = high_scores[2];
    save_buf[4] = high_scores[3];
    save_buf[5] = high_scores[4];
    save_buf[6] = 0;
    save_buf[7] = 0;
    Flash((void*)save_buf);
}

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
    for (i = 1; i < MAX_ENTS; i++) if (ent_type[i] == ENT_NONE) return i;
    return 255;
}

static void erase_ent(u8 i) {
    u8 tx, ty;
    tx = ent_otx[i]; ty = ent_oty[i];
    if (tx >= 20 || ty >= 19) return;
    if (ent_type[i] == ENT_SHIP) {
        /* 2x2 */
        PutTile(SCR_1_PLANE, 0, tx, ty, ' ');
        if (tx+1 < 20) PutTile(SCR_1_PLANE, 0, tx+1, ty, ' ');
        if (ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx, ty+1, ' ');
        if (tx+1 < 20 && ty+1 < 19) PutTile(SCR_1_PLANE, 0, tx+1, ty+1, ' ');
    } else if (ent_type[i] == ENT_UFO) {
        /* 4x2 */
        u8 c, r;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                if (tx+c < 20 && ty+r < 19)
                    PutTile(SCR_1_PLANE, 0, tx+c, ty+r, ' ');
    } else {
        PutTile(SCR_1_PLANE, 0, tx, ty, ' ');
    }
}

static void draw_ent(u8 i) {
    u8 tx, ty;
    u16 base;
    tx = ent_px[i] >> 3;
    ty = ent_py[i] >> 3;
    if (tx >= 20) tx = 19;
    if (ty >= 19) ty = 18;

    if (ent_type[i] == ENT_SHIP) {
        if (tx >= 19) tx = 18;
        if (ty >= 18) ty = 17;
        if (thrusting) base = T_THRST + (u16)ent_dir[i] * 4;
        else base = T_SHIP + (u16)ent_dir[i] * 4;
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, base);
        PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty, base+1);
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty+1, base+2);
        PutTile(SCR_1_PLANE, ent_pal[i], tx+1, ty+1, base+3);
    } else if (ent_type[i] == ENT_UFO) {
        if (tx >= 17) tx = 16;
        if (ty >= 18) ty = 17;
        base = T_ALIEN;
        {
            u8 c, r;
            for (r = 0; r < 2; r++)
                for (c = 0; c < 4; c++)
                    PutTile(SCR_1_PLANE, ent_pal[i], tx+c, ty+r, base + r*4 + c);
        }
    } else if (ent_type[i] == ENT_ROCK_L) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_L);
    } else if (ent_type[i] == ENT_ROCK_M) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_M);
    } else if (ent_type[i] == ENT_ROCK_S) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_ROCK_S);
    } else if (ent_type[i] == ENT_BULLET) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_BULLET);
    } else if (ent_type[i] == ENT_USHOT) {
        PutTile(SCR_1_PLANE, ent_pal[i], tx, ty, T_USHOT);
    }
    ent_otx[i] = tx;
    ent_oty[i] = ty;
}

static void move_ent(u8 i) {
    u8 d;
    ent_tick[i] = ent_tick[i] + 1;
    if (ent_tick[i] < ent_spd[i]) return;
    ent_tick[i] = 0;
    d = ent_dir[i];
    if (dx_add[d]) { ent_px[i] = ent_px[i]+1; if (ent_px[i] >= 160) ent_px[i] = 0; }
    if (dx_sub[d]) { if (ent_px[i] == 0) ent_px[i] = 159; else ent_px[i] = ent_px[i]-1; }
    if (dy_add[d]) { ent_py[i] = ent_py[i]+1; if (ent_py[i] >= 152) ent_py[i] = 8; }
    if (dy_sub[d]) { if (ent_py[i] <= 8) ent_py[i] = 151; else ent_py[i] = ent_py[i]-1; }
}

/* ---- Rocks ---- */
static void spawn_rock(u8 type, u8 px, u8 py) {
    u8 idx;
    idx = find_free();
    if (idx == 255) return;
    ent_type[idx] = type; ent_px[idx] = px; ent_py[idx] = py;
    ent_dir[idx] = cheap_rand(8); ent_tick[idx] = 0;
    ent_otx[idx] = 255; ent_oty[idx] = 255; ent_life[idx] = 0;
    if (type == ENT_ROCK_L) { ent_spd[idx] = 6; ent_pal[idx] = PAL_ROCK1; }
    else if (type == ENT_ROCK_M) { ent_spd[idx] = 4; ent_pal[idx] = PAL_ROCK2; }
    else { ent_spd[idx] = 3; ent_pal[idx] = PAL_ROCK3; }
}

static void spawn_wave(void) {
    u8 count, i;
    count = 3 + wave; if (count > 8) count = 8;
    for (i = 0; i < count; i++) {
        if (cheap_rand(2) == 0)
            spawn_rock(ENT_ROCK_L, cheap_rand(160), cheap_rand(2)==0 ? 12 : 140);
        else
            spawn_rock(ENT_ROCK_L, cheap_rand(2)==0 ? 4 : 152, 12+cheap_rand(128));
    }
}

static void destroy_rock(u8 idx) {
    u8 t, px, py;
    t = ent_type[idx]; px = ent_px[idx]; py = ent_py[idx];
    if (t == ENT_ROCK_L) score = score + 20;
    else if (t == ENT_ROCK_M) score = score + 50;
    else score = score + 100;
    erase_ent(idx); ent_type[idx] = ENT_NONE;
    if (t == ENT_ROCK_L) { spawn_rock(ENT_ROCK_M, px+4, py); spawn_rock(ENT_ROCK_M, px, py+4); }
    else if (t == ENT_ROCK_M) { spawn_rock(ENT_ROCK_S, px+2, py); spawn_rock(ENT_ROCK_S, px, py+2); }
}

static u8 count_rocks(void) {
    u8 i, c; c = 0;
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

/* ---- UFO — appears every wave ---- */
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
    else { ent_px[idx] = 128; ent_dir[idx] = 6; }
    ent_py[idx] = 20 + cheap_rand(100);
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

static void update_ufo(void) {
    if (!ufo_active) {
        /* UFO spawns every wave — shorter timer */
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

/* ---- Forward decl ---- */
static void insert_high_score(u16 s);
static void draw_title(void);

static void do_game_over(void) {
    game_over = 1; state = STATE_OVER;
    insert_high_score(score);
    save_high_scores();
    PlaySound(SND_EXPLODE);
    PrintString(SCR_1_PLANE, PAL_SHIP, 4, 9, "GAME OVER!");
    PrintString(SCR_1_PLANE, PAL_TEXT, 2, 11, "A:RETRY OPT:SCORES");
    skip = 30;
}

static void warp_ship(void) {
    if (warp_cooldown > 0) return;
    erase_ent(0);
    ent_px[0] = 16+cheap_rand(128); ent_py[0] = 16+cheap_rand(112);
    ship_vx_add = 0; ship_vx_sub = 0; ship_vy_add = 0; ship_vy_sub = 0;
    drift_timer = 0; ent_otx[0] = 255; ent_oty[0] = 255;
    warp_cooldown = WARP_COOLDOWN;
    PlaySound(SND_WARP);
    if (cheap_rand(10)==0) {
        ent_type[0] = ENT_NONE; alive = 0;
        if (lives > 0) lives = lives - 1;
        spawn_timer = 0;
        if (lives == 0) do_game_over();
    }
}

/* ---- Collision: tile-overlap for different sized entities ---- */
static u8 check_hit(u8 a, u8 b) {
    u8 ax, ay, bx, by, aw, ah, bw, bh;
    ax = ent_px[a] >> 3; ay = ent_py[a] >> 3;
    bx = ent_px[b] >> 3; by = ent_py[b] >> 3;
    /* Sizes in tiles */
    if (ent_type[a] == ENT_SHIP) { aw = 2; ah = 2; }
    else if (ent_type[a] == ENT_UFO) { aw = 4; ah = 2; }
    else { aw = 1; ah = 1; }
    if (ent_type[b] == ENT_SHIP) { bw = 2; bh = 2; }
    else if (ent_type[b] == ENT_UFO) { bw = 4; bh = 2; }
    else { bw = 1; bh = 1; }
    if (ax+aw <= bx || bx+bw <= ax) return 0;
    if (ay+ah <= by || by+bh <= ay) return 0;
    return 1;
}

/* ---- HUD ---- */
static void put_score_at(u8 x, u8 y, u16 val) {
    u16 v; v = val;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+4, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+3, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+2, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x+1, y, '0'+v%10); v=v/10;
    PutTile(SCR_1_PLANE, PAL_TEXT, x,   y, '0'+v%10);
}

static void draw_hud(void) {
    put_score_at(0, 0, score);
    PrintString(SCR_1_PLANE, PAL_DIM, 7, 0, "LIVES:");
    PutTile(SCR_1_PLANE, PAL_TEXT, 13, 0, '0'+lives);
    PutTile(SCR_1_PLANE, PAL_DIM, 15, 0, 'W');
    PutTile(SCR_1_PLANE, PAL_TEXT, 16, 0, '0'+wave/10);
    PutTile(SCR_1_PLANE, PAL_TEXT, 17, 0, '0'+wave%10);
    if (warp_cooldown > 0) PutTile(SCR_1_PLANE, PAL_ROCK3, 19, 0, 'B');
    else PutTile(SCR_1_PLANE, PAL_SHIP, 19, 0, 'B');
}

/* ---- Stars ---- */
static void init_stars(void) {
    u8 i;
    for (i = 0; i < 16; i++) { star_x[i] = cheap_rand(20); star_y[i] = 1+cheap_rand(17); }
}
static void draw_stars(void) {
    u8 i;
    for (i = 0; i < 16; i++) PutTile(SCR_1_PLANE, PAL_DIM, star_x[i], star_y[i], T_STAR);
}

/* ---- Palettes ---- */
static void setup_palettes(void) {
    SetBackgroundColour(RGB(0, 0, 0));
    SetPalette(SCR_1_PLANE, PAL_TEXT, 0, RGB(15,15,15), RGB(15,15,15), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_SHIP, 0, RGB(10,12,15), RGB(7,9,13), RGB(15,10,2));
    SetPalette(SCR_1_PLANE, PAL_ROCK1, 0, RGB(15,15,15), RGB(13,13,13), RGB(15,15,15));
    SetPalette(SCR_1_PLANE, PAL_ROCK2, 0, RGB(12,12,12), RGB(10,10,10), RGB(14,14,14));
    SetPalette(SCR_1_PLANE, PAL_ROCK3, 0, RGB(9,9,9), RGB(7,7,7), RGB(11,11,11));
    SetPalette(SCR_1_PLANE, PAL_DIM, 0, RGB(4,4,4), RGB(3,3,3), RGB(5,5,5));
    SetPalette(SCR_1_PLANE, PAL_UFO, 0, RGB(6,15,6), RGB(4,12,4), RGB(8,15,8));
    SetPalette(SCR_1_PLANE, PAL_MARQUEE, 0, RGB(15,13,8), RGB(12,10,6), RGB(15,15,12));
}

/* ---- Title ---- */
static void draw_title(void) {
    u8 tx, ty;
    ClearScreen(SCR_1_PLANE);
    setup_palettes(); SysSetSystemFont(); install_tiles();

    /* Marquee */
    for (ty = 0; ty < MARQUEE_H; ty++)
        for (tx = 0; tx < MARQUEE_W; tx++)
            PutTile(SCR_1_PLANE, PAL_MARQUEE, tx, ty+1, T_MARQUEE + ty*MARQUEE_W + tx);
    install_marquee();

    /* Ship: 20% left, pointing NW with thrust */
    {
        u16 base;
        base = T_THRST + 7 * 4;  /* NW thrust */
        PutTile(SCR_1_PLANE, PAL_SHIP, 5, 8, base);
        PutTile(SCR_1_PLANE, PAL_SHIP, 6, 8, base+1);
        PutTile(SCR_1_PLANE, PAL_SHIP, 5, 9, base+2);
        PutTile(SCR_1_PLANE, PAL_SHIP, 6, 9, base+3);
    }
    PutTile(SCR_1_PLANE, PAL_SHIP, 4, 7, T_BULLET);

    /* Alien */
    {
        u8 c, r;
        for (r = 0; r < 2; r++)
            for (c = 0; c < 4; c++)
                PutTile(SCR_1_PLANE, PAL_UFO, 14+c, 8+r, T_ALIEN + r*4 + c);
    }

    /* Rocks + stars */
    PutTile(SCR_1_PLANE, PAL_ROCK1, 3, 10, T_ROCK_L);
    PutTile(SCR_1_PLANE, PAL_ROCK2, 11, 11, T_ROCK_M);
    PutTile(SCR_1_PLANE, PAL_ROCK3, 8, 12, T_ROCK_S);
    PutTile(SCR_1_PLANE, PAL_DIM, 1, 7, T_STAR);
    PutTile(SCR_1_PLANE, PAL_DIM, 19, 10, T_STAR);
    PutTile(SCR_1_PLANE, PAL_DIM, 10, 13, T_STAR);

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
    setup_palettes(); SysSetSystemFont();
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
    setup_palettes(); SysSetSystemFont(); install_tiles();
    for (i = 0; i < MAX_ENTS; i++) { ent_type[i] = ENT_NONE; ent_otx[i] = 255; ent_oty[i] = 255; ent_pal[i] = 0; }
    ent_type[0] = ENT_SHIP; ent_px[0] = 76; ent_py[0] = 72;
    ent_dir[0] = 0; ent_spd[0] = 3; ent_tick[0] = 0;
    ent_pal[0] = PAL_SHIP; ent_otx[0] = 255; ent_oty[0] = 255;
    ship_dir = 0; ship_vx_add = 0; ship_vx_sub = 0;
    ship_vy_add = 0; ship_vy_sub = 0; drift_timer = 0;
    thrusting = 0; rot_tick = 0; fire_tick = 0;
    score = 0; lives = 3; wave = 1; spawn_timer = 0;
    game_over = 0; alive = 1; warp_cooldown = 0;
    ufo_active = 0; ufo_timer = 100; ufo_fire_tmr = UFO_FIRE_RATE;
    thrust_snd_tick = 0;
    init_stars(); draw_stars(); spawn_wave(); draw_hud();
}

/* ---- Game update ---- */
static void game_update(void) {
    u8 i, j, bcount;
    if (game_over) return;
    if (warp_cooldown > 0) warp_cooldown--;
    if (pad_press & J_OPTION) { state = STATE_TITLE; skip = 10; draw_title(); return; }
    update_ufo();

    if (!alive) {
        spawn_timer++;
        if (spawn_timer > 60 && lives > 0) {
            spawn_timer = 0;
            ent_type[0] = ENT_SHIP; ent_px[0] = 76; ent_py[0] = 72;
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
            if (pad_cur & J_LEFT) { erase_ent(0); if (ship_dir==0) ship_dir=7; else ship_dir=ship_dir-1; ent_dir[0]=ship_dir; }
            if (pad_cur & J_RIGHT) { erase_ent(0); ship_dir=ship_dir+1; if (ship_dir>7) ship_dir=0; ent_dir[0]=ship_dir; }
        }
        thrusting = 0;
        if (pad_cur & J_UP) {
            thrusting = 1;
            ship_vx_add = dx_add[ship_dir]; ship_vx_sub = dx_sub[ship_dir];
            ship_vy_add = dy_add[ship_dir]; ship_vy_sub = dy_sub[ship_dir];
            thrust_snd_tick++;
            if (thrust_snd_tick >= 15) { thrust_snd_tick = 0; PlaySound(SND_THRUST); }
        } else { thrust_snd_tick = 0; }
        drift_timer++;
        if (drift_timer >= 3) {
            drift_timer = 0; erase_ent(0);
            if (ship_vx_add) { ent_px[0]=ent_px[0]+1; if (ent_px[0]>=160) ent_px[0]=0; }
            if (ship_vx_sub) { if (ent_px[0]==0) ent_px[0]=159; else ent_px[0]=ent_px[0]-1; }
            if (ship_vy_add) { ent_py[0]=ent_py[0]+1; if (ent_py[0]>=152) ent_py[0]=8; }
            if (ship_vy_sub) { if (ent_py[0]<=8) ent_py[0]=151; else ent_py[0]=ent_py[0]-1; }
        }
        fire_tick++;
        if ((pad_press & J_A) && fire_tick >= 6) {
            bcount = 0;
            for (i = 0; i < MAX_ENTS; i++) if (ent_type[i]==ENT_BULLET) bcount++;
            if (bcount < 4) { fire_bullet(); fire_tick = 0; PlaySound(SND_FIRE); }
        }
        if ((pad_press & J_B) && alive) warp_ship();
    }

    for (i = 1; i < MAX_ENTS; i++) {
        if (ent_type[i]==ENT_NONE) continue;
        erase_ent(i); move_ent(i);
        if (ent_type[i]==ENT_BULLET || ent_type[i]==ENT_USHOT) {
            if (ent_life[i] > 0) ent_life[i] = ent_life[i]-1;
            if (ent_life[i]==0) { erase_ent(i); ent_type[i]=ENT_NONE; continue; }
        }
    }

    /* Bullets vs rocks */
    for (i = 0; i < MAX_ENTS; i++) {
        if (ent_type[i]!=ENT_BULLET) continue;
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j]<ENT_ROCK_L || ent_type[j]>ENT_ROCK_S) continue;
            if (check_hit(i,j)) { erase_ent(i); ent_type[i]=ENT_NONE; destroy_rock(j); break; }
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
    /* Ship vs rocks */
    if (alive && ent_type[0]==ENT_SHIP) {
        for (j = 1; j < MAX_ENTS; j++) {
            if (ent_type[j]<ENT_ROCK_L || ent_type[j]>ENT_ROCK_S) continue;
            if (check_hit(0,j)) {
                erase_ent(0); ent_type[0]=ENT_NONE; alive=0;
                PlaySound(SND_EXPLODE);
                if (lives>0) lives=lives-1; spawn_timer=0;
                if (lives==0) do_game_over(); break;
            }
        }
    }
    /* Ship vs UFO */
    if (alive && ufo_active && ent_type[0]==ENT_SHIP) {
        if (check_hit(0, ufo_idx)) {
            erase_ent(0); ent_type[0]=ENT_NONE;
            erase_ent(ufo_idx); ent_type[ufo_idx]=ENT_NONE;
            alive=0; ufo_active=0; PlaySound(SND_EXPLODE);
            if (lives>0) lives=lives-1; spawn_timer=0;
            if (lives==0) do_game_over();
        }
    }
    /* UFO shots vs ship */
    if (alive && ent_type[0]==ENT_SHIP) {
        for (j = 0; j < MAX_ENTS; j++) {
            if (ent_type[j]!=ENT_USHOT) continue;
            if (check_hit(0,j)) {
                erase_ent(j); ent_type[j]=ENT_NONE;
                erase_ent(0); ent_type[0]=ENT_NONE; alive=0;
                PlaySound(SND_EXPLODE);
                if (lives>0) lives=lives-1; spawn_timer=0;
                if (lives==0) do_game_over(); break;
            }
        }
    }

    if (count_rocks()==0 && !game_over) { wave=wave+1; draw_stars(); spawn_wave(); }

    for (i = 0; i < MAX_ENTS; i++) {
        if (ent_type[i]!=ENT_NONE) {
            if (ent_type[i]==ENT_SHIP) erase_ent(i);
            draw_ent(i);
        }
    }
    draw_hud();
}

/* ---- Main ---- */
void main(void) {
    InitNGPC(); SysSetSystemFont(); install_tiles(); setup_palettes();
    if (!sound_installed) {
        InstallSoundDriver();
        InstallSounds(game_sounds, 6);
        sound_installed = 1;
    }
    thrust_snd_tick = 0;
    load_high_scores();
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
