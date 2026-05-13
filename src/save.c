/* save.c — Flash high score save */
#include "save.h"

#define SAVE_MAGIC 0xA5A5
static u16 save_buf[8];

void load_high_scores(void) {
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

void save_high_scores(void) {
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

void insert_high_score(u16 s) {
    u8 i, j;
    for (i = 0; i < 5; i++) {
        if (s > high_scores[i]) {
            j = 4;
            while (j > i) { high_scores[j] = high_scores[j-1]; j--; }
            high_scores[i] = s; return;
        }
    }
}
