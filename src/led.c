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

void display_init_spi()
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
    float clkdiv = (float)sys_hz / 15700.0f;
    pwm_set_clkdiv(slice_num, clkdiv);
    
    pwm_set_chan_level(slice_num, channel, 10); //highest possible duty cycle
    pwm_set_enabled(slice_num, true);
    // sleep_ms(10);

    // pwm_set_enabled(slice_num, false);
    // pwm_set_chan_level(slice_num, channel, 0);
}

void display_print(int id) {
    // fill in   
    uint16_t led_id = 1 << id;
    spi_write16_blocking(spi1, &led_id, 1);
}


/*
plan of action:
1) since we are using p channel mosfets to charge to 5V, we need to make shift reg active low (not everything)
2) set 15.7 kHz led pwm, maybe another mosfet

*/
