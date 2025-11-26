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
#include "duck_hunt.h"
#include "button.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "gun.c"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "audio.h"


/* this can be the static background*/
int main() {
  stdio_init_all();
  initVGA();
  init_inputs();
  audio_init();

  // seed random number generator -> otherwise same sequence every time
  srand(time_us_32());

  // //testing RGB 
  // fillRect(0, 0, 640, 480, BLACK);
  // fillRect(80, 40, 160, 120, RED);  // i don't see red :( [-> think gpio 21 issue -> shows up now]
  // fillRect(260, 40, 160, 120, GREEN); // i see green
  // fillRect(170, 200, 160, 120, BLUE); // i see blue


  // Show intro screen: black background, big title
  fillRect(0, 0, 640, 480, BLACK);
  // Big title in the middle
  setTextColorBig(WHITE, BLACK);
  setCursor(120, 140);
  writeStringBig("DUCK HUNT");
  // small prompt
  setTextColor(WHITE);
  setCursor(260, 360);
  writeStringBig("Press button to start");

  // press the button
  wait_for_button_press();

  // Start game screen: draw background and spawn initial duck
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


  // now enter game loop for continuous spawning
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
  -make it change color to red when shot? -> might not be that hard (unsatisfying red doesnt show up well)

2) multiple ducks (for now they spaw randomly in the sky)
  -they move into the screen from random heights and speeds
  -need to manage ducks (alive/dead, position, speed)

3)make a nicer looking duck? 
  - 2 black eyes and a orange triangle beak? -> i tried but cant art :(
  -or 3 square's of differnent sizes (big body small face tiny tiny beak/tail)

*/



