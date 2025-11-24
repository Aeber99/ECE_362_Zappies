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


/* this can be the static background*/
int main() {
  stdio_init_all();
  initVGA();

  // // Draw sky and grass background
  fillRect(0, 0, 640, 360, CYAN);
  fillRect(0, 360, 640, 120, GREEN);
  while (1) {
    sleep_ms(1000);
  }
}



