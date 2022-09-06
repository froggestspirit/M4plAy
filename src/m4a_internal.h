#ifndef GUARD_GBA_M4A_INTERNAL_H
#define GUARD_GBA_M4A_INTERNAL_H

#include <stdint.h>
#include "io_reg.h"

// ASCII encoding of 'Smsh' in reverse
// This is presumably short for SMASH, the developer of MKS4AGB.
#define ID_NUMBER 0x68736D53
#define MAX_SAMPLE_CHANNELS 12
#define MIXED_AUDIO_BUFFER_SIZE 4907
#define C_V 0x40  // center value for PAN, BEND, and TUNE

#define SOUND_MODE_REVERB_VAL 0x0000007F
#define SOUND_MODE_REVERB_SET 0x00000080
#define SOUND_MODE_MAXCHN 0x00000F00
#define SOUND_MODE_MAXCHN_SHIFT 8
#define SOUND_MODE_MASVOL 0x0000F000
#define SOUND_MODE_MASVOL_SHIFT 12
#define SOUND_MODE_FREQ_05734 0x00010000
#define SOUND_MODE_FREQ_07884 0x00020000
#define SOUND_MODE_FREQ_10512 0x00030000
#define SOUND_MODE_FREQ_13379 0x00040000
#define SOUND_MODE_FREQ_15768 0x00050000
#define SOUND_MODE_FREQ_18157 0x00060000
#define SOUND_MODE_FREQ_21024 0x00070000
#define SOUND_MODE_FREQ_26758 0x00080000
#define SOUND_MODE_FREQ_31536 0x00090000
#define SOUND_MODE_FREQ_36314 0x000A0000
#define SOUND_MODE_FREQ_40137 0x000B0000
#define SOUND_MODE_FREQ_42048 0x000C0000
#define SOUND_MODE_FREQ 0x000F0000
#define SOUND_MODE_FREQ_SHIFT 16
#define SOUND_MODE_DA_BIT_9 0x00800000
#define SOUND_MODE_DA_BIT_8 0x00900000
#define SOUND_MODE_DA_BIT_7 0x00A00000
#define SOUND_MODE_DA_BIT_6 0x00B00000
#define SOUND_MODE_DA_BIT 0x00B00000
#define SOUND_MODE_DA_BIT_SHIFT 20

struct WaveData {
    union {
        struct {
            uint16_t type;
            uint16_t status;
        };
        struct {
            uint8_t compressionFlags1;
            uint8_t compressionFlags2;
            uint8_t compressionFlags3;
            uint8_t loopFlags;
        };
    };
    uint32_t freq;  // 22.10 fixed width decimal, freq of C4. Or just the frequency of C14.
    uint32_t loopStart;
    uint32_t size;   // number of samples
    int8_t data[1];  // samples
};

#define TONEDATA_TYPE_CGB 0x07
#define TONEDATA_TYPE_FIX 0x08
#define TONEDATA_TYPE_SPL 0x40  // key split
#define TONEDATA_TYPE_RHY 0x80  // rhythm

#define TONEDATA_P_S_PAN 0xc0
#define TONEDATA_P_S_PAM TONEDATA_P_S_PAN

struct ToneData {
    uint8_t type;
    union {
        uint8_t key;
        uint8_t drumKey;
    };
    union {
        uint8_t length;  // sound length (compatible sound)
        uint8_t cgbLength;
    };
    union {
        uint8_t pan_sweep;  // pan or sweep (compatible sound ch. 1)
        uint8_t panSweep;
    };
    union {
        struct WaveData *wav;
        struct ToneData *group;
        uint32_t squareNoiseConfig;
    };
    union {
        struct {
            uint8_t attack;
            uint8_t decay;
            uint8_t sustain;
            uint8_t release;
        };
        uint8_t *keySplitTable;
    };
};

#define SOUND_CHANNEL_SF_START 0x80
#define SOUND_CHANNEL_SF_STOP 0x40
#define SOUND_CHANNEL_SF_LOOP 0x10
#define SOUND_CHANNEL_SF_IEC 0x04
#define SOUND_CHANNEL_SF_ENV 0x03
#define SOUND_CHANNEL_SF_ENV_ATTACK 0x03
#define SOUND_CHANNEL_SF_ENV_DECAY 0x02
#define SOUND_CHANNEL_SF_ENV_SUSTAIN 0x01
#define SOUND_CHANNEL_SF_ENV_RELEASE 0x00
#define SOUND_CHANNEL_SF_ON (SOUND_CHANNEL_SF_START | SOUND_CHANNEL_SF_STOP | SOUND_CHANNEL_SF_IEC | SOUND_CHANNEL_SF_ENV)

