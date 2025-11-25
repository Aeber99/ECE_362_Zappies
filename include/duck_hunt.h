#ifndef DUCK_HUNT_H
#define DUCK_HUNT_H

#include "vga16_graphics_v2.h"

// Constants
extern const int DUCK_W;
extern const int DUCK_H;
extern const int STEP;
extern const int PIECE_SIZE;
#define MAX_PARTICLES 400

// Duck state
extern int duck_x;
extern int duck_y;
extern int duck_alive;

// Particle arrays
extern int part_x[];
extern int part_y[];
extern int part_vx[];
extern int part_vy[];
extern char part_color[];
extern int part_alive[];
extern int n;

// Function declarations
void drawDuck(int x, int y);
void eraseDuck(int x, int y);
int isShotInDuck(int sx, int sy);
void shatterDuck(void);
void handleShot(int sx, int sy);
void spawnRandomDuck(void);
void initDuckHunt(void);

#endif // DUCK_HUNT_H