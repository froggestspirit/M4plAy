#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include <stdint.h>
#include "m4a_internal.h"

float *RunMixerFrame(uint16_t samplesPerFrame);
static inline void GenerateAudio(struct SoundInfo *mixer, struct SoundChannel *chan, struct WaveData *wav, float *outBuffer, uint16_t samplesPerFrame, float sampleRateReciprocal);
void SampleMixer(struct SoundInfo *mixer, uint16_t samplesPerFrame, float *outBuffer, uint16_t maxBufSize);
static inline uint32_t TickEnvelope(struct SoundChannel *chan, struct WaveData *wav);
void cgb_audio_generate(uint16_t samplesPerFrame);
float *cgb_get_buffer();

#endif  // SOUND_MIXER_H
