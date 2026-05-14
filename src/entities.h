/* entities.h — Entity pool management */
#ifndef ENTITIES_H
#define ENTITIES_H

#include "game.h"

void setup_sprite_palettes(void);
void init_entities(void);
void erase_ent(u8 i);
void draw_ent(u8 i);
void move_ent(u8 i);
u8   check_hit(u8 a, u8 b);

/* Rocks */
void spawn_rock(u8 type, u8 px, u8 py);
void spawn_wave(void);
void destroy_rock(u8 idx);
u8   count_rocks(void);

/* Bullets */
void fire_bullet(void);

/* UFO */
void spawn_ufo(void);
void update_ufo(void);

/* Ship */
void warp_ship(void);
void spawn_explosion(u8 px, u8 py, u8 pal);

#endif
