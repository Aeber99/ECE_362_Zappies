#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"


const int SPI_7SEG_SCK = 30; // Replace with your SCK pin number for the 7-segment display
const int SPI_7SEG_CSn = 29; // Replace with your CSn pin number for the 7-segment display
const int SPI_7SEG_TX = 27; // Replace with your TX pin number for the 7-segment display

const int LED_PWM = 40;

void init_hit_sensor()
{

    // fill in    
    gpio_set_function(SPI_7SEG_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_CSn, GPIO_FUNC_SPI);
    
    spi_init(spi1, 125000); //  125 KHz
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); 

    gpio_set_function(LED_PWM, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PWM);
    uint channel = pwm_gpio_to_channel(LED_PWM);

    pwm_config c = pwm_get_default_config();
    pwm_init(slice_num, &c, false);

    //we want 15.7 Khz PWM for LED brightness control
    uint32_t sys_hz = clock_get_hz(clk_sys);
    // float clkdiv = (float)sys_hz / 1570000.0f;
    float clkdiv = (float)sys_hz / 1000000.0f;
    pwm_set_clkdiv(slice_num, clkdiv);
    
    // uint32_t wrap = 1000000u / freq_hz;
    uint32_t wrap = 1000000u / 15700; // 15.7 kHz (whattttt - Michael)
    // uint32_t wrap = 1000000u / 20; // 15.7 kHz (whattttt - Michael)
    pwm_set_wrap(slice_num, wrap - 1);
    pwm_set_chan_level(slice_num, channel, (wrap / 2));
    
    pwm_set_enabled(slice_num, true);
    // sleep_ms(10);

    // pwm_set_enabled(slice_num, false);
    // pwm_set_chan_level(slice_num, channel, 0);
}

void activate_hit_sensor(int id) {
    // fill in   
    printf("flag\n");
    uint16_t led_id;
    if (id == -1) {
        led_id = 0;
    } else {
        led_id = 1 << id;
    }
    
    spi_write16_blocking(spi1, &led_id, 1);
}


/*
plan of action:
1) since we are using p channel mosfets to charge to 5V, we need to make shift reg active low (not everything)
2) set 15.7 kHz led pwm, maybe another mosfet

*/
