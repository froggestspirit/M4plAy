#include <stdint.h>

#ifndef CGB_AUDIO_H
#define CGB_AUDIO_H

struct AudioCGB{
    uint16_t ch1Freq;
    uint8_t ch1SweepCounter;
    uint8_t ch1SweepCounterI;
    uint8_t ch1SweepDir;
    uint8_t ch1SweepShift;
    uint8_t Vol[4];
    uint8_t VolI[4];
    uint8_t Len[4];
    uint8_t LenI[4];
    uint8_t LenOn[4];
    uint8_t EnvCounter[4];
    uint8_t EnvCounterI[4];
    uint8_t EnvDir[4];
    uint8_t DAC[4];
    float WAVRAM[32];
    uint16_t ch4LFSR [2];
};

void cgb_audio_init(uint32_t rate);
void cgb_set_sweep(uint8_t sweep);
void cgb_set_wavram();
void cgb_toggle_length(uint8_t channel, uint8_t state);
void cgb_set_length(uint8_t channel, uint8_t length);
void cgb_set_envelope(uint8_t channel, uint8_t envelope);
void cgb_trigger_note(uint8_t channel);
void cgb_audio_generate(uint16_t samplesPerFrame, float *outBuffer);

#endif
