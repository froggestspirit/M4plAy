#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <stdint.h>
#include "sound_mixer.h"

#define PLAYER_UNLOCKED 0x68736D53
#define PLAYER_LOCKED PLAYER_UNLOCKED+1

struct WaveData2
{
    uint8_t compressionFlags1;
    uint8_t compressionFlags2;
    uint8_t compressionFlags3;
    uint8_t loopFlags;
    uint32_t freq;  // 22.10 fixed width decimal, freq of C4. Or just the frequency of C14.
    uint32_t loopStart;
    uint32_t size; // number of samples
    int8_t data[]; // samples
};


struct MP2KInstrument {
    uint8_t type;
    uint8_t drumKey;
    uint8_t cgbLength;
    uint8_t panSweep;
    union {
        uint32_t wav;  // struct WaveData *wav;
        uint32_t group;  // struct MP2KInstrument *group;
        uint32_t cgbSample;  // uint32_t *cgb3Sample;
        uint32_t squareNoiseConfig;
    };
    union {
        struct {
            uint8_t attack;
            uint8_t decay;
            uint8_t sustain;
            uint8_t release;
        };
        uint32_t keySplitTable;  // uint8_t *keySplitTable;
    };
};

struct MP2KTrack {
    uint8_t status;
    uint8_t wait;
    uint8_t patternLevel;
    uint8_t repeatCount;
    uint8_t gateTime; // 0 if TIE
    uint8_t key;
    uint8_t velocity;
    uint8_t runningStatus;
    int8_t keyShiftCalculated; // Calculated by TrkVolPitSet using fields below. Units: semitones
    uint8_t pitchCalculated;    // Calculated by TrkVolPitSet using fields below. Units: 256ths of a semitone
    int8_t keyShift;           // Units: semitones
    int8_t keyShiftPublic;     // Units: semitones
    int8_t tune;               // Units: 64ths of a semitone
    uint8_t pitchPublic;        // Units: 256ths of a semitone
    int8_t bend;               // Units: (bendRange / 64)ths of a semitone
    uint8_t bendRange;
    uint8_t volRightCalculated;
    uint8_t volLeftCalculated;
    uint8_t vol;
    uint8_t volPublic; // Used both for fades and MPlayVolumeControl
    int8_t pan;
    int8_t panPublic;
    int8_t modCalculated;   // Pitch units: 16ths of a semitone
    uint8_t modDepth;
    uint8_t modType;
    uint8_t lfoSpeed;
    uint8_t lfoSpeedCounter;
    uint8_t lfoDelay;
    uint8_t lfoDelayCounter;
    uint8_t priority;
    uint8_t echoVolume;
    uint8_t echoLength;
    struct MixerSource *chan;
    struct MP2KInstrument instrument;
    uint8_t gap[10];
    uint16_t unk_3A;
    uint32_t ct;
    uint8_t *cmdPtr;
    uint8_t *patternStack[3];
};

struct MP2KPlayerState {
    struct SongHeader *songHeader;
    volatile uint32_t status;
    uint8_t trackCount;
    uint8_t priority;
    uint8_t cmd;
    uint8_t checkSongPriority;
    uint32_t clock;
    uint8_t padding[8];
    uint8_t *memaccArea;
    uint16_t tempoRawBPM; // 150 initially... this doesn't seem right but whatever
    uint16_t tempoScale; // 0x100 initially
    uint16_t tempoInterval; // 150 initially
    uint16_t tempoCounter;
    uint16_t fadeInterval;
    uint16_t fadeCounter;
    uint16_t isFadeTemporary:1;
    uint16_t isFadeIn:1;
    uint16_t fadeVolume:7;
    uint16_t :7; // padding
    struct MP2KTrack *tracks;
    struct MP2KInstrument *voicegroup;
    volatile uint32_t lockStatus;
    void (*nextPlayerFunc)(void *);
    void *nextPlayer;
};

struct MP2KPlayerCtor {
    struct MP2KPlayerState *player;
    struct MP2KTrack *tracks;
    uint8_t trackCount;
    uint8_t padding;
    uint16_t checkSongPriority;
};

void clear_modM(struct MP2KPlayerState *unused, struct MP2KTrack *track);
void MP2K_event_endtie(struct MP2KPlayerState *unused, struct MP2KTrack *track);
void MP2K_event_lfos(struct MP2KPlayerState *unused, struct MP2KTrack *track);
void MP2K_event_mod(struct MP2KPlayerState *unused, struct MP2KTrack *track);
#endif
