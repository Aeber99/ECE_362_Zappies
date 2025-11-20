#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
// 7-segment display message buffer
// Declared as static to limit scope to this file only.
static char msg[8] = {
    0x3F, // seven-segment value of 0
    0x06, // seven-segment value of 1
    0x5B, // seven-segment value of 2
    0x4F, // seven-segment value of 3
    0x66, // seven-segment value of 4
    0x6D, // seven-segment value of 5
    0x7D, // seven-segment value of 6
    0x07, // seven-segment value of 7
};
extern char font[]; // Font mapping for 7-segment display
static int index = 0; // Current index in the message buffer

// We provide you with this function for directly displaying characters.
// This now accounts for the decimal point.
void display_char_print(const char message[]) {
    int dp_found = 0;
    for (int i = 0; i < 8; i++) {
        if (message[i] == '.') {
            // add it to the decimal point bit for prev digit if i > 0
            if (i > 0) {
                msg[i - 1] |= (1 << 7); // Set the decimal point bit
                dp_found = 1; // Mark that we found a decimal point
            }
        }
        else {
            msg[i - dp_found] = font[message[i] & 0xFF];
        }
    }
    if (dp_found) {
        msg[7] = font[message[8] & 0xFF]; // Clear the last character if we found a decimal point
    }
}

/********************************************************* */
// Implement the functions below.


void display_init_pins() {
    for(int i = 10; i < 21; i++){
        sio_hw->gpio_oe_set = (1 << i);
        sio_hw->gpio_clr = (1 << i);

        hw_write_masked(&pads_bank0_hw->io[i],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
            );
            // Zero all fields apart from fsel; we want this IO to do what the peripheral tells it.
            // This doesn't affect e.g. pullup/pulldown, as these are in pad controls.
        io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO;

            // Remove pad isolation now that the correct peripheral is in control of the pad
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
}

}

void display_isr() {
    hw_clear_bits(&timer1_hw->intr, 1u << 0);
    timer1_hw->intr |= (1 << 0); //originally was intr

    sio_hw->gpio_clr = (0b11111111111 << 10);
    sio_hw->gpio_set = (index << 18) | (msg[index] << 10);

    index = (index + 1) % 8;

    //make TIMER1 ALARM0 fire interrupt again in 3ms
    irq_set_exclusive_handler(TIMER1_IRQ_0, display_isr); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER1_IRQ_0, true);
    uint64_t target = timer1_hw->timerawl + 3000;
    timer1_hw->alarm[0] = (uint32_t) target;
}

void display_init_timer() {
    timer1_hw->inte |= 1u << 0;
    
    irq_set_exclusive_handler(TIMER1_IRQ_0, display_isr); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER1_IRQ_0, true);
    uint64_t target = timer1_hw->timerawl + 3000;
    timer1_hw->alarm[0] = (uint32_t) target;
}

void display_print(const uint16_t message[]) {
    for (int i = 0; i < 8; i++) {
        msg[i] = font[message[i] & 0xFF];

        uint16_t val = (message[i] >> 8) << 7;
        msg[i] |= val;
    }
}