#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "m4a_internal.h"

static inline void GenerateAudio(struct SoundInfo *mixer, struct SoundChannel *chan, struct WaveData *wav, float *pcmBuffer, uint16_t samplesPerFrame, float divFreq);
void SampleMixer(struct SoundInfo *mixer, uint16_t samplesPerFrame, float *pcmBuffer, uint16_t maxBufSize);
static inline uint32_t TickEnvelope(struct SoundChannel *chan, struct WaveData *wav);
void cgb_audio_generate(uint16_t samplesPerFrame);
float *cgb_get_buffer();

#endif  // SOUND_MIXER_H
