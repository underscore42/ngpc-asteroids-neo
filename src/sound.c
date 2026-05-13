/* sound.c — Sound effects */
#include "sound.h"

static const SOUNDEFFECT game_sounds[] = {
    /* 0: fire */    { 1, 3, 0, 0x0180, 0x0020, 1, 0, 0x0100, 0x0200, 15, 3, 1, 0, 0, 15 },
    /* 1: thrust */  { 2, 4, 0, 0x0040, 0x0010, 2, 0, 0x0020, 0x0100, 12, 2, 1, 0, 0, 15 },
    /* 2: warp */    { 0, 6, 0, 0x0200, 0x0040, 1, 0, 0x0040, 0x0300, 14, 1, 2, 0, 0, 15 },
    /* 3: explode */ { 2, 8, 0, 0x0030, 0x0008, 2, 0, 0x0010, 0x0080, 15, 1, 2, 0, 0, 15 },
    /* 4: ufo hum */ { 0, 10, 1, 0x0100, 0x0010, 3, 1, 0x00C0, 0x0140, 8, 1, 3, 1, 4, 10 },
    /* 5: ufo fire */{ 1, 3, 0, 0x0140, 0x0030, 1, 0, 0x0080, 0x0200, 13, 2, 1, 0, 0, 15 }
};

void sound_init(void) {
    InstallSoundDriver();
    InstallSounds(game_sounds, 6);
}
