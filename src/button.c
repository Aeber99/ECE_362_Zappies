#include "button.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>


#define BUTTON_GPIO 26

void init_inputs(void) {
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);
}

bool button_pressed(void) {
    // active low- pressed when reads 0
    return gpio_get(BUTTON_GPIO) == 0;
}

void wait_for_button_press(void) {
    // wait for stable pressed state for 30 ms
    while (1) {
        if (button_pressed()) {
            printf("Button press detected");
            sleep_ms(30);
            if (button_pressed()) {


                while (button_pressed()) 
                
                sleep_ms(10);
                printf("Button released, continuing\n");
                return;
            }
        }
        sleep_ms(10);
    }
}

