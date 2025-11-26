#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

void audio_init(void);
void audio_play_tone(uint32_t freq_hz, uint32_t duration_ms);
void audio_play_shot(void);

#endif // AUDIO_H
