#ifndef GUN_H
#define GUN_H

#include <stdbool.h>
void init_gun(int hit_pin, int trigger_pin);
void gun_isr();

#endif // GUN_H 