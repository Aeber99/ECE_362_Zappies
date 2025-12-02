#include "audio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define AUDIO_PIN 45

int sound_id = 0;
int note_count = 0;
int note_frequencies[2][3] = {{1047, 2043, 0}, {262, 131, 0}};
int note_length[2][3] = {{2000,2000, 0}, {2000,2000, 0}};

timer_hw_t* audio_timer = timer1_hw;


void audio_init(void) {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
}

void audio_play_tone(uint32_t freq_hz, uint32_t duration_ms) {

    uint slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    uint chan  = pwm_gpio_to_channel(AUDIO_PIN);

    uint32_t sys_hz = clock_get_hz(clk_sys);
    float clkdiv = (float)sys_hz / 1000000.0f;
    pwm_set_clkdiv(slice, clkdiv);

    uint32_t wrap = 1000000u / freq_hz;
    pwm_set_wrap(slice, wrap - 1);

    pwm_set_chan_level(slice, chan, (wrap - 1) / 2);
    pwm_set_enabled(slice, true);
    sleep_ms(duration_ms);

    pwm_set_enabled(slice, false);
    pwm_set_chan_level(slice, chan, 0);
}

void audio_play_shot(void) {
    audio_play_tone(2500, 30);
    audio_play_tone(1400, 40);
    audio_play_tone(900, 50);
}


/*
to do :
1) replace sine wave with square wave
2) replace all sleep with timers
3) create ISRs to change freq of PWM
4) create fake "state machine" to time music note sequences
*/

// 2) is below

// NOTE: I only see one sleep_ms, is that the only one you want changed 
// to a timer
void timer_isr(uint32_t duration_ms) {
    //acknowledge ALARM0 interrupt on timer1
    hw_clear_bits(&timer1_hw->intr, 1u << 0);
    timer1_hw->intr |= (1 << 0); //originally was intr

    // sio_hw->gpio_clr = (0b11111111111 << 10);
    // sio_hw->gpio_set = (index << 18) | (msg[index] << 10);

    // index = (index + 1) % 8;

    //make TIMER1 ALARM0 fire interrupt again in 3ms
    irq_set_exclusive_handler(TIMER1_IRQ_0, timer_isr); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER1_IRQ_0, true);
    uint64_t target = timer1_hw->timerawl + 3000;
    timer1_hw->alarm[0] = (uint32_t) target;
}

void timer_setup(){
    //playsound functionality to assign sound_id and note_count
    audio_timer->inte |= 1u << 0;
    
    irq_set_exclusive_handler(TIMER1_IRQ_0, timer_isr); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER1_IRQ_0, true);
    // uint64_t target = timer1_hw->timerawl + duration_ms;
    // timer1_hw->alarm[0] = (uint32_t) target;
}


// adjust in audio_isr (can use mary's play_tone function)

void playsound(int id){
    sound_id = id;
    note_count = 0;

    timer_setup();
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);
    int chan_num = pwm_gpio_to_channel(AUDIO_PIN);

    pwm_config c = pwm_get_default_config();
    pwm_init(slice_num, &c, false);

    pwm_set_clkdiv(slice_num, 150);
    pwm_set_chan_level(slice_num, chan_num, 10);
    

    playsound_isr();

}

void playsound_isr(){
    hw_clear_bits(&timer1_hw->intr, 1u << 0);
    timer1_hw->intr |= (1 << 0); //originally was intr

    uint slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    uint chan  = pwm_gpio_to_channel(AUDIO_PIN);

    int curr_note_frequency = note_frequencies[sound_id][note_count];
    int curr_note_length = note_length[sound_id][note_count];
  
    if (curr_note_length == 0) {
        pwm_set_enabled(slice, false);
        pwm_set_chan_level(slice, chan, 0);
    } else {
        int period = 1000000 / curr_note_frequency;

        pwm_set_wrap(slice, period - 1);

        pwm_set_chan_level(slice, chan, (period - 1) / 2);
        pwm_set_enabled(slice, true);

        //make TIMER1 ALARM0 fire interrupt again in 3ms
        irq_set_exclusive_handler(TIMER1_IRQ_0, timer_isr); //set irq handler for alarm irq (needs ALARM0??)
        //enable alarm irq
        irq_set_enabled(TIMER1_IRQ_0, true);
        uint64_t target = timer1_hw->timerawl + curr_note_length;
        timer1_hw->alarm[0] = (uint32_t) target;

        note_count++;
    }
}