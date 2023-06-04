#include <stdint.h>
#include <string.h>
#include "m4a_internal.h"
#include "io_reg.h"
#include "cgb_audio.h"

uint8_t *musicData;
uint32_t songTableOffset;

extern const uint8_t gCgb3Vol[];

extern struct SoundMixerState *SOUND_INFO_PTR;

struct SoundMixerState gSoundInfo;
MPlayFunc gMPlayJumpTable[36];
struct CgbChannel gCgbChans[4];
struct MusicPlayerInfo gMPlayInfo_BGM;
struct MusicPlayerInfo gMPlayInfo_SE1;
struct MusicPlayerInfo gMPlayInfo_SE2;
struct MusicPlayerInfo gMPlayInfo_SE3;
struct MusicPlayerTrack gMPlayTrack_BGM[MAX_MUSICPLAYER_TRACKS];
struct MusicPlayerTrack gMPlayTrack_SE1[3];
struct MusicPlayerTrack gMPlayTrack_SE2[9];
struct MusicPlayerTrack gMPlayTrack_SE3[1];
uint8_t gMPlayMemAccArea[0x10];

void MP2K_event_nxx();
void MP2KPlayerMain();

void offsetPointer(uintptr_t *ptr) {
    *ptr -= 0x8000000;
    *ptr += (uintptr_t)musicData;
}

uint32_t MidiKeyToFreq(struct WaveData *wav, uint8_t key, uint8_t fineAdjust)
{
    uint32_t val1;
    uint32_t val2;
    uint32_t fineAdjustShifted = fineAdjust << 24;

    if (key > 178)
    {
        key = 178;
        fineAdjustShifted = 255 << 24;
    }

    val1 = gScaleTable[key];
    val1 = gFreqTable[val1 & 0xF] >> (val1 >> 4);

    val2 = gScaleTable[key + 1];
    val2 = gFreqTable[val2 & 0xF] >> (val2 >> 4);

    return umul3232H32(wav->freq, val1 + umul3232H32(val2 - val1, fineAdjustShifted));
}

