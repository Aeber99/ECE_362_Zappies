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
#include "led.h"
#include "pico/rand.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

typedef enum {
    START = 0,      // Let player interface to start game (goto WAIT)
    WAIT = 1,       // Wait for BALLOON to appear (goto BALLOON)
    BALLOON = 2,       // BALLOON has appeared (goto CHECK if gun is triggered, goto MISS if times up)
    CHECK = 3,    // Respond to gun trigger (goto to BALLOON if missed, goto HIT if hit)
    HIT = 4,        // Indicate hit (goto WAIT if game is still going, goto FINISH if game is done)
    MISS = 5,       // Indicate miss (goto WAIT if game is still going, goto FINISH if game is done)
    FINISH = 6      // Show score (goto WAIT when player is ready)
} game_state_t;
game_state_t game_state = START;

uint32_t score = 0;
uint32_t seed = 0;
int short_timer_done = 0;
int long_timer_done = 0;


// /* this can be the static background*/
// int main() {
//   stdio_init_all();
//   initVGA();
  //  init_inputs();
//   audio_init();

//   // seed random number generator -> otherwise same sequence every time
//   srand(time_us_32());

//   // //testing RGB 
//   // fillRect(0, 0, 640, 480, BLACK);
//   // fillRect(80, 40, 160, 120, RED);  // i don't see red :( [-> think gpio 21 issue -> shows up now]
//   // fillRect(260, 40, 160, 120, GREEN); // i see green
//   // fillRect(170, 200, 160, 120, BLUE); // i see blue


//   // Show intro screen: black background, big title
//   fillRect(0, 0, 640, 480, BLACK);
//   // Big title in the middle
//   setTextColorBig(WHITE, BLACK);
//   setCursor(120, 140);
//   writeStringBig("DUCK HUNT");
//   // small prompt
//   setTextColor(WHITE);
//   setCursor(260, 360);
//   writeStringBig("Press button to start");

//   // press the button
//   wait_for_button_press();

//   // Start game screen: draw background and spawn initial duck
//   // // Draw sky and grass background
//   fillRect(0, 0, 640, 360, CYAN);
//   fillRect(0, 360, 640, 120, GREEN);

//   // //testing to see if yellow shows up -> faintly but yes :)
//   // fillRect(0, 0, 640, 480, CYAN);     // background
//   // fillRect(200, 160, 80, 80, YELLOW); // yellow square on top

//   // draw a static duck (doesn't move ryt?)
//   duck_x = 200;
//   duck_y = 80;
//   if (duck_alive) drawDuck(duck_x, duck_y);

//   // simulate a shot after 2 seconds to test shattering
//   sleep_ms(5000);
//   printf("Simulating shot at duck center (%d,%d)\n", duck_x + DUCK_W/2, duck_y + DUCK_H/2);
//   handleShot(duck_x + DUCK_W/2, duck_y + DUCK_H/2);


//   // now enter game loop for continuous spawning
//   while (1) {
//     sleep_ms(1000);
//   }

//     // stdio_init_all();
//     // display_init_spi();
//     // for (;;) {
//     //   for(int i = 0; i < 8; i++){
//     //     display_print(i);
//     //     printf("LED %d\n", i);
//     //     sleep_ms(500);
//     //   }
        
//     // }

//     // for(;;);
//     // return 0;




// /*
// to do :    
// 1) make kill logic more dramatic 
//   -spining duck? (idk how to do this)
//   -add sound?
//   -kind of like a shatter effect with pieces? (this might be nice)  --done and satisfied ish :D
//   -make it change color to red when shot? -> might not be that hard (unsatisfying red doesnt show up well)

// 2) multiple ducks (for now they spaw randomly in the sky)
//   -they move into the screen from random heights and speeds
//   -need to manage ducks (alive/dead, position, speed)

// 3)make a nicer looking duck? 
//   - 2 black eyes and a orange triangle beak? -> i tried but cant art :(
//   -or 3 square's of differnent sizes (big body small face tiny tiny beak/tail)

// */



// //further to do :

//     // for (;;) {
//     //     printf("Hello world!\n");
//     //     sleep_ms(1000);
//     // }

//     // Code for testing gun
//     init_gun(21, 26);
    
// }




int main() {
    stdio_init_all();
  initVGA();
  init_inputs();
  audio_init();
    init_dummy_controls();
  // Show intro screen: black background, big title
  fillRect(0, 0, 640, 480, BLACK);
  // Big title in the middle
  setTextColorBig(WHITE, BLACK);
  setCursor(120, 140);
  writeStringBig("BALLOON POP");
  // small prompt
  setTextColor(WHITE);
  setCursor(260, 360);
  writeStringBig("Press button to start");

    // START logic
    
    // *************
    // Write code for start screen logic
    printf("Press buttton to start game\n");
    // dummy_control should move state to WAIT
    while (game_state == START);
    // **************

    // Start long timer
    init_game_timer_long(60000000); // Game last: 60 seconds

    for (;;) {
        // Reset score
        score = 0;

        // Run as long as long timer is going
        while(long_timer_done == 0) {
            // WAIT Logic
            printf("Waiting\n");
            seed = get_rand_32() % 30;
            init_game_timer_short(seed * 100000 + 4000000);
            // timer short should isr should move state to BALLOON
            while (game_state == WAIT);

            // BALLON Logic
            printf("BALLOoN\n");
            seed = get_rand_32() % 30;
            init_game_timer_short(seed * 100000 + 5000000);
            while (game_state != WAIT);
        }

        // FINISH logic
    
        // *************
        // Write code for finish screen logic
        printf("Game finished!\n");
        printf("Score: %d\n", (int)score);
        printf("Press buttton to play again\n");
        // dummy_control should move state to WAIT
        while (game_state == FINISH);
        // **************
    }

    
}
