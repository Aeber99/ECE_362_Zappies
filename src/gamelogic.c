
#include "pico/rand.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "vga16_graphics_v2.h"
#include "pico/stdlib.h"
#include "pico/time.h"

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


timer_hw_t *game_timer = timer0_hw;
uint32_t game_alarm_short = 0; 
uint32_t game_alarm_long = 1;
uint32_t game_alarm_tick = 2;

uint32_t score = 0;


int short_timer_done = 0;

void game_timer_isr_short() {
    short_timer_done = 1;
    hw_clear_bits(&game_timer->intr, 1u << game_alarm_short);

    if (game_state == WAIT) {
        game_state = BALLOON;
    // }
        // if (game_state == BIRD) {
    } else if (game_state == BALLOON) {
        printf("Duck flew away\n");
        game_state = WAIT;
    } else if (game_state == HIT) {
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
    start_game_clock_seconds(us / 1000000);

}

static int remaining_seconds = 0;

void update_time(int num) {
    char buf[32];
    int minutes = num / 60;
    int seconds = num % 60;
    snprintf(buf, sizeof(buf), "Time: %02d:%02d", minutes, seconds);
    fillRect(10, 10, 120, 40, BLACK);
    setTextColor(WHITE);
    setCursor(10, 10);
    writeStringBig(buf);
}

//per sec stuff 
void game_timer_isr_tick() {
    hw_clear_bits(&game_timer->intr, 1u << game_alarm_tick);
    printf("Tick! Remaining seconds: %d\n", remaining_seconds);

    if (remaining_seconds > 0) {
        remaining_seconds--;
        update_time(remaining_seconds);
        if (remaining_seconds > 0) {
            int target = game_timer->timerawl + 1000000;
            game_timer->alarm[game_alarm_tick] = (uint32_t)target;
        }
    }
}

//called to start the game clock
void start_game_clock_seconds(int seconds) {
    //initial countdown value
    remaining_seconds = seconds;

    update_time(remaining_seconds);

    hw_set_bits(&game_timer->inte, 1u << game_alarm_tick);
    irq_set_exclusive_handler(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_tick), game_timer_isr_tick);
    irq_set_enabled(TIMER_ALARM_IRQ_NUM(game_timer, game_alarm_tick), true);

    int target = game_timer->timerawl + 1000000;
    game_timer->alarm[game_alarm_tick] = (uint32_t)target;
}

//logic to update score 
void update_score(int hit) { //1 or -1 
if (hit == 1) 
{
    score += (uint32_t)hit * 100;
}
else if (hit == -1){
    score -= (uint32_t)(-hit) * 100;  //(-1)*(-1)=100 -> so -100 overall ryt
}
char buf[32];
snprintf(buf, sizeof(buf), "Score: %u", (unsigned int)score);
// fillRect(480, 10, 540, 40, BLACK);
fillRect(10, 10, 120, 40, BLACK);
setTextColor(WHITE);
setCursor(10, 10);
writeStringBig(buf);
}


void dummy_controls_isr() {
    if (gpio_get_irq_event_mask(21) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
    }
    printf("Press\n");

    if (game_state ==  START) {
        update_score(0);
        printf("Start\n");
        game_state = WAIT;
    } else if (game_state == BALLOON) {
        // CHECK LOGIC
        game_state = CHECK;
        // ******************
        // ADD CHECK LOGIC HERE
        int hit = gpio_get(26);
        if (hit) {
            printf("HIT\n");
            // update score: +100 per hit
            update_score(1);
            kill_game_timer_short();
            game_state = HIT;
            // sleep_ms(1000);
            game_state = WAIT;
            // handled by short timer ISR instead of sleep_ms
            init_game_timer_short(1000000);

        } else {
            printf("MISS\n");
            // score --;
            // keep score unchanged on miss?
            update_score(-1);
            game_state = MISS;
            game_state = BALLOON;
        }
        // ******************
    }
}

void init_dummy_controls() {
    gpio_init(21); // trigger
    gpio_init(26); // hit 
    gpio_set_dir(21, 0);
    gpio_set_dir(26, 0);

    gpio_add_raw_irq_handler(21, dummy_controls_isr);
    gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

int seed = 0;
