/**
 * 
    * VGA 640x480x16 graphics driver for RP2040
 * Uses PIO-assembly VGA driver
 *
 * HARDWARE CONNECTIONS
   - GPIO 16 ---> VGA Hsync 
   - GPIO 17 ---> VGA Vsync 
   - GPIO 18 ---> VGA Green lo-bit --> 470 ohm resistor --> VGA_Green
   - GPIO 19 ---> VGA Green hi_bit --> 330 ohm resistor --> VGA_Green
   - GPIO 20 ---> 330 ohm resistor ---> VGA-Blue 
   - GPIO 21 ---> 330 ohm resistor ---> VGA-Red 
   - RP2040 GND ---> VGA-GND
 * 
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels obtained by claim mechanism
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 */
#include "vga16_graphics_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

// int main() {

//     // Overclock
//     // set_sys_clock_khz(150000, true) ;

//     // Initialize stdio
//     stdio_init_all();

//     // Initialize VGA
//     initVGA() ;

//     /////////////////////////////////////////////////////////////////////
//     /////////////////////////////////////// Game of Life ////////////////
//     /////////////////////////////////////////////////////////////////////

//     // Initialize the screen (specific eternal growth initial conditions)
//     drawCell(160, -50+70, WHITE) ;
//     drawCell(160, -50+71, WHITE) ;
//     drawCell(160, -50+72, WHITE) ;
//     drawCell(160, -50+73, WHITE) ;
//     drawCell(160, -50+74, WHITE) ;
//     drawCell(160, -50+75, WHITE) ;
//     drawCell(160, -50+76, WHITE) ;
//     drawCell(160, -50+77, WHITE) ;

//     drawCell(160, -50+79, WHITE) ;
//     drawCell(160, -50+80, WHITE) ;
//     drawCell(160, -50+81, WHITE) ;
//     drawCell(160, -50+82, WHITE) ;
//     drawCell(160, -50+83, WHITE) ;

//     drawCell(160, -50+87, WHITE) ;
//     drawCell(160, -50+88, WHITE) ;
//     drawCell(160, -50+89, WHITE) ;

//     drawCell(160, -50+96, WHITE) ;
//     drawCell(160, -50+97, WHITE) ;
//     drawCell(160, -50+98, WHITE) ;
//     drawCell(160, -50+99, WHITE) ;
//     drawCell(160, -50+100, WHITE) ;
//     drawCell(160, -50+101, WHITE) ;
//     drawCell(160, -50+102, WHITE) ;

//     drawCell(160, -50+104, WHITE) ;
//     drawCell(160, -50+105, WHITE) ;
//     drawCell(160, -50+106, WHITE) ;
//     drawCell(160, -50+107, WHITE) ;
//     drawCell(160, -50+108, WHITE) ;
    
//     int i = 0 ;
//     int j = 0 ;
//     char saved_row[320] = {BLACK} ;
//     char updated_row[320] = {BLACK} ;
//     int saved_row_num = 238 ;

//     int living ;
//     int neighbors ;

//     uint32_t start_time ;
//     uint32_t end_time ;

//     while(1) {

//         start_time = time_us_32() ;

//         for (j=1; j<239; j++) {
//             for (i=1; i<319; i++) {
//                 // Check if cell is alive, and get number of neighbors
//                 living = isAlive(i, j) ;
//                 neighbors = checkNeighbors(i, j) ;

//                 // Apply rules, save living/dead status in saved_row array
//                 if (living && ((neighbors==2) || (neighbors==3))) {
//                     updated_row[i] = WHITE ;
//                 }

//                 else if (!living && (neighbors==3)) {
//                     updated_row[i] = WHITE ;
//                 }

//                 else {
//                     updated_row[i] = BLACK ;
//                 }
//             }
//             // Draw the saved row
//             for (i=0; i<319; i++) {
//                 drawCell(i, saved_row_num, saved_row[i]) ;
//             }
//             // Move the updated row to the saved row
//             memcpy(saved_row, updated_row, 320) ;
//             // Increment the saved row number, wrapping at 318
//             saved_row_num += 1 ;
//             if (saved_row_num >= 239) {
//                 saved_row_num = 1 ;
//             }
//         }

//         end_time = time_us_32() ;
//         printf("Time to animate: %f\n", (float)(end_time-start_time)*(1./1000000.)) ;
//     }
// }





// //testing understanding using a simple bouncing square
// int main() {
//     stdio_init_all();
//     initVGA();

//     // initial background clear
//     fillRect(0, 0, 640, 480, BLACK);

//     int x = 100, y = 100;
//     int w = 20, h = 20;
//     int vx = 2, vy = 1;
//     while (1) {
//         // erase previous
//         fillRect(x, y, w, h, BLACK);

//         // update physics
//         x += vx;
//         y += vy;
//         if (x <= 0 || (x + w) >= 640) vx = -vx;
//         if (y <= 0 || (y + h) >= 480) vy = -vy;

//         // draw square
//         fillRect(x, y, w, h, WHITE);

