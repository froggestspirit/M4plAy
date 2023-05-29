#ifndef GUARD_GBA_M4A_INTERNAL_H
#define GUARD_GBA_M4A_INTERNAL_H


// ASCII encoding of 'Smsh' in reverse
// This is presumably short for SMASH, the developer of MKS4AGB.
#define ID_NUMBER 0x68736D53

#define C_V 0x40 // center value for PAN, BEND, and TUNE

#define SOUND_MODE_REVERB_VAL   0x0000007F
#define SOUND_MODE_REVERB_SET   0x00000080
#define SOUND_MODE_MAXCHN       0x00000F00
#define SOUND_MODE_MAXCHN_SHIFT 8
#define SOUND_MODE_MASVOL       0x0000F000
#define SOUND_MODE_MASVOL_SHIFT 12
#define SOUND_MODE_FREQ_05734   0x00010000
#define SOUND_MODE_FREQ_07884   0x00020000
#define SOUND_MODE_FREQ_10512   0x00030000
#define SOUND_MODE_FREQ_13379   0x00040000
#define SOUND_MODE_FREQ_15768   0x00050000
#define SOUND_MODE_FREQ_18157   0x00060000
#define SOUND_MODE_FREQ_21024   0x00070000
#define SOUND_MODE_FREQ_26758   0x00080000
#define SOUND_MODE_FREQ_31536   0x00090000
#define SOUND_MODE_FREQ_36314   0x000A0000
#define SOUND_MODE_FREQ_40137   0x000B0000
#define SOUND_MODE_FREQ_42048   0x000C0000
#define SOUND_MODE_FREQ         0x000F0000
#define SOUND_MODE_FREQ_SHIFT   16
#define SOUND_MODE_DA_BIT_9     0x00800000
#define SOUND_MODE_DA_BIT_8     0x00900000
#define SOUND_MODE_DA_BIT_7     0x00A00000
#define SOUND_MODE_DA_BIT_6     0x00B00000
#define SOUND_MODE_DA_BIT       0x00B00000
#define SOUND_MODE_DA_BIT_SHIFT 20

struct WaveData
{
    uint16_t type;
    uint16_t status;
    uint32_t freq;
    uint32_t loopStart;
    uint32_t size; // number of samples
    int8_t data[1]; // samples
};

#define TONEDATA_TYPE_CGB    0x07
#define TONEDATA_TYPE_FIX    0x08
#define TONEDATA_TYPE_SPL    0x40 // key split
#define TONEDATA_TYPE_RHY    0x80 // rhythm

#define TONEDATA_P_S_PAN    0xc0
#define TONEDATA_P_S_PAM    TONEDATA_P_S_PAN

struct ToneData
{
    uint8_t type;
    uint8_t key;
    uint8_t length; // sound length (compatible sound)
    uint8_t pan_sweep; // pan or sweep (compatible sound ch. 1)
    struct WaveData *wav;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
};

#define SOUND_CHANNEL_SF_START       0x80
#define SOUND_CHANNEL_SF_STOP        0x40
#define SOUND_CHANNEL_SF_LOOP        0x10
#define SOUND_CHANNEL_SF_IEC         0x04
#define SOUND_CHANNEL_SF_ENV         0x03
#define SOUND_CHANNEL_SF_ENV_ATTACK  0x03
#define SOUND_CHANNEL_SF_ENV_DECAY   0x02
#define SOUND_CHANNEL_SF_ENV_SUSTAIN 0x01
#define SOUND_CHANNEL_SF_ENV_RELEASE 0x00
#define SOUND_CHANNEL_SF_ON (SOUND_CHANNEL_SF_START | SOUND_CHANNEL_SF_STOP | SOUND_CHANNEL_SF_IEC | SOUND_CHANNEL_SF_ENV)

#define CGB_CHANNEL_MO_PIT  0x02
#define CGB_CHANNEL_MO_VOL  0x01

#define CGB_NRx2_ENV_DIR_DEC 0x00
#define CGB_NRx2_ENV_DIR_INC 0x08

