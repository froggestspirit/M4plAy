#include <stdbool.h>
#include <stddef.h>

#include "m4a_internal.h"
#include "sound_mixer.h"

extern struct SoundInfo *SOUND_INFO_PTR;
extern struct AudioCGB gb;
extern float soundChannelPos[4];
extern const int16_t *PU1Table;
extern const int16_t *PU2Table;
extern uint32_t apuFrame;
extern uint8_t apuCycle;
extern uint32_t sampleRate;
extern uint16_t lfsrMax[2];
extern float ch4Samples;
extern struct MusicPlayerInfo gMPlayInfo_BGM;

float audioBuffer[MIXED_AUDIO_BUFFER_SIZE];

float *RunMixerFrame(uint16_t samplesPerFrame) {
    struct SoundInfo *mixer = SOUND_INFO_PTR;

    static float playerCounter = 0;

    playerCounter += samplesPerFrame;
    while (playerCounter >= mixer->updateRate) {
        MP2KPlayerMain(&gMPlayInfo_BGM);
        playerCounter -= mixer->updateRate;
    }

    float *m4aBuffer = mixer->pcmBuffer;

    SampleMixer(mixer, samplesPerFrame, m4aBuffer, MIXED_AUDIO_BUFFER_SIZE);
    cgb_audio_generate(samplesPerFrame);

    samplesPerFrame = mixer->samplesPerFrame * 2;
    float *cgbBuffer = cgb_get_buffer();

    for (uint32_t i = 0; i < samplesPerFrame; i++)
        audioBuffer[i] = m4aBuffer[i] + cgbBuffer[i];
    // printf("spf: %d\n", samplesPerFrame * 4);
    //  SDL_QueueAudio(1, audioBuffer, samplesPerFrame * 4);
    return audioBuffer;
}

void SampleMixer(struct SoundInfo *mixer, uint16_t samplesPerFrame, float *pcmBuffer, uint16_t maxBufSize) {
    uint32_t reverb = mixer->reverb;
    if (reverb) {
        // The vanilla reverb effect outputs a mono sound from four sources:
        //  - L/R channels as they were mixer->framesPerDmaCycle frames ago
        //  - L/R channels as they were (mixer->framesPerDmaCycle - 1) frames ago
        float *tmp1 = pcmBuffer;
        float *tmp2;
        tmp2 = pcmBuffer + samplesPerFrame * 2;
        uint_fast16_t i = 0;
        do {
            float s = tmp1[0] + tmp1[1] + tmp2[0] + tmp2[1];
            s *= ((float)reverb / 512.0f);
            tmp1[0] = tmp1[1] = s;
            tmp1 += 2;
            tmp2 += 2;
        } while (++i < samplesPerFrame);
    } else {
        for (int i = 0; i < samplesPerFrame; i++) {
            float *dst = &pcmBuffer[i * 2];
            dst[1] = dst[0] = 0.0f;
        }
    }

    float divFreq = mixer->divFreq;
    int_fast8_t numChans = mixer->numChans;
    struct SoundChannel *chan = mixer->chans;

    for (int i = 0; i < numChans; i++, chan++) {
        struct WaveData *wav = chan->wav;

        if (TickEnvelope(chan, wav)) {
            GenerateAudio(mixer, chan, wav, pcmBuffer, samplesPerFrame, divFreq);
        }
    }
}

// Returns 1 if channel is still active after moving envelope forward a frame

