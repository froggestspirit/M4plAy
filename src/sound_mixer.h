#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "music_player.h"

#define MIXER_UNLOCKED 0x68736D53
#define MIXER_LOCKED PLAYER_UNLOCKED+1

typedef void (*MixerRamFunc)(struct SoundMixerState *, uint32_t, uint16_t, int8_t *, uint16_t);

struct MixerSource {
    uint8_t status;
    uint8_t type;
    uint8_t rightVol;
    uint8_t leftVol;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t key;
    uint8_t envelopeVol;
    union {
        uint8_t envelopeVolR;
        uint8_t envelopeGoal;
    }__attribute__((packed));
    union {
        uint8_t envelopeVolL;
        uint8_t envelopeCtr;
    }__attribute__((packed));
    uint8_t echoVol;
    uint8_t echoLen;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t gateTime;
    uint8_t untransposedKey;
    uint8_t velocity;
    uint8_t priority;
    uint8_t rhythmPan;
    uint8_t padding3;
    uint8_t padding4;
    uint8_t padding5;
    union {
        uint32_t ct;
        struct {
            uint8_t padding6;
            uint8_t sustainGoal;
            uint8_t nrx4;
            uint8_t pan;
        };
    };
    union {
        float fw;
        struct {
            uint8_t panMask;
            uint8_t cgbStatus;
            uint8_t length;
            uint8_t sweep;
        };
    };
    uint32_t freq;
    union {
        uint32_t *newCgb3Sample;
        struct WaveData *wav;
    };
    union {
        uint32_t *oldCgb3Sample;
        int8_t *current;
    };
    struct MP2KTrack *track;
    struct MixerSource *prev;
    struct MixerSource *next;
    uint32_t padding7; //d4
    uint16_t extra1;
    uint16_t extra2;
};

enum { MAX_SAMPLE_CHANNELS = 12 };
enum { MIXED_AUDIO_BUFFER_SIZE = 4907 };

struct SoundMixerState {
    volatile uint32_t lockStatus;
    volatile uint8_t dmaCounter;
    uint8_t reverb;
    uint8_t numChans;
    uint8_t masterVol;
    uint8_t freqOption;
    uint8_t extensionFlags;
    uint8_t cgbCounter15;
    uint8_t framesPerDmaCycle;
    uint8_t maxScanlines;
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int32_t samplesPerFrame;
    int32_t sampleRate;
    float sampleRateReciprocal;
    struct MixerSource *cgbChans;
    void (*firstPlayerFunc)(void *player);
    void *firstPlayer;
    void (*cgbMixerFunc)(void);
    void (*cgbNoteOffFunc)(uint8_t chan);
    uint32_t (*cgbCalcFreqFunc)(uint8_t chan, uint8_t key, uint8_t pitch);
    void (**mp2kEventFuncTable)();
    void (*mp2kEventNxxFunc)(uint8_t clock, struct MP2KPlayerState *player, struct MP2KTrack *track);
    void *reserved1; // In poke* this is called "ExtVolPit"
    void *reserved2;
    void *reserved3;
    void *reversed4;
    void *reserved5;
    struct MixerSource chans[MAX_SAMPLE_CHANNELS];
    __attribute__((aligned(4))) float outBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
    //int8_t outBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
};

#endif//SOUND_MIXER_H