struct CgbChannel
{
    uint8_t statusFlags;
    uint8_t type;
    uint8_t rightVolume;
    uint8_t leftVolume;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t key;
    uint8_t envelopeVolume;
    uint8_t envelopeGoal;
    uint8_t envelopeCounter;
    uint8_t pseudoEchoVolume;
    uint8_t pseudoEchoLength;
    uint8_t dummy1;
    uint8_t dummy2;
    uint8_t gateTime;
    uint8_t midiKey;
    uint8_t velocity;
    uint8_t priority;
    uint8_t rhythmPan;
    uint8_t dummy3[3];
    uint8_t dummy5;
    uint8_t sustainGoal;
    uint8_t n4;                  // NR[1-4]4 register (initial, length bit)
    uint8_t pan;
    uint8_t panMask;
    uint8_t modify;
    uint8_t length;
    uint8_t sweep;
    uint32_t frequency;
    uint32_t *wavePointer;       // instructs CgbMain to load targeted wave
    uint32_t *currentPointer;    // stores the currently loaded wave
    struct MusicPlayerTrack *track;
    void *prevChannelPointer;
    void *nextChannelPointer;
    uint8_t dummy4[8];
};

struct MusicPlayerTrack;

struct SoundChannel
{
    uint8_t statusFlags;
    uint8_t type;
    uint8_t rightVolume;
    uint8_t leftVolume;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t key;             // midi key as it was translated into final pitch
    uint8_t envelopeVolume;
    uint8_t envelopeVolumeRight;
    uint8_t envelopeVolumeLeft;
    uint8_t pseudoEchoVolume;
    uint8_t pseudoEchoLength;
    uint8_t dummy1;
    uint8_t dummy2;
    uint8_t gateTime;
    uint8_t midiKey;         // midi key as it was used in the track data
    uint8_t velocity;
    uint8_t priority;
    uint8_t rhythmPan;
    uint8_t dummy3[3];
    uint32_t count;
    float fw;
    uint32_t frequency;
    struct WaveData *wav;
    int8_t *currentPointer;
    struct MusicPlayerTrack *track;
    void *prevChannelPointer;
    void *nextChannelPointer;
    uint32_t dummy4;
    uint16_t xpi;
    uint16_t xpc;
};

#define MAX_DIRECTSOUND_CHANNELS 12
#define PCM_DMA_BUF_SIZE 4907 // size of Direct Sound buffer

struct MusicPlayerInfo;

typedef void (*MPlayFunc)();
typedef void (*PlyNoteFunc)(uint32_t, struct MusicPlayerInfo *, struct MusicPlayerTrack *);
typedef void (*CgbSoundFunc)(void);
typedef void (*CgbOscOffFunc)(uint8_t);
typedef uint32_t (*MidiKeyToCgbFreqFunc)(uint8_t, uint8_t, uint8_t);
typedef void (*ExtVolPitFunc)(void);
typedef void (*MPlayMainFunc)(struct MusicPlayerInfo *);

struct SoundInfo
{
    // This field is normally equal to ID_NUMBER but it is set to other
    // values during sensitive operations for locking purposes.
    // This field should be volatile but isn't. This could potentially cause
    // race conditions.
    uint32_t ident;

    volatile uint8_t pcmDmaCounter;

    // Direct Sound
    uint8_t reverb;
    uint8_t maxChans;
    uint8_t masterVolume;
    uint8_t freq;

    uint8_t mode;
    uint8_t c15;          // periodically counts from 14 down to 0 (15 states)
    uint8_t pcmDmaPeriod; // number of V-blanks per PCM DMA
    uint8_t maxLines;
    uint8_t gap[3];
    int32_t pcmSamplesPerVBlank;
    int32_t pcmFreq;
    float divFreq;
    struct CgbChannel *cgbChans;
    MPlayMainFunc MPlayMainHead;
    struct MusicPlayerInfo *musicPlayerHead;
    CgbSoundFunc CgbSound;
    CgbOscOffFunc CgbOscOff;
    MidiKeyToCgbFreqFunc MidiKeyToCgbFreq;
    MPlayFunc *MPlayJumpTable;
    PlyNoteFunc plynote;
    ExtVolPitFunc ExtVolPit;
    uint8_t gap2[16];
    struct SoundChannel chans[MAX_DIRECTSOUND_CHANNELS];
    float pcmBuffer[PCM_DMA_BUF_SIZE * 2];
};

