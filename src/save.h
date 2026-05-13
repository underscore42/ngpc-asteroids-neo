/* save.h — Flash high score save */
#ifndef SAVE_H
#define SAVE_H

#include "game.h"

void load_high_scores(void);
void save_high_scores(void);
void insert_high_score(u16 s);

#endif