static inline uint32_t TickEnvelope(struct SoundChannel *chan, struct WaveData *wav) {
    // MP2K envelope shape
    //                                                                 |
    // (linear)^                                                       |
    // Attack / \Decay (exponential)                                   |
    //       /   \_                                                    |
    //      /      '.,        Sustain                                  |
    //     /          '.______________                                 |
    //    /                           '-.       Echo (linear)          |
    //   /                 Release (exp) ''--..|\                      |
    //  /                                        \                     |

    uint8_t status = chan->status;
    if ((status & 0xC7) == 0) {
        return false;
    }

    uint8_t env = 0;
    if ((status & 0x80) == 0) {
        env = chan->envelopeVol;

        if (status & 4) {
            // Note-wise echo
            --chan->echoVol;
            if (chan->echoVol <= 0) {
                chan->status = 0;
                return 0;
            } else {
                return 1;
            }
        } else if (status & 0x40) {
            // Release
            chan->envelopeVol = env * chan->release / 256U;
            uint8_t echoVol = chan->echoVol;
            if (chan->envelopeVol > echoVol) {
                return 1;
            } else if (echoVol == 0) {
                chan->status = 0;
                return false;
            } else {
                chan->status |= 4;
                return 1;
            }
        }

        switch (status & 3) {
            uint_fast16_t newEnv;
            case 2:
                // Decay
                chan->envelopeVol = env * chan->decay / 256U;

                uint8_t sustain = chan->sustain;
                if (chan->envelopeVol <= sustain && sustain == 0) {
                    // Duplicated echo check from Release section above
                    if (chan->echoVol == 0) {
                        chan->status = 0;
                        return false;
                    } else {
                        chan->status |= 4;
                        return 1;
                    }
                } else if (chan->envelopeVol <= sustain) {
                    chan->envelopeVol = sustain;
                    --chan->status;
                }
                break;
            case 3:
            attack:
                newEnv = env + chan->attack;
                if (newEnv > 0xFF) {
                    chan->envelopeVol = 0xFF;
                    --chan->status;
                } else {
                    chan->envelopeVol = newEnv;
                }
                break;
            case 1:  // Sustain
            default:
                break;
        }

        return 1;
    } else if (status & 0x40) {
        // Init and stop cancel each other out
        chan->status = 0;
        return false;
    } else {
        // Init channel
        chan->status = 3;
        chan->current = wav->data;
        chan->count = wav->size;
        chan->fw = 0;
        chan->envelopeVol = 0;
        if (wav->loopFlags & 0xC0) {
            chan->status |= 0x10;
        }
        goto attack;
    }
}

static inline void GenerateAudio(struct SoundInfo *mixer, struct SoundChannel *chan, struct WaveData *wav, float *pcmBuffer, uint16_t samplesPerFrame, float divFreq) { /*, [[[]]]) {*/
    uint_fast8_t v = chan->envelopeVol * (mixer->masterVol + 1) / 16U;
    chan->envelopeVolR = chan->rightVol * v / 256U;
    chan->envelopeVolL = chan->leftVol * v / 256U;

    int32_t loopLen = 0;
    int8_t *loopStart;
    if (chan->status & 0x10) {
        loopStart = wav->data + wav->loopStart;
        loopLen = wav->size - wav->loopStart;
    }
    int32_t samplesLeftInWav = chan->count;
    int8_t *current = chan->current;
    signed envR = chan->envelopeVolR;
    signed envL = chan->envelopeVolL;
    if (chan->type & 8) {
        for (uint16_t i = 0; i < samplesPerFrame; i++, pcmBuffer += 2) {
            int_fast8_t c = *(current++);

            pcmBuffer[1] += (c * envR) / 32768.0f;
            pcmBuffer[0] += (c * envL) / 32768.0f;
            if (--samplesLeftInWav == 0) {
                samplesLeftInWav = loopLen;
                if (loopLen != 0) {
                    current = loopStart;
                } else {
                    chan->status = 0;
                    return;
                }
            }
        }

        chan->count = samplesLeftInWav;
        chan->current = current;
    } else {
        float finePos = chan->fw;
        float romSamplesPerOutputSample = chan->freq * divFreq;
        int_fast16_t b = current[0];
        int_fast16_t m = current[1] - b;
        current += 1;

        for (uint16_t i = 0; i < samplesPerFrame; i++, pcmBuffer += 2) {
            // Use linear interpolation to calculate a value between the current sample in the wav
            // and the next sample. Also cancel out the 9.23 stuff
            float sample = (finePos * m) + b;

            pcmBuffer[1] += (sample * envR) / 32768.0f;
            pcmBuffer[0] += (sample * envL) / 32768.0f;

            finePos += romSamplesPerOutputSample;
            uint32_t newCoarsePos = finePos;
            if (newCoarsePos != 0) {
                finePos -= (int)finePos;
                samplesLeftInWav -= newCoarsePos;
                if (samplesLeftInWav <= 0) {
                    if (loopLen != 0) {
                        current = loopStart;
                        newCoarsePos = -samplesLeftInWav;
                        samplesLeftInWav += loopLen;
                        while (samplesLeftInWav <= 0) {
                            newCoarsePos -= loopLen;
                            samplesLeftInWav += loopLen;
                        }
                        b = current[newCoarsePos];
                        m = current[newCoarsePos + 1] - b;
                        current += newCoarsePos + 1;
                    } else {
                        chan->status = 0;
                        return;
                    }
                } else {
                    b = current[newCoarsePos - 1];
                    m = current[newCoarsePos] - b;
                    current += newCoarsePos;
                }
            }
        }

        chan->fw = finePos;
        chan->count = samplesLeftInWav;
        chan->current = current - 1;
    }
}

