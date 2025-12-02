#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"


const int SPI_7SEG_SCK = 30; // Replace with your SCK pin number for the 7-segment display
const int SPI_7SEG_CSn = 29; // Replace with your CSn pin number for the 7-segment display
const int SPI_7SEG_TX = 27; // Replace with your TX pin number for the 7-segment display



void display_init_spi()
{
    // fill in    
    gpio_set_function(SPI_7SEG_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_CSn, GPIO_FUNC_SPI);
    
    spi_init(spi1, 125000); //  125 KHz
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); 
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
