#ifndef GUARD_GBA_M4A_INTERNAL_H
#define GUARD_GBA_M4A_INTERNAL_H

#include <stdint.h>
#include "io_reg.h"

#define MAX_SAMPLE_CHANNELS 16
#define MIXED_AUDIO_BUFFER_SIZE 4907

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
    uint16_t type;
    uint8_t padding;
    uint8_t loopFlags;
    uint32_t freq;  // 22.10 fixed width decimal, freq of C4. Or just the freq of C14.
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
    uint8_t key;
    uint8_t length;    // sound length (compatible sound)
    uint8_t panSweep;  // pan or sweep (compatible sound ch. 1)
    union {
        uint32_t wav;
        uint32_t group;
        uint32_t squareNoiseConfig;
    };
    union {
        struct {
            uint8_t attack;
            uint8_t decay;
            uint8_t sustain;
            uint8_t release;
        };
        uint32_t keySplitTable;
    };
};

struct SongHeader {
    uint8_t trackCount;
    uint8_t blockCount;
    uint8_t priority;
    uint8_t reverb;
    uint32_t instrument;
    uint32_t part[1];
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
    uint8_t status;
    uint8_t type;
    uint8_t rightVol;
    uint8_t leftVol;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t key;  // midi key as it was translated into final pitch
    uint8_t envelopeVol;
    union {
        uint8_t envelopeVolR;
        uint8_t envelopeGoal;
    };
    union {
        uint8_t envelopeVolL;
        uint8_t envelopeCounter;
    };
    uint8_t echoVol;
    uint8_t echoLen;
    uint8_t gateTime;
    uint8_t midiKey;  // midi key as it was used in the track data
    uint8_t velocity;
    uint8_t priority;
    uint8_t rhythmPan;
    union {
        uint32_t count;
        struct {
            uint8_t padding3;
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
    struct WaveData *wav;
    int8_t *current;
    struct MusicPlayerTrack *track;
    struct SoundChannel *prev;
    struct SoundChannel *next;
};

struct MusicPlayerInfo;

struct SoundInfo {
    float updateRate;
    // Direct Sound
    uint8_t reverb;
    uint8_t numChans;
    uint8_t masterVol;
    uint8_t freq;
    uint8_t mode;
    uint8_t c15;                // periodically counts from 14 down to 0 (15 states)
    uint8_t framesPerDmaCycle;  // number of V-blanks per PCM DMA
    int32_t samplesPerFrame;
    int32_t sampleRate;
    float divFreq;
    struct SoundChannel *cgbChans;
    struct SoundChannel chans[MAX_SAMPLE_CHANNELS];
    float pcmBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
};

#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_START 0x40
#define MPT_FLG_EXIST 0x80

struct MusicPlayerTrack {
    uint32_t offset;
    uint8_t status;
    uint8_t wait;
    uint8_t patternLevel;
    uint8_t repeatCtr;
    uint8_t gateTime;
    uint8_t key;
    uint8_t velocity;
    uint8_t runningStatus;
    int8_t keyShiftCalc;  // Calculated by TrkVolPitSet using fields below. Units: semitones
    uint8_t pitchM;       // Calculated by TrkVolPitSet using fields below. Units: 256ths of a semitone
    int8_t keyShift;      // Units: semitones
    int8_t keyShiftX;     // Units: semitones
    int8_t tune;          // Units: 64ths of a semitone
    uint8_t pitchX;       // Units: 256ths of a semitone
    int8_t bend;          // Units: (bendRange / 64)ths of a semitone
    uint8_t bendRange;
    uint8_t volMR;
    uint8_t volML;
    uint8_t vol;
    uint8_t volX;  // Used both for fades and MPlayVolumeControl
    int8_t pan;
    int8_t panX;
    int8_t mod;  // Pitch units: 16ths of a semitone
    uint8_t modDepth;
    uint8_t modType;
    uint8_t lfoSpeed;
    uint8_t lfoSpeedCtr;
    uint8_t lfoDelay;
    uint8_t lfoDelayCtr;
    uint8_t priority;
    uint8_t echoVol;
    uint8_t echoLen;
    struct SoundChannel *chan;
    struct ToneData instrument;
    uint16_t unk_3A;
    uint32_t count;
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
    uint32_t clock;
    uint16_t tempo;
    uint16_t tempoScale;
    uint16_t tempoInterval;
    uint16_t tempoCtr;
    uint16_t fadeSpeed;
    uint16_t fadeCtr;
    uint16_t fadeVol;
    struct MusicPlayerTrack *tracks;
    struct ToneData *instrument;
};

struct Song {
    struct SongHeader *header;
    uint16_t ms;
    uint16_t me;
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
    __attribute__((aligned(4))) float pcmBuffer[MIXED_AUDIO_BUFFER_SIZE * 2];
};

extern uint8_t gMPlayMemAccArea[];

extern char SoundMainRAM[];

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
void SoundInit(uint32_t freq);
void MPlayExtender(struct SoundChannel *cgbChans);
void m4aSoundMode(uint32_t mode);
void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track, uint8_t a3);
void cgbMixerFunc(void);
void cgbNoteOffFunc(uint8_t);
void CgbModVol(struct SoundChannel *chan);
uint32_t cgbCalcFreqFunc(uint8_t, uint8_t, uint8_t);
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