struct SongHeader
{
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
#define MPT_FLG_START  0x40
#define MPT_FLG_EXIST  0x80

struct MusicPlayerTrack
{
    uint8_t flags;
    uint8_t wait;
    uint8_t patternLevel;
    uint8_t repN;
    uint8_t gateTime;
    uint8_t key;
    uint8_t velocity;
    uint8_t runningStatus;
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
    struct SoundChannel *chan;
    struct ToneData tone;
    uint8_t gap[10];
    uint16_t unk_3A;
    uint32_t unk_3C;
    uint8_t *cmdPtr;
    uint8_t *patternStack[3];
};

#define MUSICPLAYER_STATUS_TRACK 0x0000ffff
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

#define MAX_MUSICPLAYER_TRACKS 16

#define TEMPORARY_FADE  0x0001
#define FADE_IN         0x0002
#define FADE_VOL_MAX    64
#define FADE_VOL_SHIFT  2

struct MusicPlayerInfo
{
    struct SongHeader *songHeader;
    uint32_t status;
    uint8_t trackCount;
    uint8_t priority;
    uint8_t cmd;
    uint8_t unk_B;
    uint32_t clock;
    uint8_t gap[8];
    uint8_t *memAccArea;
    uint16_t tempoD;
    uint16_t tempoU;
    uint16_t tempoI;
    uint16_t tempoC;
    uint16_t fadeOI;
    uint16_t fadeOC;
    uint16_t fadeOV;
    struct MusicPlayerTrack *tracks;
    struct ToneData *tone;
    uint32_t ident;
    MPlayMainFunc MPlayMainNext;
    struct MusicPlayerInfo *musicPlayerNext;
};

struct MusicPlayer
{
    struct MusicPlayerInfo *info;
    struct MusicPlayerTrack *track;
    uint8_t unk_8;
    uint16_t unk_A;
};

struct Song
{
    struct SongHeader *header;
    uint16_t ms;
    uint16_t me;
};

extern const struct MusicPlayer gMPlayTable[];
extern const struct Song gSongTable[];



extern uint8_t gMPlayMemAccArea[];

extern char SoundMainRAM[];

extern MPlayFunc gMPlayJumpTable[];

typedef void (*XcmdFunc)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
extern const XcmdFunc gXcmdTable[];

extern struct CgbChannel gCgbChans[];

extern const uint8_t gScaleTable[];
extern const uint32_t gFreqTable[];
extern const uint16_t gPcmSamplesPerVBlankTable[];

extern const uint8_t gCgbScaleTable[];
extern const int16_t gCgbFreqTable[];
extern const uint8_t gNoiseTable[];

extern const struct ToneData voicegroup000;

#define NUM_MUSIC_PLAYERS 4
#define MAX_LINES 0

uint32_t umul3232H32(uint32_t multiplier, uint32_t multiplicand);
void SoundMain(void);
void SoundMainBTM(void *ptr);
void TrackStop(struct MP2KPlayerState *player, struct MP2KTrack *track);
void MPlayMain(struct MusicPlayerInfo *);
void MP2KClearChain(struct MixerSource *chan);

void MPlayContinue(struct MusicPlayerInfo *mplayInfo);
void MPlayStart(struct MusicPlayerInfo *mplayInfo, struct SongHeader *songHeader);
void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo);
void FadeOutBody(struct MusicPlayerInfo *mplayInfo);
void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track);
void MPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed);
void ClearChain(void *x);
void Clear64byte(void *addr);
void SoundInit(struct SoundInfo *soundInfo);
void MPlayExtender(struct CgbChannel *cgbChans);
void m4aSoundMode(uint32_t mode);
void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track, uint8_t a3);
void CgbSound(void);
void CgbOscOff(uint8_t);
void CgbModVol(struct CgbChannel *chan);
uint32_t MidiKeyToCgbFreq(uint8_t, uint8_t, uint8_t);
void DummyFunc(void);
void MPlayJumpTableCopy(void **mplayJumpTable);
void SampleFreqSet(uint32_t freq);
void m4aSoundVSyncOn(void);
void m4aSoundVSyncOff(void);

void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, uint16_t tempo);
void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint16_t volume);
void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int16_t pitch);
void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int8_t pan);
void ClearModM(struct MusicPlayerTrack *track);
void m4aMPlayModDepthSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t modDepth);
void m4aMPlayLFOSpeedSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t lfoSpeed);

// sound command handler functions
void MP2K_event_fine(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_goto(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_patt(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_pend(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_rept(struct MP2KPlayerState *, struct MP2KTrack *);
void ply_memacc(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_prio(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_tempo(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_keysh(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_voice(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_vol(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_pan(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_bend(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_bendr(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_lfos(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_lfodl(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_mod(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_modt(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_tune(struct MP2KPlayerState *, struct MP2KTrack *);
void MP2K_event_port(struct MP2KPlayerState *, struct MP2KTrack *);
void ply_xcmd(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
void MP2K_event_endtie(struct MP2KPlayerState *, struct MP2KTrack *);
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

#endif // GUARD_GBA_M4A_INTERNAL_H
