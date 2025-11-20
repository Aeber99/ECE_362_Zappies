#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"

// Global column variable
int col = -1;

// Global key state
static bool state[16]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[17] = "DCBA#9630852*741";

void keypad_drive_column();
void keypad_isr();

/********************************************************* */
// Implement the functions below.

void keypad_init_pins() {
                    for(int i = 2; i < 6; i++){
        sio_hw->gpio_oe_clr = (1 << i);

        hw_write_masked(&pads_bank0_hw->io[i],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
            );
            // Zero all fields apart from fsel; we want this IO to do what the peripheral tells it.
            // This doesn't affect e.g. pullup/pulldown, as these are in pad controls.
        io_bank0_hw->io[i].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;

            // Remove pad isolation now that the correct peripheral is in control of the pad
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
    }

    for(int i = 6; i < 10; i++){
        sio_hw->gpio_oe_set = (1 << i);
        sio_hw->gpio_clr = (1 << i);

        hw_write_masked(&pads_bank0_hw->io[i],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
            );
            // Zero all fields apart from fsel; we want this IO to do what the peripheral tells it.
            // This doesn't affect e.g. pullup/pulldown, as these are in pad controls.
        io_bank0_hw->io[i].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;

            // Remove pad isolation now that the correct peripheral is in control of the pad
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);


    }    

}

void keypad_init_timer() {
        timer_hw->inte |= 1u << 0;
    
    irq_set_exclusive_handler(TIMER0_IRQ_0, keypad_drive_column); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER0_IRQ_0, true);
    uint64_t target = timer_hw->timerawl + 1000000;
    timer_hw->alarm[0] = (uint32_t) target;

    timer_hw->inte |= 1u << 1;
    //initialize TIMER0 for alarm1 after 1.10s to call keypad_isr
    irq_set_exclusive_handler(TIMER0_IRQ_1, keypad_isr); //set irq handler for alarm irq (needs ALARM0??)
    //enable alarm irq
    irq_set_enabled(TIMER0_IRQ_1, true); 
    uint64_t target2 = timer_hw->timerawl + 1100000;
    timer_hw->alarm[1] = (uint32_t) target2;

    nvic_hw->iser[0] |= 0x3;

}

void keypad_drive_column() {
        hw_clear_bits(&timer_hw->intr, 1u << 0);
    timer_hw->intr |= (1 << 0);
    col = (col + 1) % 4;


    u_int32_t full_mask = (0xf << 6) & sio_hw->gpio_out;
    u_int32_t col_mask = 0x1 << (col + 6); //starts at 0

    sio_hw->gpio_togl = full_mask ^ col_mask;

    uint64_t target = timer_hw->timerawl + 25000;
    timer_hw->alarm[0] = (uint32_t) target;

}

uint8_t keypad_read_rows() {
    return ((sio_hw->gpio_in >> 2) & 0xF);

}

void keypad_isr() {
        hw_clear_bits(&timer_hw->intr, 1u << 1);
    timer_hw->intr |= (1u << 1);
    
    uint8_t current_row_pins = keypad_read_rows();

    //is it a loop or a mask that indicates which bits are high?


    //for the first check
    for(int i = 0; i < 4; i++){
        if((current_row_pins & (1 << i)) && !(state[(4 * col) + i])){ //checking state and if high using bit shift mask
            state[(col * 4) + (i)] = 1; //setting high
            //printf("%c\n", key); //NOTE THIS IS USED FOR TESTING STEP 3

            key_push(keymap[(4*col) + (i)] | (1 << 8)); //moves 9 spots
        }

        else if(((current_row_pins & (1 << i)) == 0) && (state[(4 * col) + (i)])){
            state[(col * 4) + (i)] = 0; //setting low
            
            key_push(keymap[(4*col) + (i)] | (0 << 8));

        }

        uint64_t target = timer_hw->timerawl + 25000;
        timer_hw->alarm[1] = (uint32_t) target;

    }
}
