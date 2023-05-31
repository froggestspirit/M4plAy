#include <stddef.h>
#include <stdint.h>
#include "io_reg.h"
#include "music_player.h"
#include "sound_mixer.h"
#include "mp2k_common.h"
#include "cgb_audio.h"


#define VCOUNT_VBLANK 160
#define TOTAL_SCANLINES 228

extern struct SoundInfo *SOUND_INFO_PTR;

static inline void GenerateAudio(struct SoundMixerState *mixer, struct MixerSource *chan, struct WaveData2 *wav, float *outBuffer, uint16_t samplesPerFrame, float sampleRateReciprocal);
void SampleMixer(struct SoundMixerState *mixer, uint32_t scanlineLimit, uint16_t samplesPerFrame, float *outBuffer, uint8_t dmaCounter, uint16_t maxBufSize);
static inline uint32_t TickEnvelope(struct MixerSource *chan, struct WaveData2 *wav);
void GeneratePokemonSampleAudio(struct SoundMixerState *mixer, struct MixerSource *chan, int8_t *current, float *outBuffer, uint16_t samplesPerFrame, float sampleRateReciprocal, int32_t samplesLeftInWav, signed envR, signed envL);

void RunMixerFrame(void) {
    struct SoundMixerState *mixer = SOUND_INFO_PTR;
    
    if (mixer->lockStatus != MIXER_UNLOCKED) {
        return;
    }
    mixer->lockStatus = MIXER_LOCKED;
    
    uint32_t maxScanlines = mixer->maxScanlines;
    if (mixer->maxScanlines != 0) {
        uint32_t vcount = REG_VCOUNT;
        maxScanlines += vcount;
        if (vcount < VCOUNT_VBLANK) {
            maxScanlines += TOTAL_SCANLINES;
        }
    }
    
    if (mixer->firstPlayerFunc != NULL) {
        mixer->firstPlayerFunc(mixer->firstPlayer);
    }
    
    mixer->cgbMixerFunc();
    
    int32_t samplesPerFrame = mixer->samplesPerFrame;
    float *outBuffer = mixer->outBuffer;
    int32_t dmaCounter = mixer->dmaCounter;
    
    if (dmaCounter > 1) {
        outBuffer += samplesPerFrame * (mixer->framesPerDmaCycle - (dmaCounter - 1)) * 2;
    }
    
    //MixerRamFunc mixerRamFunc = ((MixerRamFunc)MixerCodeBuffer);
    SampleMixer(mixer, maxScanlines, samplesPerFrame, outBuffer, dmaCounter, MIXED_AUDIO_BUFFER_SIZE);

    cgb_audio_generate(samplesPerFrame);

}



//__attribute__((target("thumb")))
void SampleMixer(struct SoundMixerState *mixer, uint32_t scanlineLimit, uint16_t samplesPerFrame, float *outBuffer, uint8_t dmaCounter, uint16_t maxBufSize) {
    uint32_t reverb = mixer->reverb;
    if (reverb) {
        // The vanilla reverb effect outputs a mono sound from four sources:
        //  - L/R channels as they were mixer->framesPerDmaCycle frames ago
        //  - L/R channels as they were (mixer->framesPerDmaCycle - 1) frames ago
        float *tmp1 = outBuffer;
        float *tmp2;
        if (dmaCounter == 2) {
            tmp2 = mixer->outBuffer;
        } else {
            tmp2 = outBuffer + samplesPerFrame * 2;
        }
        uint_fast16_t i = 0;
        do {
            float s = tmp1[0] + tmp1[1] + tmp2[0] + tmp2[1];
            s *= ((float)reverb / 512.0f);
            tmp1[0] = tmp1[1] = s;
            tmp1+=2;
            tmp2+=2;
        }
        while(++i < samplesPerFrame);
    } else {
        // memset(outBuffer, 0, samplesPerFrame);
        // memset(outBuffer + maxBufSize, 0, samplesPerFrame);
        for (int i = 0; i < samplesPerFrame; i++) {
            float *dst = &outBuffer[i*2];
            dst[1] = dst[0] = 0.0f;
        }
    }
    
    float sampleRateReciprocal = mixer->sampleRateReciprocal;
    int_fast8_t numChans = mixer->numChans;
    struct MixerSource *chan = mixer->chans;
    
    for (int i = 0; i < numChans; i++, chan++) {
        struct WaveData2 *wav = chan->wav;
        
        if (scanlineLimit != 0) {
            uint_fast16_t vcount = REG_VCOUNT;
            if (vcount < VCOUNT_VBLANK) {
                vcount += TOTAL_SCANLINES;
            }
            if (vcount >= scanlineLimit) {
                goto returnEarly;
            }
        }
        
        if (TickEnvelope(chan, wav)) 
        {

            GenerateAudio(mixer, chan, wav, outBuffer, samplesPerFrame, sampleRateReciprocal);
        }
    }
returnEarly:
    mixer->lockStatus = MIXER_UNLOCKED;
}

