#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <stdbool.h>

extern int short_timer_done;
extern int long_timer_done;
extern uint32_t score;
extern uint32_t seed;
extern enum game_state_t {
    START,
    WAIT,
    BALLOON,
    CHECK,
    HIT,
    MISS,
    FINISH
} game_state;

void game_timer_isr_short();
void init_game_timer_short(int us);
void kill_game_timer_short();
void game_timer_isr_long();
void init_game_timer_long(int us);
void dummy_controls_isr();
void init_dummy_controls() ;
void update_score(int hit);
void update_time(int num);

#endif // GAMELOGIC_H 