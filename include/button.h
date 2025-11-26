#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

void init_inputs(void);
bool button_pressed(void);
void wait_for_button_press(void);


#endif // BUTTON_H
