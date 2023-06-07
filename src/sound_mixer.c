#include <stddef.h>
#include <stdint.h>
#include "mp2k_common.h"
#include "cgb_audio.h"
#include "m4a_internal.h"

#define VCOUNT_VBLANK 160
#define TOTAL_SCANLINES 228

extern struct SoundMixerState *SOUND_INFO_PTR;

static inline void GenerateAudio(struct SoundMixerState *mixer, struct SoundChannel *chan, struct WaveData *wav, float *outBuffer, uint16_t samplesPerFrame, float divFreq);
void SampleMixer(struct SoundMixerState *mixer, uint32_t scanlineLimit, uint16_t samplesPerFrame, float *outBuffer, uint8_t dmaCounter, uint16_t maxBufSize);
static inline uint32_t TickEnvelope(struct SoundChannel *chan, struct WaveData *wav);


uint8_t RunMixerFrame(void *audioBuffer, int32_t samplesPerFrame) {
    struct SoundMixerState *mixer = SOUND_INFO_PTR;

    static float playerCounter = 0;
    
    playerCounter += samplesPerFrame;
    while (playerCounter >= mixer->samplesPerFrame) {
        playerCounter -= mixer->samplesPerFrame;
        uint32_t maxScanlines = mixer->maxScanlines;
        
        if (mixer->firstPlayerFunc != NULL) {
            mixer->firstPlayerFunc(mixer->firstPlayer);
        }
        
        mixer->cgbMixerFunc();
    }
    samplesPerFrame = mixer->samplesPerFrame;
    float *outBuffer = mixer->outBuffer;
    float *cgbBuffer = mixer->cgbBuffer;
    
    int32_t dmaCounter = mixer->dmaCounter;
    
    if (dmaCounter > 1) {
        outBuffer += samplesPerFrame * (mixer->pcmDmaPeriod - (dmaCounter - 1)) * 2;
    }
    
    //MixerRamFunc mixerRamFunc = ((MixerRamFunc)MixerCodeBuffer);
    SampleMixer(mixer, 0, samplesPerFrame, outBuffer, dmaCounter, PCM_DMA_BUF_SIZE);

    cgb_audio_generate(samplesPerFrame, cgbBuffer);

    //struct SoundMixerState *mixer = SOUND_INFO_PTR;
    samplesPerFrame = mixer->samplesPerFrame * 2;
    float *m4aBuffer = mixer->outBuffer;
    cgbBuffer = mixer->cgbBuffer;

    if (dmaCounter > 1) {
        m4aBuffer += samplesPerFrame * (mixer->pcmDmaPeriod - (dmaCounter - 1));
    }

    float *outBuf = audioBuffer;
    for(uint32_t i = 0; i < samplesPerFrame; i++)
        outBuf[i] = m4aBuffer[i] + cgbBuffer[i];

    if((int8_t)(--mixer->dmaCounter) <= 0)
        mixer->dmaCounter = mixer->pcmDmaPeriod;
    
    return 1;
}