void MPlayContinue(struct MusicPlayerInfo *mplayInfo)
{
    if (mplayInfo->lockStatus == ID_NUMBER)
    {
        mplayInfo->lockStatus++;
        mplayInfo->status &= ~MUSICPLAYER_STATUS_PAUSE;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void MPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed)
{
    if (mplayInfo->lockStatus == ID_NUMBER)
    {
        mplayInfo->lockStatus++;
        mplayInfo->fadeCounter = speed;
        mplayInfo->fadeInterval = speed;
        mplayInfo->fadeVolume = (64 << FADE_VOL_SHIFT);
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void m4aSoundInit(uint8_t *_music, uint32_t _songTableAddress)
{
    musicData = _music;
    songTableOffset = _songTableAddress;
    int32_t i;

    SoundInit(&gSoundInfo);
    MPlayExtender(gCgbChans);
    m4aSoundMode(SOUND_MODE_DA_BIT_8
               | SOUND_MODE_FREQ_42048
               | (12 << SOUND_MODE_MASVOL_SHIFT)
               | (5 << SOUND_MODE_MAXCHN_SHIFT));

    for (i = 0; i < NUM_MUSIC_PLAYERS; i++)
    {
        struct MusicPlayerInfo *mplayInfo = &gMPlayInfo_BGM;
        MPlayOpen(mplayInfo, &gMPlayTrack_BGM, MAX_MUSICPLAYER_TRACKS);
        mplayInfo->checkSongPriority = 0;
        mplayInfo->memAccArea = gMPlayMemAccArea;
    }
}

void m4aSoundMain(void)
{
    RunMixerFrame();
}

void m4aSongNumStart(uint16_t n)
{
    const struct Song *songTable = (uintptr_t) musicData + songTableOffset;  //gSongTable;
    const struct Song *song = &songTable[n];

    MPlayStart(&gMPlayInfo_BGM, song->header);
}

void m4aSongNumStartOrChange(uint16_t n)
{
    const struct Song *songTable = (uintptr_t) musicData + songTableOffset;  //gSongTable;
    const struct Song *song = &songTable[n];

    if (gMPlayInfo_BGM.songHeader != song->header)
    {
        MPlayStart(&gMPlayInfo_BGM, song->header);
    }
    else
    {
        if ((gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK) == 0
         || (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE))
        {
            MPlayStart(&gMPlayInfo_BGM, song->header);
        }
    }
}

void m4aSongNumStartOrContinue(uint16_t n)
{
    const struct Song *songTable = (uintptr_t) musicData + songTableOffset;  //gSongTable;
    const struct Song *song = &songTable[n];

    if (gMPlayInfo_BGM.songHeader != song->header)
        MPlayStart(&gMPlayInfo_BGM, song->header);
    else if ((gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK) == 0)
        MPlayStart(&gMPlayInfo_BGM, song->header);
    else if (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
        MPlayContinue(&gMPlayInfo_BGM);
}

void m4aSongNumStop(uint16_t n)
{
    const struct Song *songTable = (uintptr_t) musicData + songTableOffset;  //gSongTable;
    const struct Song *song = &songTable[n];

    if (gMPlayInfo_BGM.songHeader == song->header)
        m4aMPlayStop(&gMPlayInfo_BGM);
}

void m4aSongNumContinue(uint16_t n)
{
    const struct Song *songTable = (uintptr_t) musicData + songTableOffset;  //gSongTable;
    const struct Song *song = &songTable[n];

    if (gMPlayInfo_BGM.songHeader == song->header)
        MPlayContinue(&gMPlayInfo_BGM);
}

void m4aMPlayAllStop(void)
{
    int32_t i;

    for (i = 0; i < NUM_MUSIC_PLAYERS; i++)
        m4aMPlayStop(&gMPlayInfo_BGM);
}

void m4aMPlayContinue(struct MusicPlayerInfo *mplayInfo)
{
    MPlayContinue(mplayInfo);
}

void m4aMPlayAllContinue(void)
{
    int32_t i;

    for (i = 0; i < NUM_MUSIC_PLAYERS; i++)
        MPlayContinue(&gMPlayInfo_BGM);
}

void m4aMPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed)
{
    MPlayFadeOut(mplayInfo, speed);
}

void m4aMPlayFadeOutTemporarily(struct MusicPlayerInfo *mplayInfo, uint16_t speed)
{
    if (mplayInfo->lockStatus == ID_NUMBER)
    {
        mplayInfo->lockStatus++;
        mplayInfo->fadeCounter = speed;
        mplayInfo->fadeInterval = speed;
        mplayInfo->fadeVolume = (64 << FADE_VOL_SHIFT) | TEMPORARY_FADE;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void m4aMPlayFadeIn(struct MusicPlayerInfo *mplayInfo, uint16_t speed)
{
    if (mplayInfo->lockStatus == ID_NUMBER)
    {
        mplayInfo->lockStatus++;
        mplayInfo->fadeCounter = speed;
        mplayInfo->fadeInterval = speed;
        mplayInfo->fadeVolume = (0 << FADE_VOL_SHIFT) | FADE_IN;
        mplayInfo->status &= ~MUSICPLAYER_STATUS_PAUSE;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void m4aMPlayImmInit(struct MusicPlayerInfo *mplayInfo)
{
    int32_t trackCount = mplayInfo->trackCount;
    struct MusicPlayerTrack *track = mplayInfo->tracks;

    while (trackCount > 0)
    {
        if (track->flags & MPT_FLG_EXIST)
        {
            if (track->flags & MPT_FLG_START)
            {
                track->flags = MPT_FLG_EXIST;
                track->bendRange = 2;
                track->volPublic = 64;
                track->lfoSpeed = 22;
                track->instrument.type = 1;
            }
        }

        trackCount--;
        track++;
    }
}

void MPlayExtender(struct CgbChannel *cgbChans)
{
    struct SoundMixerState *soundInfo;
    uint32_t lockStatus;

    REG_SOUNDCNT_X = SOUND_MASTER_ENABLE
                   | SOUND_4_ON
                   | SOUND_3_ON
                   | SOUND_2_ON
                   | SOUND_1_ON;
    REG_SOUNDCNT_L = 0; // set master volume to zero
    REG_NR12 = 0x8;
    REG_NR22 = 0x8;
    REG_NR42 = 0x8;
    REG_NR14 = 0x80;
    REG_NR24 = 0x80;
    REG_NR44 = 0x80;
    REG_NR30 = 0;
    REG_NR50 = 0x77;


    for(uint8_t i = 0; i < 4; i++){
        cgb_set_envelope(i, 8);
        cgb_trigger_note(i);
    }

    soundInfo = SOUND_INFO_PTR;

    lockStatus = soundInfo->lockStatus;

    if (lockStatus != ID_NUMBER)
        return;

    soundInfo->lockStatus++;

    gMPlayJumpTable[8] = ply_memacc;
    gMPlayJumpTable[17] = MP2K_event_lfos;
    gMPlayJumpTable[19] = MP2K_event_mod;
    gMPlayJumpTable[28] = ply_xcmd;
    gMPlayJumpTable[29] = MP2K_event_endtie;
    gMPlayJumpTable[30] = SampleFreqSet;
    gMPlayJumpTable[31] = TrackStop;
    gMPlayJumpTable[32] = FadeOutBody;
    gMPlayJumpTable[33] = TrkVolPitSet;

    soundInfo->cgbChans = cgbChans;
    soundInfo->cgbMixerFunc = cgbMixerFunc;
    soundInfo->cgbNoteOffFunc = cgbNoteOffFunc;
    soundInfo->cgbCalcFreqFunc = cgbCalcFreqFunc;
    soundInfo->maxScanlines = MAX_LINES;

    //CpuFill32(0, cgbChans, sizeof(struct CgbChannel) * 4);

    cgbChans[0].type = 1;
    cgbChans[0].panMask = 0x11;
    cgbChans[1].type = 2;
    cgbChans[1].panMask = 0x22;
    cgbChans[2].type = 3;
    cgbChans[2].panMask = 0x44;
    cgbChans[3].type = 4;
    cgbChans[3].panMask = 0x88;

    soundInfo->lockStatus = lockStatus;
}



void ClearChain(void *x)
{
    MP2KClearChain(x);
}

void SoundInit(struct SoundMixerState *soundInfo)
{
    soundInfo->lockStatus = 0;

    if (REG_DMA1CNT & (DMA_REPEAT << 16))
        REG_DMA1CNT = ((DMA_ENABLE | DMA_START_NOW | DMA_32BIT | DMA_SRC_INC | DMA_DEST_FIXED) << 16) | 4;

    if (REG_DMA2CNT & (DMA_REPEAT << 16))
        REG_DMA2CNT = ((DMA_ENABLE | DMA_START_NOW | DMA_32BIT | DMA_SRC_INC | DMA_DEST_FIXED) << 16) | 4;

    REG_DMA1CNT_H = DMA_32BIT;
    REG_DMA2CNT_H = DMA_32BIT;
    REG_SOUNDCNT_X = SOUND_MASTER_ENABLE
                   | SOUND_4_ON
                   | SOUND_3_ON
                   | SOUND_2_ON
                   | SOUND_1_ON;
    REG_SOUNDCNT_H = SOUND_B_FIFO_RESET | SOUND_B_TIMER_0 | SOUND_B_LEFT_OUTPUT
                   | SOUND_A_FIFO_RESET | SOUND_A_TIMER_0 | SOUND_A_RIGHT_OUTPUT
                   | SOUND_ALL_MIX_FULL;
    REG_SOUNDBIAS_H = (REG_SOUNDBIAS_H & 0x3F) | 0x40;

    /*REG_DMA1SAD = (int32_t)soundInfo->outBuffer;
    REG_DMA1DAD = (int32_t)&REG_FIFO_A;
    REG_DMA2SAD = (int32_t)soundInfo->outBuffer + PCM_DMA_BUF_SIZE;
    REG_DMA2DAD = (int32_t)&REG_FIFO_B;*/

    SOUND_INFO_PTR = soundInfo;
    //CpuFill32(0, soundInfo, sizeof(struct SoundMixerState));

    soundInfo->numChans = 8;
    soundInfo->masterVol = 15;
    soundInfo->mp2kEventNxxFunc = MP2K_event_nxx;
    soundInfo->cgbMixerFunc = DummyFunc;
    soundInfo->cgbNoteOffFunc = (CgbOscOffFunc)DummyFunc;
    soundInfo->cgbCalcFreqFunc = (MidiKeyToCgbFreqFunc)DummyFunc;
    soundInfo->ExtVolPit = (ExtVolPitFunc)DummyFunc;

    MPlayJumpTableCopy(gMPlayJumpTable);

    soundInfo->mp2kEventFuncTable = gMPlayJumpTable;

    SampleFreqSet(SOUND_MODE_FREQ_42048);

    soundInfo->lockStatus = ID_NUMBER;
}

void SampleFreqSet(uint32_t freq)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;

    freq = (freq & 0xF0000) >> 16;
    soundInfo->freq = freq;

    soundInfo->samplesPerFrame = 701;

    soundInfo->pcmDmaPeriod = PCM_DMA_BUF_SIZE / soundInfo->samplesPerFrame;

    soundInfo->sampleRate = 60.0f * soundInfo->samplesPerFrame;

    soundInfo->divFreq = 1.0f / soundInfo->sampleRate;

    // Turn off timer 0.
    REG_TM0CNT_H = 0;

    // cycles per LCD fresh 280896
    REG_TM0CNT_L = -(280896 / soundInfo->samplesPerFrame);

    m4aSoundVSyncOn();

    REG_TM0CNT_H = TIMER_ENABLE | TIMER_1CLK;
}

void m4aSoundMode(uint32_t mode)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;
    uint32_t temp;

    if (soundInfo->lockStatus != ID_NUMBER)
        return;

    soundInfo->lockStatus++;

    temp = mode & (SOUND_MODE_REVERB_SET | SOUND_MODE_REVERB_VAL);

    if (temp)
        soundInfo->reverb = temp & SOUND_MODE_REVERB_VAL;

    temp = mode & SOUND_MODE_MAXCHN;

    if (temp)
    {
        struct SoundChannel *chan;

        soundInfo->numChans = temp >> SOUND_MODE_MAXCHN_SHIFT;

        temp = MAX_DIRECTSOUND_CHANNELS;
        chan = &soundInfo->chans[0];

        while (temp != 0)
        {
            chan->statusFlags = 0;
            temp--;
            chan++;
        }
    }

    temp = mode & SOUND_MODE_MASVOL;

    if (temp)
        soundInfo->masterVol = temp >> SOUND_MODE_MASVOL_SHIFT;

    temp = mode & SOUND_MODE_DA_BIT;

    if (temp)
    {
        temp = (temp & 0x300000) >> 14;
        REG_SOUNDBIAS_H = (REG_SOUNDBIAS_H & 0x3F) | temp;
    }

    temp = mode & SOUND_MODE_FREQ;

    if (temp)
    {
        m4aSoundVSyncOff();
        SampleFreqSet(temp);
    }

    soundInfo->lockStatus = ID_NUMBER;
}

void SoundClear(void)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;
    int32_t i;
    void *chan;

    if (soundInfo->lockStatus != ID_NUMBER)
        return;

    soundInfo->lockStatus++;

    i = MAX_DIRECTSOUND_CHANNELS;
    chan = &soundInfo->chans[0];

    while (i > 0)
    {
        ((struct SoundChannel *)chan)->statusFlags = 0;
        i--;
        chan = (void *)((int32_t)chan + sizeof(struct SoundChannel));
    }

    chan = soundInfo->cgbChans;

    if (chan)
    {
        i = 1;

        while (i <= 4)
        {
            soundInfo->cgbNoteOffFunc(i);
            ((struct CgbChannel *)chan)->statusFlags = 0;
            i++;
            chan = (void *)((int32_t)chan + sizeof(struct CgbChannel));
        }
    }

    soundInfo->lockStatus = ID_NUMBER;
}

void m4aSoundVSyncOff(void)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;

    if (soundInfo->lockStatus >= ID_NUMBER && soundInfo->lockStatus <= ID_NUMBER + 1)
    {
        soundInfo->lockStatus += 10;

        if (REG_DMA1CNT & (DMA_REPEAT << 16))
            REG_DMA1CNT = ((DMA_ENABLE | DMA_START_NOW | DMA_32BIT | DMA_SRC_INC | DMA_DEST_FIXED) << 16) | 4;

        if (REG_DMA2CNT & (DMA_REPEAT << 16))
            REG_DMA2CNT = ((DMA_ENABLE | DMA_START_NOW | DMA_32BIT | DMA_SRC_INC | DMA_DEST_FIXED) << 16) | 4;

        REG_DMA1CNT_H = DMA_32BIT;
        REG_DMA2CNT_H = DMA_32BIT;

        //CpuFill32(0, soundInfo->outBuffer, sizeof(soundInfo->outBuffer));
    }
}

void m4aSoundVSyncOn(void)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;
    uint32_t lockStatus = soundInfo->lockStatus;

    if (lockStatus == ID_NUMBER)
        return;

    REG_DMA1CNT_H = DMA_ENABLE | DMA_START_SPECIAL | DMA_32BIT | DMA_REPEAT;
    REG_DMA2CNT_H = DMA_ENABLE | DMA_START_SPECIAL | DMA_32BIT | DMA_REPEAT;

    soundInfo->dmaCounter = 0;
    soundInfo->lockStatus = lockStatus - 10;
}

void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *tracks, uint8_t trackCount)
{
    struct SoundMixerState *soundInfo;

    if (trackCount == 0)
        return;

    if (trackCount > MAX_MUSICPLAYER_TRACKS)
        trackCount = MAX_MUSICPLAYER_TRACKS;

    soundInfo = SOUND_INFO_PTR;

    if (soundInfo->lockStatus != ID_NUMBER)
        return;

    soundInfo->lockStatus++;

    mplayInfo->tracks = tracks;
    mplayInfo->trackCount = trackCount;
    mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

    while (trackCount != 0)
    {
        tracks->flags = 0;
        trackCount--;
        tracks++;
    }

    // append music player and MPlayMain to linked list

    if (soundInfo->firstPlayerFunc != NULL)
    {
        mplayInfo->nextPlayerFunc = soundInfo->firstPlayerFunc;
        mplayInfo->nextPlayer = soundInfo->firstPlayer;
    }

    soundInfo->firstPlayer = (uintptr_t)mplayInfo;
    soundInfo->firstPlayerFunc = (uintptr_t)MP2KPlayerMain;
    soundInfo->lockStatus = ID_NUMBER;
    mplayInfo->lockStatus = ID_NUMBER;
}

void MPlayStart(struct MusicPlayerInfo *mplayInfo, struct SongHeader *songHeader)
{
    offsetPointer(&songHeader);
    int32_t i;
    uint8_t checkSongPriority;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    checkSongPriority = mplayInfo->checkSongPriority;

    if (!checkSongPriority
        || ((!mplayInfo->songHeader || !(mplayInfo->tracks[0].flags & MPT_FLG_START))
            && ((mplayInfo->status & MUSICPLAYER_STATUS_TRACK) == 0
                || (mplayInfo->status & MUSICPLAYER_STATUS_PAUSE)))
        || (mplayInfo->priority <= songHeader->priority))
    {
        mplayInfo->lockStatus++;
        mplayInfo->status = 0;
        mplayInfo->songHeader = songHeader;
        mplayInfo->voicegroup = songHeader->instrument;
        offsetPointer(&mplayInfo->voicegroup);
        mplayInfo->priority = songHeader->priority;
        mplayInfo->clock = 0;
        mplayInfo->tempoRawBPM = 150;
        mplayInfo->tempoInterval = 150;
        mplayInfo->tempoScale = 0x100;
        mplayInfo->tempoCounter = 0;
        mplayInfo->fadeInterval = 0;

        i = 0;
        track = mplayInfo->tracks;

        while (i < songHeader->trackCount && i < mplayInfo->trackCount)
        {
            TrackStop(mplayInfo, track);
            track->flags = MPT_FLG_EXIST | MPT_FLG_START;
            track->chan = 0;
            track->cmdPtr = songHeader->part[i];// + (uintptr_t) musicData;
            offsetPointer(&track->cmdPtr);
            i++;
            track++;
        }

        while (i < mplayInfo->trackCount)
        {
            TrackStop(mplayInfo, track);
            track->flags = 0;
            i++;
            track++;
        }

        if (songHeader->reverb & SOUND_MODE_REVERB_SET)
            m4aSoundMode(songHeader->reverb);

        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo)
{
    int32_t i;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;
    mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0)
    {
        TrackStop(mplayInfo, track);
        i--;
        track++;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

void FadeOutBody(struct MusicPlayerInfo *mplayInfo)
{
    int32_t i;
    struct MusicPlayerTrack *track;
    uint16_t fadeVolume;

    if (mplayInfo->fadeInterval == 0)
        return;
    if (--mplayInfo->fadeCounter != 0)
        return;

    mplayInfo->fadeCounter = mplayInfo->fadeInterval;

    if (mplayInfo->fadeVolume & FADE_IN)
    {
        if ((uint16_t)(mplayInfo->fadeVolume += (4 << FADE_VOL_SHIFT)) >= (64 << FADE_VOL_SHIFT))
        {
            mplayInfo->fadeVolume = (64 << FADE_VOL_SHIFT);
            mplayInfo->fadeInterval = 0;
        }
    }
    else
    {
        if ((int16_t)(mplayInfo->fadeVolume -= (4 << FADE_VOL_SHIFT)) <= 0)
        {
            i = mplayInfo->trackCount;
            track = mplayInfo->tracks;

            while (i > 0)
            {
                uint32_t val;

                TrackStop(mplayInfo, track);

                val = TEMPORARY_FADE;
                fadeVolume = mplayInfo->fadeVolume;
                val &= fadeVolume;

                if (!val)
                    track->flags = 0;

                i--;
                track++;
            }

            if (mplayInfo->fadeVolume & TEMPORARY_FADE)
                mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;
            else
                mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

            mplayInfo->fadeInterval = 0;
            return;
        }
    }

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0)
    {
        if (track->flags & MPT_FLG_EXIST)
        {
            fadeVolume = mplayInfo->fadeVolume;

            track->volPublic = (fadeVolume >> FADE_VOL_SHIFT);
            track->flags |= MPT_FLG_VOLCHG;
        }

        i--;
        track++;
    }
}

void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    if (track->flags & MPT_FLG_VOLSET)
    {
        int32_t x;
        int32_t y;

        x = (uint32_t)(track->vol * track->volPublic) >> 5;

        if (track->modType == 1)
            x = (uint32_t)(x * (track->modCalculated + 128)) >> 7;

        y = 2 * track->pan + track->panPublic;

        if (track->modType == 2)
            y += track->modCalculated;

        if (y < -128)
            y = -128;
        else if (y > 127)
            y = 127;

        track->volRightCalculated = (uint32_t)((y + 128) * x) >> 8;
        track->volLeftCalculated = (uint32_t)((127 - y) * x) >> 8;
    }

    if (track->flags & MPT_FLG_PITSET)
    {
        int32_t bend = track->bend * track->bendRange;
        int32_t x = (track->tune + bend)
              * 4
              + (track->keyShift << 8)
              + (track->keyShiftPublic << 8)
              + track->pitchPublic;

        if (track->modType == 0)
            x += 16 * track->modCalculated;

        track->keyShiftCalculated = x >> 8;
        track->pitchCalculated = x;
    }

    track->flags &= ~(MPT_FLG_PITSET | MPT_FLG_VOLSET);
}

uint32_t cgbCalcFreqFunc(uint8_t chanNum, uint8_t key, uint8_t fineAdjust)
{
    if (chanNum == 4)
    {
        if (key <= 20)
        {
            key = 0;
        }
        else
        {
            key -= 21;
            if (key > 59)
                key = 59;
        }

        return gNoiseTable[key];
    }
    else
    {
        int32_t val1;
        int32_t val2;

        if (key <= 35)
        {
            fineAdjust = 0;
            key = 0;
        }
        else
        {
            key -= 36;
            if (key > 130)
            {
                key = 130;
                fineAdjust = 255;
            }
        }

        val1 = gCgbScaleTable[key];
        val1 = gCgbFreqTable[val1 & 0xF] >> (val1 >> 4);

        val2 = gCgbScaleTable[key + 1];
        val2 = gCgbFreqTable[val2 & 0xF] >> (val2 >> 4);

        return val1 + ((fineAdjust * (val2 - val1)) >> 8) + 2048;
    }
}

void cgbNoteOffFunc(uint8_t chanNum)
{
    switch (chanNum)
    {
    case 1:
        REG_NR12 = 8;
        REG_NR14 = 0x80;
        break;
    case 2:
        REG_NR22 = 8;
        REG_NR24 = 0x80;
        break;
    case 3:
        REG_NR30 = 0;
        break;
    default:
        REG_NR42 = 8;
        REG_NR44 = 0x80;
    }

    cgb_set_envelope(chanNum - 1, 8);
    cgb_trigger_note(chanNum - 1);

}

static inline int CgbPan(struct CgbChannel *chan)
{
    uint32_t rightVolume = chan->rightVolume;
    uint32_t leftVolume = chan->leftVolume;

    if ((rightVolume = (uint8_t)rightVolume) >= (leftVolume = (uint8_t)leftVolume))
    {
        if (rightVolume / 2 >= leftVolume)
        {
            chan->pan = 0x0F;
            return 1;
        }
    }
    else
    {
        if (leftVolume / 2 >= rightVolume)
        {
            chan->pan = 0xF0;
            return 1;
        }
    }

    return 0;
}

void CgbModVol(struct CgbChannel *chan)
{
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;

    if ((soundInfo->mode & 1) || !CgbPan(chan))
    {
        chan->pan = 0xFF;
        chan->envelopeGoal = (uint32_t)(chan->rightVolume + chan->leftVolume) >> 4;
    }
    else
    {
        // Force chan->rightVolume and chan->leftVolume to be read from memory again,
        // even though there is no reason to do so.
        // The command line option "-fno-gcse" achieves the same result as this.

        chan->envelopeGoal = (uint32_t)(chan->rightVolume + chan->leftVolume) >> 4;
        if (chan->envelopeGoal > 15)
            chan->envelopeGoal = 15;
    }

    chan->sustainGoal = (chan->envelopeGoal * chan->sustain + 15) >> 4;
    chan->pan &= chan->panMask;
}

void cgbMixerFunc(void)
{
    int32_t ch;
    struct CgbChannel *channels;
    int32_t envelopeStepTimeAndDir;
    int32_t prevC15;
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;
    volatile uint8_t *nrx0ptr;
    volatile uint8_t *nrx1ptr;
    volatile uint8_t *nrx2ptr;
    volatile uint8_t *nrx3ptr;
    volatile uint8_t *nrx4ptr;

    // Most comparision operations that cast to int8_t perform 'and' by 0xFF.
    int mask = 0xff;

    if (soundInfo->cgbCounter15)
        soundInfo->cgbCounter15--;
    else
        soundInfo->cgbCounter15 = 14;

    for (ch = 1, channels = soundInfo->cgbChans; ch <= 4; ch++, channels++)
    {
        if (!(channels->statusFlags & SOUND_CHANNEL_SF_ON))
            continue;

        /* 1. determine hardware channel registers */
        switch (ch)
        {
        case 1:
            nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR10);
            nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR11);
            nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR12);
            nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR13);
            nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR14);
            break;
        case 2:
            nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR10+1);
            nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR21);
            nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR22);
            nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR23);
            nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR24);
            break;
        case 3:
            nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR30);
            nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR31);
            nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR32);
            nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR33);
            nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR34);
            break;
        default:
            nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR30+1);
            nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR41);
            nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR42);
            nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR43);
            nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR44);
            break;
        }

        prevC15 = soundInfo->cgbCounter15;
        envelopeStepTimeAndDir = *nrx2ptr;

        /* 2. calculate envelope volume */
        if (channels->statusFlags & SOUND_CHANNEL_SF_START)
        {
            if (!(channels->statusFlags & SOUND_CHANNEL_SF_STOP))
            {
                channels->statusFlags = SOUND_CHANNEL_SF_ENV_ATTACK;
                channels->modify = CGB_CHANNEL_MO_PIT | CGB_CHANNEL_MO_VOL;
                CgbModVol(channels);
                switch (ch)
                {
                case 1:
                    *nrx0ptr = channels->sweep;
                    cgb_set_sweep(channels->sweep);

                    // fallthrough
                case 2:
                    *nrx1ptr = ((uintptr_t)channels->wavePointer << 6) + channels->length;
                    goto init_env_step_time_dir;
                case 3:
                    if (channels->wavePointer != channels->currentPointer)
                    {
                        *nrx0ptr = 0x40;
                        REG_WAVE_RAM0 = channels->wavePointer[0];
                        REG_WAVE_RAM1 = channels->wavePointer[1];
                        REG_WAVE_RAM2 = channels->wavePointer[2];
                        REG_WAVE_RAM3 = channels->wavePointer[3];
                        channels->currentPointer = channels->wavePointer;
                        cgb_set_wavram();

                    }
                    *nrx0ptr = 0;
                    *nrx1ptr = channels->length;
                    if (channels->length)
                        channels->n4 = 0xC0;
                    else
                        channels->n4 = 0x80;
                    break;
                default:
                    *nrx1ptr = channels->length;
                    *nrx3ptr = (uintptr_t)channels->wavePointer << 3;
                init_env_step_time_dir:
                    envelopeStepTimeAndDir = channels->attack + CGB_NRx2_ENV_DIR_INC;
                    if (channels->length)
                        channels->n4 = 0x40;
                    else
                        channels->n4 = 0x00;
                    break;
                }
                cgb_set_length(ch - 1, channels->length);
                channels->envelopeCounter = channels->attack;
                if ((int8_t)(channels->attack & mask))
                {
                    channels->envelopeVolume = 0;
                    goto envelope_step_complete;
                }
                else
                {
                    // skip attack phase if attack is instantaneous (=0)
                    goto envelope_decay_start;
                }
            }
            else
            {
                goto oscillator_off;
            }
        }
        else if (channels->statusFlags & SOUND_CHANNEL_SF_IEC)
        {
            channels->echoLength--;
            if ((int8_t)(channels->echoLength & mask) <= 0)
            {
            oscillator_off:
                cgbNoteOffFunc(ch);
                channels->statusFlags = 0;
                goto channel_complete;
            }
            goto envelope_complete;
        }
        else if ((channels->statusFlags & SOUND_CHANNEL_SF_STOP) && (channels->statusFlags & SOUND_CHANNEL_SF_ENV))
        {
            channels->statusFlags &= ~SOUND_CHANNEL_SF_ENV;
            channels->envelopeCounter = channels->release;
            if ((int8_t)(channels->release & mask))
            {
                channels->modify |= CGB_CHANNEL_MO_VOL;
                if (ch != 3)
                    envelopeStepTimeAndDir = channels->release | CGB_NRx2_ENV_DIR_DEC;
                goto envelope_step_complete;
            }
            else
            {
                goto envelope_pseudoecho_start;
            }
        }
        else
        {
        envelope_step_repeat:
            if (channels->envelopeCounter == 0)
            {
                if (ch == 3)
                    channels->modify |= CGB_CHANNEL_MO_VOL;

                CgbModVol(channels);
                if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_RELEASE)
                {
                    channels->envelopeVolume--;
                    if ((int8_t)(channels->envelopeVolume & mask) <= 0)
                    {
                    envelope_pseudoecho_start:
                        channels->envelopeVolume = ((channels->envelopeGoal * channels->echoVolume) + 0xFF) >> 8;
                        if (channels->envelopeVolume)
                        {
                            channels->statusFlags |= SOUND_CHANNEL_SF_IEC;
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_complete;
                        }
                        else
                        {
                            goto oscillator_off;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->release;
                    }
                }
                else if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_SUSTAIN)
                {
                envelope_sustain:
                    channels->envelopeVolume = channels->sustainGoal;
                    channels->envelopeCounter = 7;
                }
                else if ((channels->statusFlags & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_DECAY)
                {
                    int envelopeVolume, sustainGoal;

                    channels->envelopeVolume--;
                    envelopeVolume = (int8_t)(channels->envelopeVolume & mask);
                    sustainGoal = (int8_t)(channels->sustainGoal);
                    if (envelopeVolume <= sustainGoal)
                    {
                    envelope_sustain_start:
                        if (channels->sustain == 0)
                        {
                            channels->statusFlags &= ~SOUND_CHANNEL_SF_ENV;
                            goto envelope_pseudoecho_start;
                        }
                        else
                        {
                            channels->statusFlags--;
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_sustain;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->decay;
                    }
                }
                else
                {
                    channels->envelopeVolume++;
                    if ((uint8_t)(channels->envelopeVolume & mask) >= channels->envelopeGoal)
                    {
                    envelope_decay_start:
                        channels->statusFlags--;
                        channels->envelopeCounter = channels->decay;
                        if ((uint8_t)(channels->envelopeCounter & mask))
                        {
                            channels->modify |= CGB_CHANNEL_MO_VOL;
                            channels->envelopeVolume = channels->envelopeGoal;
                            if (ch != 3)
                                envelopeStepTimeAndDir = channels->decay | CGB_NRx2_ENV_DIR_DEC;
                        }
                        else
                        {
                            goto envelope_sustain_start;
                        }
                    }
                    else
                    {
                        channels->envelopeCounter = channels->attack;
                    }
                }
            }
        }

    envelope_step_complete:
        // every 15 frames, envelope calculation has to be done twice
        // to keep up with the hardware envelope rate (1/64 s)
        channels->envelopeCounter--;
        if (prevC15 == 0)
        {
            prevC15--;
            goto envelope_step_repeat;
        }

    envelope_complete:
        /* 3. apply pitch to HW registers */
        if (channels->modify & CGB_CHANNEL_MO_PIT)
        {
            if (ch < 4 && (channels->type & TONEDATA_TYPE_FIX))
            {
                int dac_pwm_rate = REG_SOUNDBIAS_H;

                if (dac_pwm_rate < 0x40)        // if PWM rate = 32768 Hz
                    channels->freq = (channels->freq + 2) & 0x7fc;
                else if (dac_pwm_rate < 0x80)   // if PWM rate = 65536 Hz
                    channels->freq = (channels->freq + 1) & 0x7fe;
            }

            if (ch != 4)
                *nrx3ptr = channels->freq;
            else
                *nrx3ptr = (*nrx3ptr & 0x08) | channels->freq;
            channels->n4 = (channels->n4 & 0xC0) + (*((uint8_t*)(&channels->freq) + 1));
            *nrx4ptr = (int8_t)(channels->n4 & mask);
        }

        /* 4. apply envelope & volume to HW registers */
        if (channels->modify & CGB_CHANNEL_MO_VOL)
        {
            REG_NR51 = (REG_NR51 & ~channels->panMask) | channels->pan;
            if (ch == 3)
            {
                *nrx2ptr = gCgb3Vol[channels->envelopeVolume];
                if (channels->n4 & 0x80)
                {
                    *nrx0ptr = 0x80;
                    *nrx4ptr = channels->n4;
                    channels->n4 &= 0x7f;
                }
            }
            else
            {
                envelopeStepTimeAndDir &= 0xf;
                *nrx2ptr = (channels->envelopeVolume << 4) + envelopeStepTimeAndDir;
                *nrx4ptr = channels->n4 | 0x80;
                if (ch == 1 && !(*nrx0ptr & 0x08))
                    *nrx4ptr = channels->n4 | 0x80;
            }
            cgb_set_envelope(ch - 1, *nrx2ptr);
            cgb_toggle_length(ch - 1, (*nrx4ptr & 0x40));
            cgb_trigger_note(ch - 1);
        }

    channel_complete:
        channels->modify = 0;
    }
}