#define CGB_CHANNEL_MO_PIT 0x02
#define CGB_CHANNEL_MO_VOL 0x01

#define CGB_NRx2_ENV_DIR_DEC 0x00
#define CGB_NRx2_ENV_DIR_INC 0x08

struct MusicPlayerTrack;

struct SoundChannel {
    union {
        uint8_t statusFlags;
        uint8_t status;
    };
    uint8_t type;
    union {
        uint8_t rightVolume;
        uint8_t rightVol;
    };
    union {
        uint8_t leftVolume;
        uint8_t leftVol;
    };
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t key;  // midi key as it was translated into final pitch
    union {
        struct {
            uint8_t envelopeVolume;
            uint8_t envelopeVolumeRight;
            uint8_t envelopeVolumeLeft;
            uint8_t pseudoEchoVolume;
            uint8_t pseudoEchoLength;
            uint8_t dummy1;
            uint8_t dummy2;
        };
        struct {
            uint8_t envelopeVol;
            union {
                uint8_t envelopeVolR;
                uint8_t envelopeGoal;
            } __attribute__((packed));
            union {
                uint8_t envelopeVolL;
                uint8_t envelopeCtr;
                uint8_t envelopeCounter;
            } __attribute__((packed));
            uint8_t echoVol;
            uint8_t echoLen;
            uint8_t padding1;
            uint8_t padding2;
        };
    };
    uint8_t gateTime;
    union {
        uint8_t midiKey;  // midi key as it was used in the track data
        uint8_t untransposedKey;
    };
    uint8_t velocity;
    uint8_t priority;
    uint8_t rhythmPan;
    uint8_t dummy3[3];
    union {
        uint32_t count;
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
    union {
        uint32_t frequency;
        uint32_t freq;
    };
    union {
        uint32_t *newCgb3Sample;
        struct WaveData *wav;
        uint32_t *wavePointer;  // instructs CgbMain to load targeted wave
    };
    union {
        uint32_t *oldCgb3Sample;
        int8_t *current;
        int8_t *currentPointer;
    };
    struct MusicPlayerTrack *track;
    union {
        void *prevChannelPointer;
        struct SoundChannel *prev;
    };
    union {
        void *nextChannelPointer;
        struct SoundChannel *next;
    };
    uint32_t dummy4;
    uint16_t xpi;
    uint16_t xpc;
};

struct MusicPlayerInfo;

typedef void (*MPlayFunc)();
typedef void (*PlyNoteFunc)(uint32_t, struct MusicPlayerInfo *, struct MusicPlayerTrack *);
typedef void (*CgbSoundFunc)(void);
typedef void (*CgbOscOffFunc)(uint8_t);
typedef uint32_t (*MidiKeyToCgbFreqFunc)(uint8_t, uint8_t, uint8_t);
typedef void (*ExtVolPitFunc)(void);
typedef void (*MPlayMainFunc)(struct MusicPlayerInfo *);

struct SoundInfo {
    // This field is normally equal to ID_NUMBER but it is set to other
    // values during sensitive operations for locking purposes.
    // This field should be volatile but isn't. This could potentially cause
    // race conditions.
    float updateRate;
    union {
        uint32_t ident;
        volatile uint32_t lockStatus;
    };
    union {
        volatile uint8_t pcmDmaCounter;
        volatile uint8_t dmaCounter;
    };
    // Direct Sound
    uint8_t reverb;
    union {
        uint8_t maxChans;
        uint8_t numChans;
    };
    union {
        struct {
            uint8_t masterVolume;
            uint8_t freq;
            uint8_t mode;
            uint8_t c15;           // periodically counts from 14 down to 0 (15 states)
            uint8_t pcmDmaPeriod;  // number of V-blanks per PCM DMA
            uint8_t maxLines;
            uint8_t gap[3];
            int32_t pcmSamplesPerVBlank;
            int32_t pcmFreq;
            float divFreq;
            struct SoundChannel *cgbChans;
            MPlayMainFunc MPlayMainHead;
            struct MusicPlayerInfo *musicPlayerHead;
            CgbSoundFunc CgbSound;
            CgbOscOffFunc CgbOscOff;
            MidiKeyToCgbFreqFunc MidiKeyToCgbFreq;
            MPlayFunc *MPlayJumpTable;
            PlyNoteFunc plynote;
            ExtVolPitFunc ExtVolPit;
            uint8_t gap2[16];
            struct SoundChannel chans[MAX_SAMPLE_CHANNELS];
            float pcmBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
        };
        struct {
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
            struct SoundChannel *cgbChansdummy;
            void (*firstPlayerFunc)(void *player);
            void *firstPlayer;
            void (*cgbMixerFunc)(void);
            void (*cgbNoteOffFunc)(uint8_t chan);
            uint32_t (*cgbCalcFreqFunc)(uint8_t chan, uint8_t key, uint8_t pitch);
            void (**mp2kEventFuncTable)();
            void (*mp2kEventNxxFunc)(uint8_t clock, struct MusicPlayerInfo *player, struct MusicPlayerTrack *track);
            void *reserved1;  // In poke* this is called "ExtVolPit"
            void *reserved2;
            void *reserved3;
            void *reversed4;
            void *reserved5;
            struct SoundChannel chansdummy[MAX_SAMPLE_CHANNELS];
            __attribute__((aligned(4))) float outBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
            // int8_t outBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
        };
    };
};

struct SongHeader {
    uint32_t offset;
    uint8_t trackCount;
    uint8_t blockCount;
    uint8_t priority;
    uint8_t reverb;
    struct ToneData *tone;
    uint8_t *part[1];
};

#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_START 0x40
#define MPT_FLG_EXIST 0x80

struct MusicPlayerTrack {
    uint32_t offset;
    union {
        uint8_t flags;
        uint8_t status;
    };
    uint8_t wait;
    uint8_t patternLevel;
    union {
        uint8_t repN;
        uint8_t repeatCount;
    };
    uint8_t gateTime;
    uint8_t key;
    uint8_t velocity;
    uint8_t runningStatus;
    union {
        struct {
            uint8_t keyM;
            uint8_t pitM;
            int8_t keyShift;
            int8_t keyShiftX;
            int8_t tune;
            uint8_t pitX;
            int8_t bend;
            uint8_t bendRange;
            uint8_t volMR;
            uint8_t volML;
            uint8_t vol;
            uint8_t volX;
            int8_t pan;
            int8_t panX;
            int8_t modM;
            uint8_t mod;
            uint8_t modT;
            uint8_t lfoSpeed;
            uint8_t lfoSpeedC;
            uint8_t lfoDelay;
            uint8_t lfoDelayC;
            uint8_t priority;
            uint8_t pseudoEchoVolume;
            uint8_t pseudoEchoLength;
        };
        struct {
            int8_t keyShiftCalculated;  // Calculated by TrkVolPitSet using fields below. Units: semitones
            uint8_t pitchCalculated;    // Calculated by TrkVolPitSet using fields below. Units: 256ths of a semitone
            int8_t keyShiftdummy;       // Units: semitones
            int8_t keyShiftPublic;      // Units: semitones
            int8_t tunedummy;           // Units: 64ths of a semitone
            uint8_t pitchPublic;        // Units: 256ths of a semitone
            int8_t benddummy;           // Units: (bendRange / 64)ths of a semitone
            uint8_t bendRangedummy;
            uint8_t volRightCalculated;
            uint8_t volLeftCalculated;
            uint8_t voldummy;
            uint8_t volPublic;  // Used both for fades and MPlayVolumeControl
            int8_t pandummy;
            int8_t panPublic;
            int8_t modCalculated;  // Pitch units: 16ths of a semitone
            uint8_t modDepth;
            uint8_t modType;
            uint8_t lfoSpeeddummy;
            uint8_t lfoSpeedCounter;
            uint8_t lfoDelaydummy;
            uint8_t lfoDelayCounter;
            uint8_t prioritydummy;
            uint8_t echoVolume;
            uint8_t echoLength;
        };
    };
    struct SoundChannel *chan;
    union {
        struct ToneData tone;
        struct ToneData instrument;
    };
    uint8_t gap[10];
    uint16_t unk_3A;
    union {
        uint32_t unk_3C;
        uint32_t ct;
    };
    uint8_t *cmdPtr;
    uint8_t *patternStack[3];
};

#define MUSICPLAYER_STATUS_TRACK 0x0000ffff
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

#define MAX_MUSICPLAYER_TRACKS 16

#define TEMPORARY_FADE 0x0001
#define FADE_IN 0x0002
#define FADE_VOL_MAX 64
#define FADE_VOL_SHIFT 2

struct MusicPlayerInfo {
    struct SongHeader *songHeader;
    uint32_t status;
    uint8_t trackCount;
    uint8_t priority;
    uint8_t cmd;
    union {
        uint8_t unk_B;
        uint8_t checkSongPriority;
    };
    uint32_t clock;
    uint8_t gap[8];
    uint8_t *memAccArea;
    union {
        struct {
            uint16_t tempoD;
            uint16_t tempoU;
            uint16_t tempoI;
            uint16_t tempoC;
            uint16_t fadeOI;
            uint16_t fadeOC;
            uint16_t fadeOV;
            struct MusicPlayerTrack *tracks;
            struct ToneData *tone;
            MPlayMainFunc MPlayMainNext;
            struct MusicPlayerInfo *musicPlayerNext;
        };
        struct {
            uint16_t tempoRawBPM;    // 150 initially... this doesn't seem right but whatever
            uint16_t tempoScale;     // 0x100 initially
            uint16_t tempoInterval;  // 150 initially
            uint16_t tempoCounter;
            uint16_t fadeInterval;
            uint16_t fadeCounter;
            uint16_t isFadeTemporary : 1;
            uint16_t isFadeIn : 1;
            uint16_t fadeVolume : 7;
            uint16_t : 7;  // padding
            struct MusicPlayerTrack *tracksdummy;
            struct ToneData *voicegroup;
            volatile uint32_t lockStatus;
            void (*nextPlayerFunc)(void *);
            void *nextPlayer;
        };
    };
};

struct MusicPlayer {
    struct MusicPlayerInfo *info;
    struct MusicPlayerTrack *track;
    uint8_t unk_8;
    uint16_t unk_A;
};

struct Song {
    struct SongHeader *header;
};

struct AudioCGB {
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
    uint16_t ch4LFSR[2];
    __attribute__((aligned(4))) float outBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
};

extern const struct MusicPlayer gMPlayTable[];
extern const struct Song gSongTable[];

extern uint8_t gMPlayMemAccArea[];

extern char SoundMainRAM[];

extern MPlayFunc gMPlayJumpTable[];

typedef void (*XcmdFunc)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
extern const XcmdFunc gXcmdTable[];

extern struct SoundChannel gCgbChans[];

extern const uint8_t gScaleTable[];
extern const uint32_t gFreqTable[];
extern const uint16_t gPcmSamplesPerVBlankTable[];

extern const uint8_t gCgbScaleTable[];
extern const int16_t gCgbFreqTable[];
extern const uint8_t gNoiseTable[];

extern const struct ToneData voicegroup000;

extern const int16_t PU0[];
extern const int16_t PU1[];
extern const int16_t PU2[];
extern const int16_t PU3[];
extern int16_t WAV[];
extern const float freqTable[];
extern const float freqTableNSE[];

#define NUM_MUSIC_PLAYERS 1
#define MAX_LINES 0

uint32_t umul3232H32(uint32_t multiplier, uint32_t multiplicand);
void SoundMain(void);
void SoundMainBTM(void *ptr);
void TrackStop(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track);
void MPlayMain(struct MusicPlayerInfo *);
void MP2KClearChain(struct SoundChannel *chan);

void MPlayContinue(struct MusicPlayerInfo *mplayInfo);
void MPlayStart(struct MusicPlayerInfo *mplayInfo, struct SongHeader *songHeader);
void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo);
void FadeOutBody(struct MusicPlayerInfo *mplayInfo);
void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track);
void MPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed);
void ClearChain(void *x);
void Clear64byte(void *addr);
void SoundInit(struct SoundInfo *soundInfo);
void MPlayExtender(struct SoundChannel *cgbChans);
void m4aSoundMode(uint32_t mode);
void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track, uint8_t a3);
void CgbSound(void);
void CgbOscOff(uint8_t);
void CgbModVol(struct SoundChannel *chan);
uint32_t MidiKeyToCgbFreq(uint8_t, uint8_t, uint8_t);
void MPlayJumpTableCopy(void **mplayJumpTable);
void SampleFreqSet(uint32_t freq);

void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, uint16_t tempo);
void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint16_t volume);
void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int16_t pitch);
void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int8_t pan);
void ClearModM(struct MusicPlayerTrack *track);
void m4aMPlayModDepthSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t modDepth);
void m4aMPlayLFOSpeedSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t lfoSpeed);

// sound command handler functions
void MP2K_event_fine(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_goto(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_patt(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_pend(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_rept(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_memacc(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_prio(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_tempo(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_keysh(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_voice(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_vol(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_pan(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_bend(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_bendr(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_lfos(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_lfodl(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_mod(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_modt(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_tune(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_port(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xcmd(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_endtie(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_note(struct MusicPlayerInfo *, struct MusicPlayerTrack *);

// extended sound command handler functions
void ply_xxx(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xwave(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xtype(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xatta(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xdeca(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xsust(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xrele(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xiecv(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xiecl(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xleng(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xswee(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xcmd_0C(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void ply_xcmd_0D(struct MusicPlayerInfo *, struct MusicPlayerTrack *);

#endif  // GUARD_GBA_M4A_INTERNAL_H