// Returns 1 if channel is still active after moving envelope forward a frame
//__attribute__((target("thumb")))
static inline uint32_t TickEnvelope(struct MixerSource *chan, struct WaveData2 *wav) {
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
        return 0;
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
                return 0;
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
                    return 0;
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
        case 1: // Sustain
        default:
            break;
        }
        
        return 1;
    } else if (status & 0x40) {
        // Init and stop cancel each other out
        chan->status = 0;
        return 0;
    } else {
        // Init channel
        chan->status = 3;
#ifdef POKEMON_EXTENSIONS
        chan->current = wav->data + chan->ct;
        chan->ct = wav->size - chan->ct;
#else
        chan->current = wav->data;
        chan->ct = wav->size;
#endif
        chan->fw = 0;
        chan->envelopeVol = 0;
        if (wav->loopFlags & 0xC0) {
            chan->status |= 0x10;
        }
        goto attack;
    }
}

//__attribute__((target("thumb")))
static inline void GenerateAudio(struct SoundMixerState *mixer, struct MixerSource *chan, struct WaveData2 *wav, float *outBuffer, uint16_t samplesPerFrame, float sampleRateReciprocal) {/*, [[[]]]) {*/
    uint_fast8_t v = chan->envelopeVol * (mixer->masterVol + 1) / 16U;
    chan->envelopeVolR = chan->rightVol * v / 256U;
    chan->envelopeVolL = chan->leftVol * v / 256U;
    
    int32_t loopLen = 0;
    int8_t *loopStart;
    if (chan->status & 0x10) {
        loopStart = wav->data + wav->loopStart;
        loopLen = wav->size - wav->loopStart;
    }
    int32_t samplesLeftInWav = chan->ct;
    int8_t *current = chan->current;
    signed envR = chan->envelopeVolR;
    signed envL = chan->envelopeVolL;
    /*if (chan->type & 8) {
        for (uint16_t i = 0; i < samplesPerFrame; i++, outBuffer+=2) {
            int_fast8_t c = *(current++);
            
            outBuffer[1] += (c * envR) / 32768.0f;
            outBuffer[0] += (c * envL) / 32768.0f;
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
        
        chan->ct = samplesLeftInWav;
        chan->current = current;
    } else {*/
    float finePos = chan->fw;
    float romSamplesPerOutputSample = chan->freq * sampleRateReciprocal;

    int_fast16_t b = current[0];
    int_fast16_t m = current[1] - b;
    current += 1;
    
    for (uint16_t i = 0; i < samplesPerFrame; i++, outBuffer+=2) {
        // Use linear interpolation to calculate a value between the current sample in the wav
        // and the next sample. Also cancel out the 9.23 stuff
        float sample = (finePos * m) + b;
        
        outBuffer[1] += (sample * envR) / 32768.0f;
        outBuffer[0] += (sample * envL) / 32768.0f;
        
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
    chan->ct = samplesLeftInWav;
    chan->current = current - 1;
    //}
}

struct WaveData
{
    uint16_t type;
    uint16_t status;
    uint32_t freq;
    uint32_t loopStart;
    uint32_t size; // number of samples
    int8_t data[1]; // samples
};

void GeneratePokemonSampleAudio(struct SoundMixerState *mixer, struct MixerSource *chan, int8_t *current, float *outBuffer, uint16_t samplesPerFrame, float sampleRateReciprocal, int32_t samplesLeftInWav, signed envR, signed envL) {
    struct WaveData *wav = chan->wav; // r6
    float finePos = chan->fw;
    if((chan->status & 0x20) == 0) {
        chan->status |= 0x20;
        if(chan->type & 0x10) {
            int8_t *waveEnd = wav->data + wav->size;
            current = wav->data + (waveEnd - current);
            chan->current = current;
        }
        if(wav->type != 0) {
            current -= (uintptr_t)&wav->data;
            chan->current = current;
        }
    }
    float romSamplesPerOutputSample = chan->type & 8 ? 1.0f : chan->freq * sampleRateReciprocal;
    if(wav->type != 0) { // is compressed
        chan->extra1 = 0;
        chan->extra2 = 0xFF00;
        if(chan->type & 0x20) {

        }
        else {

        } 
        //TODO: implement compression
    }
    else {
        if(chan->type & 0x10) {
            current -= 1;
            int_fast16_t b = current[0];
            int_fast16_t m = current[-1] - b;
            
            for (uint16_t i = 0; i < samplesPerFrame; i++, outBuffer+=2) {
                float sample = (finePos * m) + b;
                
                outBuffer[1] += (sample * envR) / 32768.0f;
                outBuffer[0] += (sample * envL) / 32768.0f;
                
                finePos += romSamplesPerOutputSample;
                int newCoarsePos = finePos;
                if (newCoarsePos != 0) {
                    finePos -= (int)finePos;
                    samplesLeftInWav -= newCoarsePos;
                    if (samplesLeftInWav <= 0) {
                        chan->status = 0;
                        break;
                    }
                    else {
                        current -= newCoarsePos;
                        b = current[0];
                        m = current[-1] - b;
                    }
                }
            }
            
            chan->fw = finePos;
            chan->ct = samplesLeftInWav;
            chan->current = current + 1;
        }
    }
}