//         // frame timing ~60Hz
//         sleep_ms(16);
//     }
// }


// //do we want a rolling screen -> might be hard
// //have a single pixel move across the screen back and forth 
// int main() {
//     stdio_init_all();
//     initVGA();

// //     // initial background clear
//     for (int yy=0; yy<240; yy++)
//       for (int xx=0; xx<320; xx++)
//         drawCell(xx, yy, BLACK);

//     int cx = 10, cy = 10, vx = 1, vy = 0;
//     while (1) {
//       // erase old
//       drawCell(cx, cy, BLACK);
//       cx += vx; cy += vy;
//       if (cx <= 0 || cx >= 319) vx = -vx;
//       if (cy <= 0 || cy >= 239) vy = -vy;
//       drawCell(cx, cy, WHITE);
//       sleep_ms(60);
//     }
// }


//starting duck hunt logic here
// draw a yellow rectangle and erase it
// restore the sky color (keep duck in sky region?)
 const int DUCK_W = 80;
 const int DUCK_H = 80;

// duck state1 (so hit can access it) -> is it okay for this to be global?
int duck_x = 200;
int duck_y = 80;
int duck_alive = 1;

 void drawDuck(int x, int y) {
  fillRect(x, y, DUCK_W, DUCK_H, YELLOW);
}

 void eraseDuck(int x, int y);


// hit detection: returns 1 if shot at (sx,sy)is within the duck box
 int isShotInDuck(int sx, int sy) {
  if (!duck_alive) return 0;
  if (sx >= duck_x && sx < (duck_x + DUCK_W) && sy >= duck_y && sy < (duck_y + DUCK_H))
    return 1;
  return 0;
}

// // kill logic: make the duck turn red and fall straight down, then disappear
 void shatterDuck(void) {
//   // red square falling from the duck's current position
//   int prev_y = duck_y;

//   // draw initial red duck
//   fillRect(duck_x, prev_y, DUCK_W, DUCK_H, RED);
//   sleep_ms(50);

//   // step down until the rectangle reaches the bottom of the screen
//   while (prev_y + DUCK_H < 480) {
//     // erase the previous red rectangle by restoring the background
//     eraseDuck(duck_x, prev_y);

//     // move down a bit
//     prev_y += 4;

//     // draw at new position
//     fillRect(duck_x, prev_y, DUCK_W, DUCK_H, RED);
//     sleep_ms(30);
//   }

//   // final erase to remove the last red rectangle
//   eraseDuck(duck_x, prev_y);
// }


// Kill logic: break duck into pixels that fly outward and disappear
// keep it large pieces-> make step size bigger
const int STEP = 4; // spacing / to determine the size of the chunks
const int PIECE_SIZE = 4; // block size -> creates a 4x4 block
const int max_w = (DUCK_W + STEP - 1) / STEP;   //(using ceiling int div?)
const int max_h = (DUCK_H + STEP - 1) / STEP;
const int MAX_PARTICLES = max_w * max_h;

// current coordinates of the pieces
 int part_x[MAX_PARTICLES];
    int part_y[MAX_PARTICLES];
// velocity of the pieces -> dont need?
    int part_vx[MAX_PARTICLES];
    int part_vy[MAX_PARTICLES];
// -> dont need?->unless we try to get it to turn red on shot
    char part_color[MAX_PARTICLES];
 // -> to check if within the radius -> if outside then 0( it ded)
    int part_alive[MAX_PARTICLES];
    int n = 0; // current count of particles created

int cx = duck_x + DUCK_W/2;
int cy = duck_y + DUCK_H/2;

// initialize particles
for (int iy = 0; iy < DUCK_H; iy += STEP) {
for (int ix = 0; ix < DUCK_W; ix += STEP) {
int sx = duck_x + ix;
int sy = duck_y + iy;
char c = (char) readPixel(sx, sy);


            part_x[n] = sx;
            part_y[n] = sy;
            part_color[n] = c;
            part_alive[n] = 1;

// simple random diagonal-ish velocity

//look into using rand -> and using it PROPERLY**
part_vx[n] = (rand() % 3) + 1;
        part_vy[n] = (rand() % 3) + 1;
            if (rand() & 1) part_vx[n] = -part_vx[n];
            if (rand() & 1) part_vy[n] = -part_vy[n];
n++; // Increment
}
}

// remove original duck
eraseDuck(duck_x, duck_y);

// shatter parameters
const int FRAMES = 50;
const int RADIUS = 60; // containment radius from center

for (int f = 0; f < FRAMES; ++f) {
//clear clear
int left = cx - RADIUS;
int top = cy - RADIUS;
int right = cx + RADIUS + PIECE_SIZE;
int bottom = cy + RADIUS + PIECE_SIZE;
if (left < 0) left = 0;
if (top < 0) top = 0;
if (right > 639) right = 639;
if (bottom > 479) bottom = 479;
int w = right - left;
int h = bottom - top;

// bkg for square, and handling sky/grass split - uh needed? or keep in sky?
if (bottom <= 360) {
fillRect(left, top, w, h, CYAN);
} else if (top >= 360) {
fillRect(left, top, w, h, GREEN);
} else {
// split
int top_h = 360 - top;
fillRect(left, top, w, top_h, CYAN);
fillRect(left, 360, w, h - top_h, GREEN);
}

// update and draw particles

for (int i = 0; i < n; ++i) {
            // alive status
            if (!part_alive[i])
continue;
            // update position
            part_x[i] += part_vx[i]; //horizontal
            part_y[i] += part_vy[i]; //vertical
            // if particle leaves radius,dead
            int dx = part_x[i] - cx;
            int dy = part_y[i] - cy;
            if (dx*dx + dy*dy > RADIUS*RADIUS) {  //make a circle
                part_alive[i] = 0; // dead
                continue;
}

// draw piece
if (part_x[i] + PIECE_SIZE > 0 && part_x[i] < 640 && part_y[i] +
PIECE_SIZE > 0 &&
                part_y[i] < 480) {
                fillRect(part_x[i], part_y[i], PIECE_SIZE, PIECE_SIZE,
part_color[i]);
            } else {
                part_alive[i] = 0; // dead if it flies off-screen
            }
        }
        //slow frame for a more gradual shatter
        sleep_ms(60);
    }

// clear clear
int left = cx - RADIUS;
int top = cy - RADIUS;
if (left < 0) left = 0;
if (top < 0) top = 0;
int right = cx + RADIUS + PIECE_SIZE;
int bottom = cy + RADIUS + PIECE_SIZE;
if (right > 639) right = 639;
if (bottom > 479) bottom = 479;
int w = right - left;
int h = bottom - top;

//make the bkg again


if (bottom <= 360) {
fillRect(left, top, w, h, CYAN);
} else if (top >= 360) {
fillRect(left, top, w, h, GREEN);
} else {
int top_h = 360 - top;
fillRect(left, top, w, top_h, CYAN);
fillRect(left, 360, w, h - top_h, GREEN);
}
}