//__attribute__((target("thumb")))
void SampleMixer(struct SoundMixerState *mixer, uint32_t scanlineLimit, uint16_t samplesPerFrame, float *outBuffer, uint8_t dmaCounter, uint16_t maxBufSize) {
    uint32_t reverb = mixer->reverb;
    if (reverb) {
        // The vanilla reverb effect outputs a mono sound from four sources:
        //  - L/R channels as they were mixer->pcmDmaPeriod frames ago
        //  - L/R channels as they were (mixer->pcmDmaPeriod - 1) frames ago
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
    
    float divFreq = mixer->divFreq;
    int_fast8_t numChans = mixer->numChans;
    struct SoundChannel *chan = mixer->chans;
    
    for (int i = 0; i < numChans; i++, chan++) {
        struct WaveData *wav = chan->wav;
        
        if (TickEnvelope(chan, wav)) 
        {

            GenerateAudio(mixer, chan, wav, outBuffer, samplesPerFrame, divFreq);
        }
    }
}

// Returns 1 if channel is still active after moving envelope forward a frame
//__attribute__((target("thumb")))
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
    
    uint8_t status = chan->statusFlags;
    if ((status & 0xC7) == 0) {
        return 0;
    }
    
    uint8_t env = 0;
    if ((status & 0x80) == 0) {
        env = chan->envelopeVolume;
        
        if (status & 4) {
            // Note-wise echo
            --chan->echoVolume;
            if (chan->echoVolume <= 0) {
                chan->statusFlags = 0;
                return 0;
            } else {
                return 1;
            }
        } else if (status & 0x40) {
            // Release
            chan->envelopeVolume = env * chan->release / 256U;
            uint8_t echoVolume = chan->echoVolume;
            if (chan->envelopeVolume > echoVolume) {
                return 1;
            } else if (echoVolume == 0) {
                chan->statusFlags = 0;
                return 0;
            } else {
                chan->statusFlags |= 4;
                return 1;
            }
        }
        
        switch (status & 3) {
        uint_fast16_t newEnv;
        case 2:
            // Decay
            chan->envelopeVolume = env * chan->decay / 256U;
            
            uint8_t sustain = chan->sustain;
            if (chan->envelopeVolume <= sustain && sustain == 0) {
                // Duplicated echo check from Release section above
                if (chan->echoVolume == 0) {
                    chan->statusFlags = 0;
                    return 0;
                } else {
                    chan->statusFlags |= 4;
                    return 1;
                }
            } else if (chan->envelopeVolume <= sustain) {
                chan->envelopeVolume = sustain;
                --chan->statusFlags;
            }
            break;
        case 3:
        attack:
            newEnv = env + chan->attack;
            if (newEnv > 0xFF) {
                chan->envelopeVolume = 0xFF;
                --chan->statusFlags;
            } else {
                chan->envelopeVolume = newEnv;
            }
            break;
        case 1: // Sustain
        default:
            break;
        }
        
        return 1;
    } else if (status & 0x40) {
        // Init and stop cancel each other out
        chan->statusFlags = 0;
        return 0;
    } else {
        // Init channel
        chan->statusFlags = 3;
        chan->currentPointer = wav->data + chan->count;
        chan->count = wav->size - chan->count;
        chan->fw = 0;
        chan->envelopeVolume = 0;
        if (wav->loopFlags & 0xC0) {
            chan->statusFlags |= 0x10;
        }
        goto attack;
    }
}

//__attribute__((target("thumb")))
static inline void GenerateAudio(struct SoundMixerState *mixer, struct SoundChannel *chan, struct WaveData *wav, float *outBuffer, uint16_t samplesPerFrame, float divFreq) {
    uint_fast8_t v = chan->envelopeVolume * (mixer->masterVol + 1) / 16U;
    chan->envelopeVolumeRight = chan->rightVolume * v / 256U;
    chan->envelopeVolumeLeft = chan->leftVolume * v / 256U;
    
    int32_t loopLen = 0;
    int8_t *loopStart;
    if (chan->statusFlags & 0x10) {
        loopStart = wav->data + wav->loopStart;
        loopLen = wav->size - wav->loopStart;
    }
    int32_t samplesLeftInWav = chan->count;
    int8_t *currentPointer = chan->currentPointer;
    signed envR = chan->envelopeVolumeRight;
    signed envL = chan->envelopeVolumeLeft;
    /*if (chan->type & 8) {
        for (uint16_t i = 0; i < samplesPerFrame; i++, outBuffer+=2) {
            int_fast8_t c = *(currentPointer++);
            
            outBuffer[1] += (c * envR) / 32768.0f;
            outBuffer[0] += (c * envL) / 32768.0f;
            if (--samplesLeftInWav == 0) {
                samplesLeftInWav = loopLen;
                if (loopLen != 0) {
                    currentPointer = loopStart;
                } else {
                    chan->statusFlags = 0;
                    return;
                }
            }
        }
        
        chan->count = samplesLeftInWav;
        chan->currentPointer = currentPointer;
    } else {*/
    float finePos = chan->fw;
    float romSamplesPerOutputSample = chan->freq * divFreq;

    int_fast16_t b = currentPointer[0];
    int_fast16_t m = currentPointer[1] - b;
    currentPointer += 1;
    
    for (uint16_t i = 0; i < samplesPerFrame; i++, outBuffer+=2) {
        // Use linear interpolation to calculate a value between the currentPointer sample in the wav
        // and the nextChannelPointer sample. Also cancel out the 9.23 stuff
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
                    currentPointer = loopStart;
                    newCoarsePos = -samplesLeftInWav;
                    samplesLeftInWav += loopLen;
                    while (samplesLeftInWav <= 0) {
                        newCoarsePos -= loopLen;
                        samplesLeftInWav += loopLen;
                    }
                    b = currentPointer[newCoarsePos];
                    m = currentPointer[newCoarsePos + 1] - b;
                    currentPointer += newCoarsePos + 1;
                } else {
                    chan->statusFlags = 0;
                    return;
                }
            } else {
                b = currentPointer[newCoarsePos - 1];
                m = currentPointer[newCoarsePos] - b;
                currentPointer += newCoarsePos;
            }
        }
    }
    
    chan->fw = finePos;
    chan->count = samplesLeftInWav;
    chan->currentPointer = currentPointer - 1;
    //}
}
