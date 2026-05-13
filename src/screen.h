/* screen.h — Title, HUD, scores, background */
#ifndef SCREEN_H
#define SCREEN_H

#include "game.h"

void setup_palettes(void);
void draw_title(void);
void draw_scores(void);
void draw_hud(void);
void init_stars(void);
void draw_stars(void);
void put_score_at(u8 x, u8 y, u16 val);

#endif