// shot occurs at screen coordinates (sx,sy).
// if it hits the duck, the duck will shatter -> it ded
 void handleShot(int sx, int sy) {
  if (isShotInDuck(sx, sy)) {
    if (duck_alive) {
      duck_alive = 0;
      shatterDuck();
    }
  }
}

 void eraseDuck(int x, int y) {
  int sky_top = y;
  int sky_bottom = y + DUCK_H;
  if (sky_bottom <= 360) {
    // fully in sky
    fillRect(x, y, DUCK_W, DUCK_H, CYAN);
  } else if (sky_top >= 360) {
    // fully in grass
    fillRect(x, y, DUCK_W, DUCK_H, GREEN);
  } else {
    // splits sky/grass: top part = CYAN, bottom part = GREEN
    int top_h = 360 - sky_top;
    int bot_h = sky_bottom - 360;
    fillRect(x, sky_top, DUCK_W, top_h, CYAN);
    fillRect(x, 360, DUCK_W, bot_h, GREEN);
  }
}
/* this can be the static background*/
int main() {
  stdio_init_all();
  initVGA();

  // //testing RGB 
  // fillRect(0, 0, 640, 480, BLACK);
  // fillRect(80, 40, 160, 120, RED);  // i don't see red :( [-> think gpio 21 issue -> shows up now]
  // fillRect(260, 40, 160, 120, GREEN); // i see green
  // fillRect(170, 200, 160, 120, BLUE); // i see blue

  // // Draw sky and grass background
  fillRect(0, 0, 640, 360, CYAN);
  fillRect(0, 360, 640, 120, GREEN);

  // //testing to see if yellow shows up -> faintly but yes :)
  // fillRect(0, 0, 640, 480, CYAN);     // background
  // fillRect(200, 160, 80, 80, YELLOW); // yellow square on top

  // draw a static duck (doesn't move ryt?)
  duck_x = 200;
  duck_y = 80;
  if (duck_alive) drawDuck(duck_x, duck_y);

  // simulate a shot after 2 seconds to test shattering
  sleep_ms(5000);
  printf("Simulating shot at duck center (%d,%d)\n", duck_x + DUCK_W/2, duck_y + DUCK_H/2);
  handleShot(duck_x + DUCK_W/2, duck_y + DUCK_H/2);


  while (1) {
    sleep_ms(1000);
  }
}

/*
to do :    
1) make kill logic more dramatic 
  -spining duck? (idk how to do this)
  -add sound?
  -kind of like a shatter effect with pieces? (this might be nice)  --done and satisfied ish :D
  -make it change color to red when shot? -> might not be that hard 

2) multiple ducks 
  -they move into the screen from random heights and speeds
  -need to manage ducks (alive/dead, position, speed)

3)make a nicer looking duck? 
  - 2 black eyes and a orange triangle beak?
  -or 3 square's of differnent sizes (big body small face tiny tiny beak/tail)

*/




