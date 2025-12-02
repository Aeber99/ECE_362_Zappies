
#include "pico/rand.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

typedef enum {
    START = 0,      // Let player interface to start game (goto WAIT)
    WAIT = 1,       // Wait for bird to appear (goto BIRD)
    BIRD = 2,       // Bird has appeared (goto CHECK if gun is triggered, goto MISS if times up)
    CHECK = 3,    // Respond to gun trigger (goto to BIRD if missed, goto HIT if hit)
    HIT = 4,        // Indicate hit (goto WAIT if game is still going, goto FINISH if game is done)
    MISS = 5,       // Indicate miss (goto WAIT if game is still going, goto FINISH if game is done)
    FINISH = 6      // Show score (goto WAIT when player is ready)
} game_state_t;
game_state_t game_state = START;


timer_hw_t *game_timer = timer0_hw;
uint32_t game_alarm_short = 0; 
uint32_t game_alarm_long = 1;

uint32_t score = 0;


int short_timer_done = 0;
void game_timer_isr_short() {
    short_timer_done = 1;
    hw_clear_bits(&game_timer->intr, 1u << game_alarm_short);

    if (game_state == WAIT) {
        game_state = BIRD;
    }
    if (game_state == BIRD) {
        printf("Bird flew away\n");
        game_state = WAIT;
    }
}

void init_game_timer_short(int us) {
    short_timer_done = 0;

    hw_set_bits(&game_timer->inte, 1u << game_alarm_short);
    irq_set_exclusive_handler(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_short), game_timer_isr_short);
    irq_set_enabled(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_short), true);

    int target = game_timer->timerawl + us;
    game_timer->alarm[game_alarm_short] = (uint32_t) target;
}

void kill_game_timer_short() {
    game_timer->alarm[game_alarm_short] = 0;
}

int long_timer_done = 0;
void game_timer_isr_long() {
    long_timer_done = 1;
    hw_clear_bits(&game_timer->intr, 1u << game_alarm_long);
}

void init_game_timer_long(int us) {
    long_timer_done = 0;
    hw_set_bits(&game_timer->inte, 1u << game_alarm_long);
    irq_set_exclusive_handler(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_long), game_timer_isr_long);
    irq_set_enabled(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_long), true);

    int target = game_timer->timerawl + us;
    game_timer->alarm[game_alarm_long] = (uint32_t) target;

}

void dummy_controls_isr() {
    if (gpio_get_irq_event_mask(21) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
    }
    printf("Press\n");

    if (game_state ==  START) {
        game_state = WAIT;
    } else if (game_state == BIRD) {
        // CHECK LOGIC
        game_state = CHECK;

        // ******************
        // ADD CHECK LOGIC HERE
        int hit = gpio_get(26);
        if (hit) {
            printf("HIT\n");
            score++;
            kill_game_timer_short();
            game_state = HIT;
            sleep_ms(1000);
            game_state = WAIT;

        } else {
            printf("MISS\n");
            score--;
            game_state = MISS;
            game_state = BIRD;
        }
        // ******************
    }
}

void init_dummy_controls() {
    gpio_init(21);
    gpio_init(26);
    gpio_set_dir(21, 0);
    gpio_set_dir(26, 0);

    gpio_add_raw_irq_handler(21, dummy_controls_isr);
    gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

int seed = 0;