void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, uint16_t tempo)
{
    if (mplayInfo->lockStatus == ID_NUMBER)
    {
        mplayInfo->lockStatus++;
        mplayInfo->tempoScale = tempo;
        mplayInfo->tempoInterval = (mplayInfo->tempoRawBPM * mplayInfo->tempoScale) >> 8;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint16_t volume)
{
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0)
    {
        if (trackBits & bit)
        {
            if (track->flags & MPT_FLG_EXIST)
            {
                track->volPublic = volume / 4;
                track->flags |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int16_t pitch)
{
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0)
    {
        if (trackBits & bit)
        {
            if (track->flags & MPT_FLG_EXIST)
            {
                track->keyShiftPublic = pitch >> 8;
                track->pitchPublic = pitch;
                track->flags |= MPT_FLG_PITCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int8_t pan)
{
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0)
    {
        if (trackBits & bit)
        {
            if (track->flags & MPT_FLG_EXIST)
            {
                track->panPublic = pan;
                track->flags |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

void ClearModM(struct MusicPlayerTrack *track)
{
    track->lfoSpeedCounter = 0;
    track->modCalculated = 0;

    if (track->modType == 0)
        track->flags |= MPT_FLG_PITCHG;
    else
        track->flags |= MPT_FLG_VOLCHG;
}

void m4aMPlayModDepthSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t modDepth)
{
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0)
    {
        if (trackBits & bit)
        {
            if (track->flags & MPT_FLG_EXIST)
            {
                track->modDepth = modDepth;

                if (!track->modDepth)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

void m4aMPlayLFOSpeedSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t lfoSpeed)
{
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0)
    {
        if (trackBits & bit)
        {
            if (track->flags & MPT_FLG_EXIST)
            {
                track->lfoSpeed = lfoSpeed;

                if (!track->lfoSpeed)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

#define MEMACC_COND_JUMP(cond) \
if (cond)                      \
    goto cond_true;            \
else                           \
    goto cond_false;           \

void ply_memacc(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    uint32_t op;
    uint8_t *addr;
    uint8_t data;

    op = *track->cmdPtr;
    track->cmdPtr++;

    addr = mplayInfo->memAccArea + *track->cmdPtr;
    track->cmdPtr++;

    data = *track->cmdPtr;
    track->cmdPtr++;

    switch (op)
    {
    case 0:
        *addr = data;
        return;
    case 1:
        *addr += data;
        return;
    case 2:
        *addr -= data;
        return;
    case 3:
        *addr = mplayInfo->memAccArea[data];
        return;
    case 4:
        *addr += mplayInfo->memAccArea[data];
        return;
    case 5:
        *addr -= mplayInfo->memAccArea[data];
        return;
    case 6:
        MEMACC_COND_JUMP(*addr == data)
        return;
    case 7:
        MEMACC_COND_JUMP(*addr != data)
        return;
    case 8:
        MEMACC_COND_JUMP(*addr > data)
        return;
    case 9:
        MEMACC_COND_JUMP(*addr >= data)
        return;
    case 10:
        MEMACC_COND_JUMP(*addr <= data)
        return;
    case 11:
        MEMACC_COND_JUMP(*addr < data)
        return;
    case 12:
        MEMACC_COND_JUMP(*addr == mplayInfo->memAccArea[data])
        return;
    case 13:
        MEMACC_COND_JUMP(*addr != mplayInfo->memAccArea[data])
        return;
    case 14:
        MEMACC_COND_JUMP(*addr > mplayInfo->memAccArea[data])
        return;
    case 15:
        MEMACC_COND_JUMP(*addr >= mplayInfo->memAccArea[data])
        return;
    case 16:
        MEMACC_COND_JUMP(*addr <= mplayInfo->memAccArea[data])
        return;
    case 17:
        MEMACC_COND_JUMP(*addr < mplayInfo->memAccArea[data])
        return;
    default:
        return;
    }

cond_true:
    {
        // *& is required for matching
        (*&gMPlayJumpTable[1])(mplayInfo, track);
        return;
    }

cond_false:
    track->cmdPtr += 4;
}

void ply_xcmd(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    uint32_t n = *track->cmdPtr;
    track->cmdPtr++;

    gXcmdTable[n](mplayInfo, track);
}

void ply_xxx(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    gMPlayJumpTable[0](mplayInfo, track);
}

#define READ_XCMD_BYTE(var, n)       \
{                                    \
    uint32_t byte = track->cmdPtr[(n)]; \
    byte <<= n * 8;                  \
    (var) &= ~(0xFF << (n * 8));     \
    (var) |= byte;                   \
}

void ply_xwave(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    uint32_t wav;

    READ_XCMD_BYTE(wav, 0) // UB: uninitialized variable
    READ_XCMD_BYTE(wav, 1)
    READ_XCMD_BYTE(wav, 2)
    READ_XCMD_BYTE(wav, 3)

    track->instrument.wav = wav;
    track->cmdPtr += 4;
}

void ply_xtype(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.type = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xatta(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.attack = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xdeca(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.decay = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xsust(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.sustain = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xrele(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.release = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xiecv(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->echoVolume = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xiecl(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->echoLength = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xleng(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.length = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xswee(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->instrument.panSweep = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xcmd_0C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    uint32_t unk;

    READ_XCMD_BYTE(unk, 0) // UB: uninitialized variable
    READ_XCMD_BYTE(unk, 1)

    if (track->unk_3A < (uint16_t)unk)
    {
        track->unk_3A++;
        track->cmdPtr -= 2;
        track->wait = 1;
    }
    else
    {
        track->unk_3A = 0;
        track->cmdPtr += 2;
    }
}

void ply_xcmd_0D(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    uint32_t unk;
#ifdef UBFIX
    unk = 0;
#endif

    READ_XCMD_BYTE(unk, 0) // UB: uninitialized variable
    READ_XCMD_BYTE(unk, 1)
    READ_XCMD_BYTE(unk, 2)
    READ_XCMD_BYTE(unk, 3)

    track->count = unk;
    track->cmdPtr += 4;
}

void DummyFunc(void)
{
}
