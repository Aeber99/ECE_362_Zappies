#include "audio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define AUDIO_PIN 33

void audio_init(void) {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
}

void audio_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (freq_hz == 0 || duration_ms == 0) return;

    uint slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    uint chan  = pwm_gpio_to_channel(AUDIO_PIN);

    uint32_t sys_hz = clock_get_hz(clk_sys);
    float clkdiv = (float)sys_hz / 1000000.0f;
    if (clkdiv < 1.0f) clkdiv = 1.0f;
    pwm_set_clkdiv(slice, clkdiv);

    uint32_t wrap = 1000000u / freq_hz;
    if (wrap < 2) wrap = 2; 
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
