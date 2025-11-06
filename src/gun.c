#include "pico/stdlib.h"
#include <stdio.h>

int hit_gpio = -1;
int trigger_gpio = -1;

void init_gun(int hit_pin, int trigger_pin) {
    hit_gpio = hit_pin;
    trigger_gpio = trigger_pin;

    gpio_init(hit_gpio);
    gpio_init(trigger_gpio);

    gpio_set_dir(hit_gpio, GPIO_IN);
    gpio_set_dir(trigger_gpio, GPIO_IN);

    gpio_add_raw_irq_handler_masked((1 << trigger_gpio), gun_isr);
    gpio_set_irq_enabled(trigger_gpio, GPIO_IRQ_EDGE_RISE, true);
    irq_set_enabled(IO_IRQ_BANK0, true);

}

void gun_isr() {
    if (gpio_get_irq_event_mask(trigger_gpio) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(trigger_gpio, GPIO_IRQ_EDGE_RISE);
        if (gpio_get(hit_gpio)) {
            printf("Hit!\n");
        }
    }
}