void cgb_audio_generate(uint16_t samplesPerFrame) {
    float *pcmBuffer = gb.pcmBuffer;
    switch (REG_NR11 & 0xC0) {
        case 0x00:
            PU1Table = PU0;
            break;
        case 0x40:
            PU1Table = PU1;
            break;
        case 0x80:
            PU1Table = PU2;
            break;
        case 0xC0:
            PU1Table = PU3;
            break;
    }

    switch (REG_NR21 & 0xC0) {
        case 0x00:
            PU2Table = PU0;
            break;
        case 0x40:
            PU2Table = PU1;
            break;
        case 0x80:
            PU2Table = PU2;
            break;
        case 0xC0:
            PU2Table = PU3;
            break;
    }

    for (uint16_t i = 0; i < samplesPerFrame; i++, pcmBuffer += 2) {
        apuFrame += 512;
        if (apuFrame >= sampleRate) {
            apuFrame -= sampleRate;
            apuCycle++;

            if ((apuCycle & 1) == 0) {  // Length
                for (uint8_t ch = 0; ch < 4; ch++) {
                    if (gb.Len[ch]) {
                        if (--gb.Len[ch] == 0 && gb.LenOn[ch]) {
                            REG_NR52 &= (0xFF ^ (1 << ch));
                        }
                    }
                }
            }

            if ((apuCycle & 7) == 7) {  // Envelope
                for (uint8_t ch = 0; ch < 4; ch++) {
                    if (ch == 2) continue;  // Skip wave channel
                    if (gb.EnvCounter[ch]) {
                        if (--gb.EnvCounter[ch] == 0) {
                            if (gb.Vol[ch] && !gb.EnvDir[ch]) {
                                gb.Vol[ch]--;
                                gb.EnvCounter[ch] = gb.EnvCounterI[ch];
                            } else if (gb.Vol[ch] < 0x0F && gb.EnvDir[ch]) {
                                gb.Vol[ch]++;
                                gb.EnvCounter[ch] = gb.EnvCounterI[ch];
                            }
                        }
                    }
                }
            }

            if ((apuCycle & 3) == 2) {  // Sweep
                if (gb.ch1SweepCounterI && gb.ch1SweepShift) {
                    if (--gb.ch1SweepCounter == 0) {
                        gb.ch1Freq = REG_SOUND1CNT_X & 0x7FF;
                        if (gb.ch1SweepDir) {
                            gb.ch1Freq -= gb.ch1Freq >> gb.ch1SweepShift;
                            if (gb.ch1Freq & 0xF800) gb.ch1Freq = 0;
                        } else {
                            gb.ch1Freq += gb.ch1Freq >> gb.ch1SweepShift;
                            if (gb.ch1Freq & 0xF800) {
                                gb.ch1Freq = 0;
                                gb.EnvCounter[0] = 0;
                                gb.Vol[0] = 0;
                            }
                        }
                        REG_NR13 = gb.ch1Freq & 0xFF;
                        REG_NR14 &= 0xF8;
                        REG_NR14 += (gb.ch1Freq >> 8) & 0x07;
                        gb.ch1SweepCounter = gb.ch1SweepCounterI;
                    }
                }
            }
        }
        // Sound generation loop
        soundChannelPos[0] += freqTable[REG_SOUND1CNT_X & 0x7FF] / (sampleRate / 32);
        soundChannelPos[1] += freqTable[REG_SOUND2CNT_H & 0x7FF] / (sampleRate / 32);
        soundChannelPos[2] += freqTable[REG_SOUND3CNT_X & 0x7FF] / (sampleRate / 32);
        while (soundChannelPos[0] >= 32) soundChannelPos[0] -= 32;
        while (soundChannelPos[1] >= 32) soundChannelPos[1] -= 32;
        while (soundChannelPos[2] >= 32) soundChannelPos[2] -= 32;
        float outputL = 0;
        float outputR = 0;
        if (REG_NR52 & 0x80) {
            if ((gb.DAC[0]) && (REG_NR52 & 0x01)) {
                if (REG_NR51 & 0x10) outputL += gb.Vol[0] * PU1Table[(int)(soundChannelPos[0])] / 15.0f;
                if (REG_NR51 & 0x01) outputR += gb.Vol[0] * PU1Table[(int)(soundChannelPos[0])] / 15.0f;
            }
            if ((gb.DAC[1]) && (REG_NR52 & 0x02)) {
                if (REG_NR51 & 0x20) outputL += gb.Vol[1] * PU2Table[(int)(soundChannelPos[1])] / 15.0f;
                if (REG_NR51 & 0x02) outputR += gb.Vol[1] * PU2Table[(int)(soundChannelPos[1])] / 15.0f;
            }
            if ((REG_NR30 & 0x80) && (REG_NR52 & 0x04)) {
                if (REG_NR51 & 0x40) outputL += gb.Vol[2] * gb.WAVRAM[(int)(soundChannelPos[2])] / 4.0f;
                if (REG_NR51 & 0x04) outputR += gb.Vol[2] * gb.WAVRAM[(int)(soundChannelPos[2])] / 4.0f;
            }
            if ((gb.DAC[3]) && (REG_NR52 & 0x08)) {
                uint32_t lfsrMode = !!(REG_NR43 & 0x08);
                ch4Samples += freqTableNSE[REG_SOUND4CNT_H & 0xFF] / sampleRate;
                int ch4Out = 0;
                if (gb.ch4LFSR[lfsrMode] & 1) {
                    ch4Out++;
                } else {
                    ch4Out--;
                }
                int avgDiv = 1;
                while (ch4Samples >= 1) {
                    avgDiv++;
                    uint8_t lfsrCarry = 0;
                    if (gb.ch4LFSR[lfsrMode] & 2) lfsrCarry ^= 1;
                    gb.ch4LFSR[lfsrMode] >>= 1;
                    if (gb.ch4LFSR[lfsrMode] & 2) lfsrCarry ^= 1;
                    if (lfsrCarry) gb.ch4LFSR[lfsrMode] |= lfsrMax[lfsrMode];
                    if (gb.ch4LFSR[lfsrMode] & 1) {
                        ch4Out++;
                    } else {
                        ch4Out--;
                    }
                    ch4Samples--;
                }
                float sample = ch4Out;
                if (avgDiv > 1) sample /= avgDiv;
                if (REG_NR51 & 0x80) outputL += (gb.Vol[3] * sample) / 15.0f;
                if (REG_NR51 & 0x08) outputR += (gb.Vol[3] * sample) / 15.0f;
            }
        }
        pcmBuffer[0] = outputL / 4.0f;
        pcmBuffer[1] = outputR / 4.0f;
    }
}

float *cgb_get_buffer() {
    return gb.pcmBuffer;
}
