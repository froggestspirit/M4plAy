#include <stdbool.h>
#include <stddef.h>

#include "m4a_internal.h"

unsigned char REG_BASE[0x400] __attribute__((aligned(4)));
struct SoundInfo *SOUND_INFO_PTR;
extern const uint8_t gCgb3Vol[];
extern unsigned char music[];
struct SoundInfo gSoundInfo;
struct SoundChannel gCgbChans[4];
struct MusicPlayerInfo gMPlayInfo_BGM;
struct MusicPlayerTrack gMPlayTrack_BGM[MAX_MUSICPLAYER_TRACKS];
uint8_t gMPlayMemAccArea[0x10];
uint64_t songTableOffset;
uint32_t songOffset;
uint32_t mplayOffset;
void MP2K_event_nxx(uint8_t clock, struct MusicPlayerInfo *player, struct MusicPlayerTrack *track);
void MP2KPlayerMain(void *voidPtrPlayer);

void offsetPointer(uintptr_t *ptr) {
    *ptr += (uintptr_t)music;
}

uint32_t MidiKeyToFreq(struct WaveData *wav, uint8_t key, uint8_t fineAdjust) {
    uint32_t val1;
    uint32_t val2;
    uint32_t fineAdjustShifted = fineAdjust << 24;

    if (key > 178) {
        key = 178;
        fineAdjustShifted = 255 << 24;
    }

    val1 = gScaleTable[key];
    val1 = gFreqTable[val1 & 0xF] >> (val1 >> 4);

    val2 = gScaleTable[key + 1];
    val2 = gFreqTable[val2 & 0xF] >> (val2 >> 4);

    return umul3232H32(wav->freq, val1 + umul3232H32(val2 - val1, fineAdjustShifted));
}

void MPlayContinue(struct MusicPlayerInfo *mplayInfo) {
    mplayInfo->status &= ~MUSICPLAYER_STATUS_PAUSE;
}

void MPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed) {
    mplayInfo->fadeCtr = speed;
    mplayInfo->fadeSpeed = speed;
    mplayInfo->fadeVol = (64 << FADE_VOL_SHIFT);
}

void m4aSoundInit(int freq) {
    cgb_audio_init(freq);
    SoundInit(freq);
    MPlayExtender(gCgbChans);
    m4aSoundMode(SOUND_MODE_DA_BIT_8 | (12 << SOUND_MODE_MASVOL_SHIFT) | (MAX_SAMPLE_CHANNELS << SOUND_MODE_MAXCHN_SHIFT));

    struct MusicPlayerInfo *mplayInfo = &gMPlayInfo_BGM;
    MPlayOpen(mplayInfo, gMPlayTrack_BGM, MAX_MUSICPLAYER_TRACKS);
}

void m4aSongNumStart(uint16_t n) {
    songTableOffset = music + 4;
    const struct Song *song = songTableOffset + (n * 8);
    offsetPointer(&song->header);

    MPlayStart(&gMPlayInfo_BGM, song->header);
}

void m4aMPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed) {
    MPlayFadeOut(mplayInfo, speed);
}

void m4aMPlayFadeOutTemporarily(struct MusicPlayerInfo *mplayInfo, uint16_t speed) {
    mplayInfo->fadeCtr = speed;
    mplayInfo->fadeSpeed = speed;
    mplayInfo->fadeVol = (64 << FADE_VOL_SHIFT) | TEMPORARY_FADE;
}

void m4aMPlayFadeIn(struct MusicPlayerInfo *mplayInfo, uint16_t speed) {
    mplayInfo->fadeCtr = speed;
    mplayInfo->fadeSpeed = speed;
    mplayInfo->fadeVol = (0 << FADE_VOL_SHIFT) | FADE_IN;
    mplayInfo->status &= ~MUSICPLAYER_STATUS_PAUSE;
}

void m4aMPlayImmInit(struct MusicPlayerInfo *mplayInfo) {
    int32_t trackCount = mplayInfo->trackCount;
    struct MusicPlayerTrack *track = mplayInfo->tracks;

    while (trackCount > 0) {
        if (track->status & MPT_FLG_EXIST) {
            if (track->status & MPT_FLG_START) {
                track->status = MPT_FLG_EXIST;
                track->bendRange = 2;
                track->volX = 64;
                track->lfoSpeed = 22;
                track->instrument.type = 1;
            }
        }

        trackCount--;
        track++;
    }
}

void MPlayExtender(struct SoundChannel *cgbChans) {
    struct SoundInfo *soundInfo;

    REG_SOUNDCNT_X = SOUND_MASTER_ENABLE | SOUND_4_ON | SOUND_3_ON | SOUND_2_ON | SOUND_1_ON;
    REG_SOUNDCNT_L = 0;  // set master volume to zero
    REG_NR12 = 0x8;
    REG_NR22 = 0x8;
    REG_NR42 = 0x8;
    REG_NR14 = 0x80;
    REG_NR24 = 0x80;
    REG_NR44 = 0x80;
    REG_NR30 = 0;
    REG_NR50 = 0x77;

    for (uint8_t i = 0; i < 4; i++) {
        cgb_set_envelope(i, 8);
        cgb_trigger_note(i);
    }
    soundInfo = SOUND_INFO_PTR;

    soundInfo->cgbChans = cgbChans;

    cgbChans[0].type = 1;
    cgbChans[0].panMask = 0x11;
    cgbChans[1].type = 2;
    cgbChans[1].panMask = 0x22;
    cgbChans[2].type = 3;
    cgbChans[2].panMask = 0x44;
    cgbChans[3].type = 4;
    cgbChans[3].panMask = 0x88;
}

void ClearChain(void *x) {
    MP2KClearChain(x);
}

void SoundInit(uint32_t freq) {
    REG_SOUNDCNT_X = SOUND_MASTER_ENABLE | SOUND_4_ON | SOUND_3_ON | SOUND_2_ON | SOUND_1_ON;
    REG_SOUNDCNT_H = SOUND_B_FIFO_RESET | SOUND_B_TIMER_0 | SOUND_B_LEFT_OUTPUT | SOUND_A_FIFO_RESET | SOUND_A_TIMER_0 | SOUND_A_RIGHT_OUTPUT | SOUND_ALL_MIX_FULL;
    REG_SOUNDBIAS_H = (REG_SOUNDBIAS_H & 0x3F) | 0x40;
    SOUND_INFO_PTR = &gSoundInfo;
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    gSoundInfo.updateRate = freq / 60;

    soundInfo->numChans = MAX_SAMPLE_CHANNELS;
    soundInfo->masterVol = 15;

    soundInfo->samplesPerFrame = gSoundInfo.updateRate + 0.5;

    soundInfo->sampleRate = freq;

    soundInfo->divFreq = 1.0f / soundInfo->sampleRate;
}

void m4aSoundMode(uint32_t mode) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    uint32_t temp;

    temp = mode & (SOUND_MODE_REVERB_SET | SOUND_MODE_REVERB_VAL);

    if (temp)
        soundInfo->reverb = temp & SOUND_MODE_REVERB_VAL;

    temp = mode & SOUND_MODE_MAXCHN;

    if (temp) {
        struct SoundChannel *chan;

        soundInfo->numChans = temp >> SOUND_MODE_MAXCHN_SHIFT;

        temp = MAX_SAMPLE_CHANNELS;
        chan = &soundInfo->chans[0];

        while (temp != 0) {
            chan->status = 0;
            temp--;
            chan++;
        }
    }

    temp = mode & SOUND_MODE_MASVOL;

    if (temp)
        soundInfo->masterVol = temp >> SOUND_MODE_MASVOL_SHIFT;

    temp = mode & SOUND_MODE_DA_BIT;

    if (temp) {
        temp = (temp & 0x300000) >> 14;
        REG_SOUNDBIAS_H = (REG_SOUNDBIAS_H & 0x3F) | temp;
    }
}

void SoundClear(void) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    int32_t i;
    void *chan;

    i = MAX_SAMPLE_CHANNELS;
    chan = &soundInfo->chans[0];

    while (i > 0) {
        ((struct SoundChannel *)chan)->status = 0;
        i--;
        chan = (void *)((int32_t)chan + sizeof(struct SoundChannel));
    }

    chan = soundInfo->cgbChans;

    if (chan) {
        i = 1;

        while (i <= 4) {
            cgbNoteOffFunc(i);
            ((struct SoundChannel *)chan)->status = 0;
            i++;
            chan = (void *)((int32_t)chan + sizeof(struct SoundChannel));
        }
    }
}

void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *tracks, uint8_t trackCount) {
    struct SoundInfo *soundInfo;

    if (trackCount == 0)
        return;

    if (trackCount > MAX_MUSICPLAYER_TRACKS)
        trackCount = MAX_MUSICPLAYER_TRACKS;

    soundInfo = SOUND_INFO_PTR;

    mplayInfo->tracks = tracks;
    mplayInfo->trackCount = trackCount;
    mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

    while (trackCount != 0) {
        tracks->status = 0;
        trackCount--;
        tracks++;
    }
}

void MPlayStart(struct MusicPlayerInfo *mplayInfo, struct SongHeader *songHeader) {
    int32_t i;
    struct MusicPlayerTrack *track;
    mplayInfo->status = 0;
    mplayInfo->songHeader = songHeader;
    mplayInfo->instrument = songHeader->instrument;
    offsetPointer(&mplayInfo->instrument);
    mplayInfo->priority = songHeader->priority;
    mplayInfo->clock = 0;
    mplayInfo->tempo = 150;
    mplayInfo->tempoInterval = 150;
    mplayInfo->tempoScale = 0x100;
    mplayInfo->tempoCtr = 0;
    mplayInfo->fadeSpeed = 0;

    i = 0;
    track = mplayInfo->tracks;

    while (i < songHeader->trackCount && i < mplayInfo->trackCount) {
        TrackStop(mplayInfo, track);
        track->status = MPT_FLG_EXIST | MPT_FLG_START;
        track->chan = 0;
        track->offset = songHeader->offset;
        track->cmdPtr = songHeader->part[i] - track->offset;
        offsetPointer(&track->cmdPtr);
        i++;
        track++;
    }

    while (i < mplayInfo->trackCount) {
        TrackStop(mplayInfo, track);
        track->status = 0;
        i++;
        track++;
    }

    if (songHeader->reverb & SOUND_MODE_REVERB_SET)
        m4aSoundMode(songHeader->reverb);
}

void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo) {
    int32_t i;
    struct MusicPlayerTrack *track;

    mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0) {
        TrackStop(mplayInfo, track);
        i--;
        track++;
    }
}

void FadeOutBody(struct MusicPlayerInfo *mplayInfo) {
    int32_t i;
    struct MusicPlayerTrack *track;
    uint16_t fadeVol;

    if (mplayInfo->fadeSpeed == 0)
        return;
    if (--mplayInfo->fadeCtr != 0)
        return;

    mplayInfo->fadeCtr = mplayInfo->fadeSpeed;

    if (mplayInfo->fadeVol & FADE_IN) {
        if ((uint16_t)(mplayInfo->fadeVol += (4 << FADE_VOL_SHIFT)) >= (64 << FADE_VOL_SHIFT)) {
            mplayInfo->fadeVol = (64 << FADE_VOL_SHIFT);
            mplayInfo->fadeSpeed = 0;
        }
    } else {
        if ((int16_t)(mplayInfo->fadeVol -= (4 << FADE_VOL_SHIFT)) <= 0) {
            i = mplayInfo->trackCount;
            track = mplayInfo->tracks;

            while (i > 0) {
                uint32_t val;

                TrackStop(mplayInfo, track);

                val = TEMPORARY_FADE;
                fadeVol = mplayInfo->fadeVol;
                val &= fadeVol;

                if (!val)
                    track->status = 0;

                i--;
                track++;
            }

            if (mplayInfo->fadeVol & TEMPORARY_FADE)
                mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;
            else
                mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

            mplayInfo->fadeSpeed = 0;
            return;
        }
    }

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0) {
        if (track->status & MPT_FLG_EXIST) {
            fadeVol = mplayInfo->fadeVol;

            track->volX = (fadeVol >> FADE_VOL_SHIFT);
            track->status |= MPT_FLG_VOLCHG;
        }

        i--;
        track++;
    }
}

void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    if (track->status & MPT_FLG_VOLSET) {
        int32_t x;
        int32_t y;

        x = (uint32_t)(track->vol * track->volX) >> 5;

        if (track->modType == 1)
            x = (uint32_t)(x * (track->mod + 128)) >> 7;

        y = 2 * track->pan + track->panX;

        if (track->modType == 2)
            y += track->mod;

        if (y < -128)
            y = -128;
        else if (y > 127)
            y = 127;

        track->volMR = (uint32_t)((y + 128) * x) >> 8;
        track->volML = (uint32_t)((127 - y) * x) >> 8;
    }

    if (track->status & MPT_FLG_PITSET) {
        int32_t bend = track->bend * track->bendRange;
        int32_t x = (track->tune + bend) * 4 + (track->keyShift << 8) + (track->keyShiftX << 8) + track->pitchX;

        if (track->modType == 0)
            x += 16 * track->mod;

        track->keyShiftCalc = x >> 8;
        track->pitchM = x;
    }

    track->status &= ~(MPT_FLG_PITSET | MPT_FLG_VOLSET);
}

uint32_t cgbCalcFreqFunc(uint8_t chanNum, uint8_t key, uint8_t fineAdjust) {
    if (chanNum == 4) {
        if (key <= 20) {
            key = 0;
        } else {
            key -= 21;
            if (key > 59)
                key = 59;
        }

        return gNoiseTable[key];
    } else {
        int32_t val1;
        int32_t val2;

        if (key <= 35) {
            fineAdjust = 0;
            key = 0;
        } else {
            key -= 36;
            if (key > 130) {
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

void cgbNoteOffFunc(uint8_t chanNum) {
    switch (chanNum) {
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

static inline int CgbPan(struct SoundChannel *chan) {
    uint32_t rightVol = chan->rightVol;
    uint32_t leftVol = chan->leftVol;

    if ((rightVol = (uint8_t)rightVol) >= (leftVol = (uint8_t)leftVol)) {
        if (rightVol / 2 >= leftVol) {
            chan->pan = 0x0F;
            return 1;
        }
    } else {
        if (leftVol / 2 >= rightVol) {
            chan->pan = 0xF0;
            return 1;
        }
    }

    return 0;
}

void CgbModVol(struct SoundChannel *chan) {
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;

    if ((soundInfo->mode & 1) || !CgbPan(chan)) {
        chan->pan = 0xFF;
        chan->envelopeGoal = (uint32_t)(chan->rightVol + chan->leftVol) >> 4;
    } else {
// Force chan->rightVol and chan->leftVol to be read from memory again,
// even though there is no reason to do so.
// The command line option "-fno-gcse" achieves the same result as this.
#ifndef NONMATCHING
        asm(""
            :
            :
            : "memory");
#endif

        chan->envelopeGoal = (uint32_t)(chan->rightVol + chan->leftVol) >> 4;
        if (chan->envelopeGoal > 15)
            chan->envelopeGoal = 15;
    }

    chan->sustainGoal = (chan->envelopeGoal * chan->sustain + 15) >> 4;
    chan->pan &= chan->panMask;
}

void cgbMixerFunc(void) {
    int32_t ch;
    struct SoundChannel *channels;
    int32_t envelopeStepTimeAndDir;
    int32_t prevC15;
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    volatile uint8_t *nrx0ptr;
    volatile uint8_t *nrx1ptr;
    volatile uint8_t *nrx2ptr;
    volatile uint8_t *nrx3ptr;
    volatile uint8_t *nrx4ptr;

    // Most comparision operations that cast to int8_t perform 'and' by 0xFF.
    int mask = 0xff;

    if (soundInfo->c15)
        soundInfo->c15--;
    else
        soundInfo->c15 = 14;

    for (ch = 1, channels = soundInfo->cgbChans; ch <= 4; ch++, channels++) {
        if (!(channels->status & SOUND_CHANNEL_SF_ON))
            continue;

        /* 1. determine hardware channel registers */
        switch (ch) {
            case 1:
                nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR10);
                nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR11);
                nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR12);
                nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR13);
                nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR14);
                break;
            case 2:
                nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR10 + 1);
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
                nrx0ptr = (volatile uint8_t *)(REG_ADDR_NR30 + 1);
                nrx1ptr = (volatile uint8_t *)(REG_ADDR_NR41);
                nrx2ptr = (volatile uint8_t *)(REG_ADDR_NR42);
                nrx3ptr = (volatile uint8_t *)(REG_ADDR_NR43);
                nrx4ptr = (volatile uint8_t *)(REG_ADDR_NR44);
                break;
        }

        prevC15 = soundInfo->c15;
        envelopeStepTimeAndDir = *nrx2ptr;

        /* 2. calculate envelope volume */
        if (channels->status & SOUND_CHANNEL_SF_START) {
            if (!(channels->status & SOUND_CHANNEL_SF_STOP)) {
                channels->status = SOUND_CHANNEL_SF_ENV_ATTACK;
                channels->cgbStatus = CGB_CHANNEL_MO_PIT | CGB_CHANNEL_MO_VOL;
                CgbModVol(channels);
                switch (ch) {
                    case 1:
                        *nrx0ptr = channels->sweep;
                        cgb_set_sweep(channels->sweep);
                        // fallthrough
                    case 2:
                        *nrx1ptr = ((uint32_t)channels->wav << 6) + channels->length;
                        goto init_env_step_time_dir;
                    case 3:
                        if (channels->wav != channels->current) {
                            *nrx0ptr = 0x40;
                            uint32_t *wav = channels->wav;
                            REG_WAVE_RAM0 = wav[0];
                            REG_WAVE_RAM1 = wav[1];
                            REG_WAVE_RAM2 = wav[2];
                            REG_WAVE_RAM3 = wav[3];
                            channels->current = wav;
                            cgb_set_wavram();
                        }
                        *nrx0ptr = 0;
                        *nrx1ptr = channels->length;
                        if (channels->length)
                            channels->nrx4 = 0xC0;
                        else
                            channels->nrx4 = 0x80;
                        break;
                    default:
                        *nrx1ptr = channels->length;
                        *nrx3ptr = (uint32_t)channels->wav << 3;
                    init_env_step_time_dir:
                        envelopeStepTimeAndDir = channels->attack + CGB_NRx2_ENV_DIR_INC;
                        if (channels->length)
                            channels->nrx4 = 0x40;
                        else
                            channels->nrx4 = 0x00;
                        break;
                }
                cgb_set_length(ch - 1, channels->length);
                channels->envelopeCounter = channels->attack;
                if ((int8_t)(channels->attack & mask)) {
                    channels->envelopeVol = 0;
                    goto envelope_step_complete;
                } else {
                    // skip attack phase if attack is instantaneous (=0)
                    goto envelope_decay_start;
                }
            } else {
                goto oscillator_off;
            }
        } else if (channels->status & SOUND_CHANNEL_SF_IEC) {
            channels->echoLen--;
            if ((int8_t)(channels->echoLen & mask) <= 0) {
            oscillator_off:
                cgbNoteOffFunc(ch);
                channels->status = 0;
                goto channel_complete;
            }
            goto envelope_complete;
        } else if ((channels->status & SOUND_CHANNEL_SF_STOP) && (channels->status & SOUND_CHANNEL_SF_ENV)) {
            channels->status &= ~SOUND_CHANNEL_SF_ENV;
            channels->envelopeCounter = channels->release;
            if ((int8_t)(channels->release & mask)) {
                channels->cgbStatus |= CGB_CHANNEL_MO_VOL;
                if (ch != 3)
                    envelopeStepTimeAndDir = channels->release | CGB_NRx2_ENV_DIR_DEC;
                goto envelope_step_complete;
            } else {
                goto envelope_pseudoecho_start;
            }
        } else {
        envelope_step_repeat:
            if (channels->envelopeCounter == 0) {
                if (ch == 3)
                    channels->cgbStatus |= CGB_CHANNEL_MO_VOL;

                CgbModVol(channels);
                if ((channels->status & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_RELEASE) {
                    channels->envelopeVol--;
                    if ((int8_t)(channels->envelopeVol & mask) <= 0) {
                    envelope_pseudoecho_start:
                        channels->envelopeVol = ((channels->envelopeGoal * channels->echoVol) + 0xFF) >> 8;
                        if (channels->envelopeVol) {
                            channels->status |= SOUND_CHANNEL_SF_IEC;
                            channels->cgbStatus |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_complete;
                        } else {
                            goto oscillator_off;
                        }
                    } else {
                        channels->envelopeCounter = channels->release;
                    }
                } else if ((channels->status & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_SUSTAIN) {
                envelope_sustain:
                    channels->envelopeVol = channels->sustainGoal;
                    channels->envelopeCounter = 7;
                } else if ((channels->status & SOUND_CHANNEL_SF_ENV) == SOUND_CHANNEL_SF_ENV_DECAY) {
                    int envelopeVol, sustainGoal;

                    channels->envelopeVol--;
                    envelopeVol = (int8_t)(channels->envelopeVol & mask);
                    sustainGoal = (int8_t)(channels->sustainGoal);
                    if (envelopeVol <= sustainGoal) {
                    envelope_sustain_start:
                        if (channels->sustain == 0) {
                            channels->status &= ~SOUND_CHANNEL_SF_ENV;
                            goto envelope_pseudoecho_start;
                        } else {
                            channels->status--;
                            channels->cgbStatus |= CGB_CHANNEL_MO_VOL;
                            if (ch != 3)
                                envelopeStepTimeAndDir = 0 | CGB_NRx2_ENV_DIR_INC;
                            goto envelope_sustain;
                        }
                    } else {
                        channels->envelopeCounter = channels->decay;
                    }
                } else {
                    channels->envelopeVol++;
                    if ((uint8_t)(channels->envelopeVol & mask) >= channels->envelopeGoal) {
                    envelope_decay_start:
                        channels->status--;
                        channels->envelopeCounter = channels->decay;
                        if ((uint8_t)(channels->envelopeCounter & mask)) {
                            channels->cgbStatus |= CGB_CHANNEL_MO_VOL;
                            channels->envelopeVol = channels->envelopeGoal;
                            if (ch != 3)
                                envelopeStepTimeAndDir = channels->decay | CGB_NRx2_ENV_DIR_DEC;
                        } else {
                            goto envelope_sustain_start;
                        }
                    } else {
                        channels->envelopeCounter = channels->attack;
                    }
                }
            }
        }

    envelope_step_complete:
        // every 15 frames, envelope calculation has to be done twice
        // to keep up with the hardware envelope rate (1/64 s)
        channels->envelopeCounter--;
        if (prevC15 == 0) {
            prevC15--;
            goto envelope_step_repeat;
        }

    envelope_complete:
        /* 3. apply pitch to HW registers */
        if (channels->cgbStatus & CGB_CHANNEL_MO_PIT) {
            if (ch < 4 && (channels->type & TONEDATA_TYPE_FIX)) {
                int dac_pwm_rate = REG_SOUNDBIAS_H;

                if (dac_pwm_rate < 0x40)  // if PWM rate = 32768 Hz
                    channels->freq = (channels->freq + 2) & 0x7fc;
                else if (dac_pwm_rate < 0x80)  // if PWM rate = 65536 Hz
                    channels->freq = (channels->freq + 1) & 0x7fe;
            }

            if (ch != 4)
                *nrx3ptr = channels->freq;
            else
                *nrx3ptr = (*nrx3ptr & 0x08) | channels->freq;
            channels->nrx4 = (channels->nrx4 & 0xC0) + (*((uint8_t *)(&channels->freq) + 1));
            *nrx4ptr = (int8_t)(channels->nrx4 & mask);
        }

        /* 4. apply envelope & volume to HW registers */
        if (channels->cgbStatus & CGB_CHANNEL_MO_VOL) {
            REG_NR51 = (REG_NR51 & ~channels->panMask) | channels->pan;
            if (ch == 3) {
                *nrx2ptr = gCgb3Vol[channels->envelopeVol];
                if (channels->nrx4 & 0x80) {
                    *nrx0ptr = 0x80;
                    *nrx4ptr = channels->nrx4;
                    channels->nrx4 &= 0x7f;
                }
            } else {
                envelopeStepTimeAndDir &= 0xf;
                *nrx2ptr = (channels->envelopeVol << 4) + envelopeStepTimeAndDir;
                *nrx4ptr = channels->nrx4 | 0x80;
                if (ch == 1 && !(*nrx0ptr & 0x08))
                    *nrx4ptr = channels->nrx4 | 0x80;
            }
            cgb_set_envelope(ch - 1, *nrx2ptr);
            cgb_toggle_length(ch - 1, (*nrx4ptr & 0x40));
            cgb_trigger_note(ch - 1);
        }

    channel_complete:
        channels->cgbStatus = 0;
    }
}

void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, uint16_t tempo) {
    mplayInfo->tempoScale = tempo;
    mplayInfo->tempoInterval = (mplayInfo->tempo * mplayInfo->tempoScale) >> 8;
}

void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint16_t volume) {
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->volX = volume / 4;
                track->status |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }
}

void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int16_t pitch) {
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->keyShiftX = pitch >> 8;
                track->pitchX = pitch;
                track->status |= MPT_FLG_PITCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }
}

void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, int8_t pan) {
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->panX = pan;
                track->status |= MPT_FLG_VOLCHG;
            }
        }

        i--;
        track++;
        bit <<= 1;
    }
}

void ClearModM(struct MusicPlayerTrack *track) {
    track->lfoSpeedCtr = 0;
    track->mod = 0;

    if (track->modType == 0)
        track->status |= MPT_FLG_PITCHG;
    else
        track->status |= MPT_FLG_VOLCHG;
}

void m4aMPlayModDepthSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t modDepth) {
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->modDepth = modDepth;

                if (!track->modDepth)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }
}

void m4aMPlayLFOSpeedSet(struct MusicPlayerInfo *mplayInfo, uint16_t trackBits, uint8_t lfoSpeed) {
    int32_t i;
    uint32_t bit;
    struct MusicPlayerTrack *track;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->lfoSpeed = lfoSpeed;

                if (!track->lfoSpeed)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }
}

#define MEMACC_COND_JUMP(cond) \
    if (cond)                  \
        goto cond_true;        \
    else                       \
        goto cond_false;

void ply_memacc(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    uint32_t op;
    uint8_t *addr;
    uint8_t data;

    op = *track->cmdPtr;
    track->cmdPtr++;

    addr = gMPlayMemAccArea + *track->cmdPtr;
    track->cmdPtr++;

    data = *track->cmdPtr;
    track->cmdPtr++;

    switch (op) {
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
            *addr = gMPlayMemAccArea[data];
            return;
        case 4:
            *addr += gMPlayMemAccArea[data];
            return;
        case 5:
            *addr -= gMPlayMemAccArea[data];
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
            MEMACC_COND_JUMP(*addr == gMPlayMemAccArea[data])
            return;
        case 13:
            MEMACC_COND_JUMP(*addr != gMPlayMemAccArea[data])
            return;
        case 14:
            MEMACC_COND_JUMP(*addr > gMPlayMemAccArea[data])
            return;
        case 15:
            MEMACC_COND_JUMP(*addr >= gMPlayMemAccArea[data])
            return;
        case 16:
            MEMACC_COND_JUMP(*addr <= gMPlayMemAccArea[data])
            return;
        case 17:
            MEMACC_COND_JUMP(*addr < gMPlayMemAccArea[data])
            return;
        default:
            return;
    }

cond_true : {
    MP2K_event_goto(mplayInfo, track);
    return;
}

cond_false:
    track->cmdPtr += 4;
}

void ply_xcmd(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    uint32_t n = *track->cmdPtr;
    track->cmdPtr++;

    gXcmdTable[n](mplayInfo, track);
}

void ply_xxx(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    MP2K_event_fine(mplayInfo, track);
}

#define READ_XCMD_BYTE(var, n)              \
    {                                       \
        uint32_t byte = track->cmdPtr[(n)]; \
        byte <<= n * 8;                     \
        (var) &= ~(0xFF << (n * 8));        \
        (var) |= byte;                      \
    }

void ply_xwave(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    uint32_t wav;

    READ_XCMD_BYTE(wav, 0)  // UB: uninitialized variable
    READ_XCMD_BYTE(wav, 1)
    READ_XCMD_BYTE(wav, 2)
    READ_XCMD_BYTE(wav, 3)

    track->instrument.wav = (struct WaveData *)wav;

    track->cmdPtr += 4;
}

void ply_xtype(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.type = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xatta(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.attack = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xdeca(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.decay = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xsust(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.sustain = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xrele(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.release = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xiecv(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->echoVol = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xiecl(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->echoLen = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xleng(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.length = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xswee(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    track->instrument.panSweep = *track->cmdPtr;
    track->cmdPtr++;
}

void ply_xcmd_0C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    uint32_t unk;

    READ_XCMD_BYTE(unk, 0)  // UB: uninitialized variable
    READ_XCMD_BYTE(unk, 1)

    if (track->unk_3A < (uint16_t)unk) {
        track->unk_3A++;
        track->cmdPtr -= 2;
        track->wait = 1;
    } else {
        track->unk_3A = 0;
        track->cmdPtr += 2;
    }
}

void ply_xcmd_0D(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track) {
    uint32_t unk;
#ifdef UBFIX
    unk = 0;
#endif

    READ_XCMD_BYTE(unk, 0)  // UB: uninitialized variable
    READ_XCMD_BYTE(unk, 1)
    READ_XCMD_BYTE(unk, 2)
    READ_XCMD_BYTE(unk, 3)

    track->count = unk;
    track->cmdPtr += 4;
}

#define FLOOR_DIV_POW2(a, b) ((a) > 0 ? (a) / (b) : (((a) + 1 - (b)) / (b)))

extern void *const gMPlayJumpTable[];
extern const uint8_t gScaleTable[];
extern const uint32_t gFreqTable[];
extern const uint8_t gClockTable[];

uint32_t umul3232H32(uint32_t a, uint32_t b) {
    uint64_t result = a;
    result *= b;
    return result >> 32;
}

// Removes chan from the doubly-linked list of channels associated with chan->track.
// Gonna rename this to like "FreeChannel" or something, similar to VGMS
void MP2KClearChain(struct SoundChannel *chan) {
    struct MusicPlayerTrack *track = chan->track;
    if (chan->track == NULL) {
        return;
    }
    struct SoundChannel *next = chan->next;
    struct SoundChannel *prev = chan->prev;

    if (prev != NULL) {
        prev->next = next;
    } else {
        track->chan = next;
    }

    if (next != NULL) {
        next->prev = prev;
    }

    chan->track = NULL;
}

uint8_t ConsumeTrackByte(struct MusicPlayerTrack *track) {
    uint8_t byte = *track->cmdPtr;
    track->cmdPtr++;
    return byte;
}

// Ends the current track. (Fine as in the Italian musical word, not English)
void MP2K_event_fine(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    struct MusicPlayerTrack *r5 = track;
    for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->next) {
        if (chan->status & 0xC7) {
            chan->status |= 0x40;
        }
        MP2KClearChain(chan);
    }
    track->status = 0;
}

// Sets the track's cmdPtr to the specified address.
void MP2K_event_goto(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->cmdPtr = *(uint32_t *)track->cmdPtr - track->offset;
    offsetPointer(&track->cmdPtr);
}

// Sets the track's cmdPtr to the specified address after backing up its current position.
void MP2K_event_patt(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    uint8_t level = track->patternLevel;
    if (level < 3) {
        track->patternStack[level] = track->cmdPtr + 4;  // sizeof(uint32_t);
        track->patternLevel++;
        MP2K_event_goto(unused, track);
    } else {
        // Stop playing this track, as an indication to the music programmer that they need to quit
        // nesting patterns so darn much.
        MP2K_event_fine(unused, track);
    }
}

// Marks the end of the current pattern, if there is one, by resetting the pattern to the
// most recently saved value.
void MP2K_event_pend(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    if (track->patternLevel != 0) {
        uint8_t index = --track->patternLevel;
        track->cmdPtr = track->patternStack[index];
    }
}

// Loops back until a REPT event has been reached the specified number of times
void MP2K_event_rept(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    if (*track->cmdPtr == 0) {
        // "Repeat 0 times" == loop forever
        track->cmdPtr++;
        MP2K_event_goto(unused, track);
    } else {
        uint8_t repeatCtr = ++track->repeatCtr;
        if (repeatCtr < ConsumeTrackByte(track)) {
            MP2K_event_goto(unused, track);
        } else {
            track->repeatCtr = 0;
            track->cmdPtr += 5;  // sizeof(uint8_t) + sizeof(uint32_t);
        }
    }
}

// Sets the note priority for new notes in this track.
void MP2K_event_prio(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->priority = ConsumeTrackByte(track);
}

// Sets the BPM of all tracks to the specified tempo (in beats per half-minute, because 255 as a max tempo
// kinda sucks but 510 is plenty).
void MP2K_event_tempo(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    uint16_t bpm = ConsumeTrackByte(track);
    bpm *= 2;
    player->tempo = bpm;
    player->tempoInterval = (bpm * player->tempoScale) / 256;
}

void MP2K_event_keysh(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->keyShift = ConsumeTrackByte(track);
    track->status |= 0xC;
}

void MP2K_event_voice(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    uint8_t voice = *(track->cmdPtr++);
    struct ToneData *instrument = &player->instrument[voice];
    track->instrument = *instrument;
}

void MP2K_event_vol(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->vol = ConsumeTrackByte(track);
    track->status |= 0x3;
}

void MP2K_event_pan(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->pan = ConsumeTrackByte(track) - 0x40;
    track->status |= 0x3;
}

void MP2K_event_bend(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->bend = ConsumeTrackByte(track) - 0x40;
    track->status |= 0xC;
}

void MP2K_event_bendr(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->bendRange = ConsumeTrackByte(track);
    track->status |= 0xC;
}

void MP2K_event_lfodl(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->lfoDelay = ConsumeTrackByte(track);
}

void MP2K_event_modt(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    uint8_t type = ConsumeTrackByte(track);
    if (type != track->modType) {
        track->modType = type;
        track->status |= 0xF;
    }
}

void MP2K_event_tune(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->tune = ConsumeTrackByte(track) - 0x40;
    track->status |= 0xC;
}

void MP2K_event_port(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    // I'm really curious whether any games actually use this event...
    // I assume anything done by this command will get immediately overwritten by cgbMixerFunc?
    track->cmdPtr += 2;
}

void MP2KPlayerMain(void *voidPtrPlayer) {
    struct MusicPlayerInfo *player = (struct MusicPlayerInfo *)voidPtrPlayer;
    struct SoundInfo *mixer = SOUND_INFO_PTR;

    if (player->status & MUSICPLAYER_STATUS_PAUSE) return;
    FadeOutBody(voidPtrPlayer);
    if (player->status & MUSICPLAYER_STATUS_PAUSE) return;

    player->tempoCtr += player->tempoInterval;
    while (player->tempoCtr >= 150) {
        uint16_t trackBits = 0;

        for (uint32_t i = 0; i < player->trackCount; i++) {
            struct MusicPlayerTrack *currentTrack = player->tracks + i;
            struct SoundChannel *chan;
            if ((currentTrack->status & MPT_FLG_EXIST) == 0) {
                continue;
            }
            trackBits |= (1 << i);

            chan = currentTrack->chan;
            while (chan != NULL) {
                if ((chan->status & SOUND_CHANNEL_SF_ON) == 0) {
                    ClearChain(chan);
                } else if (chan->gateTime != 0 && --chan->gateTime == 0) {
                    chan->status |= SOUND_CHANNEL_SF_STOP;
                }
                chan = chan->next;
            }

            if (currentTrack->status & MPT_FLG_START) {
                currentTrack->status = MPT_FLG_EXIST;
                currentTrack->bendRange = 2;
                currentTrack->volX = 64;
                currentTrack->lfoSpeed = 22;
                currentTrack->instrument.type = 1;
            }

            while (currentTrack->wait == 0) {
                uint8_t event = *currentTrack->cmdPtr;
                if (event < 0x80) {
                    event = currentTrack->runningStatus;
                } else {
                    currentTrack->cmdPtr++;
                    if (event >= 0xBD) {
                        currentTrack->runningStatus = event;
                    }
                }

                if (event >= 0xCF) {
                    MP2K_event_nxx(event - 0xCF, player, currentTrack);
                } else if (event >= 0xB1) {
                    void (*eventFunc)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
                    player->cmd = event - 0xB1;
                    eventFunc = gMPlayJumpTable[player->cmd];
                    eventFunc(player, currentTrack);

                    if (currentTrack->status == 0) {
                        goto nextTrack;
                    }
                } else {
                    currentTrack->wait = gClockTable[event - 0x80];
                }
            }

            currentTrack->wait--;
            if (currentTrack->lfoSpeed != 0 && currentTrack->modDepth != 0) {
                if (currentTrack->lfoDelayCtr != 0U) {
                    currentTrack->lfoDelayCtr--;
                    goto nextTrack;
                }

                currentTrack->lfoSpeedCtr += currentTrack->lfoSpeed;

                int8_t r;
                if (currentTrack->lfoSpeedCtr >= 0x40U && currentTrack->lfoSpeedCtr < 0xC0U) {
                    r = 128 - currentTrack->lfoSpeedCtr;
                } else if (currentTrack->lfoSpeedCtr >= 0xC0U) {
                    // Unsigned -> signed casts where the value is out of range are implementation defined.
                    // Why not add a few extra lines to make behavior the same for literally everyone?
                    r = currentTrack->lfoSpeedCtr - 256;
                } else {
                    r = currentTrack->lfoSpeedCtr;
                }
                r = FLOOR_DIV_POW2(currentTrack->modDepth * r, 64);

                if (r != currentTrack->mod) {
                    currentTrack->mod = r;
                    if (currentTrack->modType == 0) {
                        currentTrack->status |= MPT_FLG_PITCHG;
                    } else {
                        currentTrack->status |= MPT_FLG_VOLCHG;
                    }
                }
            }

        nextTrack:;
        }

        player->clock++;
        if (trackBits == 0) {
            player->status = MUSICPLAYER_STATUS_PAUSE;
            return;
        }
        player->status = trackBits;
        player->tempoCtr -= 150;
    }

    uint32_t i = 0;

    do {
        struct MusicPlayerTrack *track = player->tracks + i;

        if ((track->status & MPT_FLG_EXIST) == 0 || (track->status & 0xF) == 0) {
            continue;
        }
        TrkVolPitSet(player, track);
        for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->next) {
            if ((chan->status & 0xC7) == 0) {
                ClearChain(chan);
                continue;
            }
            uint8_t cgbType = chan->type & 0x7;
            if (track->status & MPT_FLG_VOLCHG) {
                ChnVolSetAsm(chan, track);
                if (cgbType != 0) {
                    chan->cgbStatus |= 1;
                }
            }
            if (track->status & MPT_FLG_PITCHG) {
                int32_t key = chan->key + track->keyShiftCalc;
                if (key < 0) {
                    key = 0;
                }
                if (cgbType != 0) {
                    chan->freq = cgbCalcFreqFunc(cgbType, key, track->pitchM);
                    chan->cgbStatus |= 0x2;
                } else {
                    chan->freq = MidiKeyToFreq(chan->wav, key, track->pitchM);
                }
            }
        }
        track->status &= ~0xF;
    } while (++i < player->trackCount);
    cgbMixerFunc();
}

void TrackStop(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    if (track->status & 0x80) {
        for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->next) {
            if (chan->status != 0) {
                uint8_t cgbType = chan->type & 0x7;
                if (cgbType != 0) {
                    struct SoundInfo *mixer = SOUND_INFO_PTR;
                    cgbNoteOffFunc(cgbType);
                }
                chan->status = 0;
            }
            chan->track = NULL;
        }
        track->chan = NULL;
    }
}

void ChnVolSetAsm(struct SoundChannel *chan, struct MusicPlayerTrack *track) {
    int8_t forcedPan = chan->rhythmPan;
    uint32_t rightVol = (uint8_t)(forcedPan + 128) * chan->velocity * track->volMR / 128 / 128;
    if (rightVol > 0xFF) {
        rightVol = 0xFF;
    }
    chan->rightVol = rightVol;

    uint32_t leftVol = (uint8_t)(127 - forcedPan) * chan->velocity * track->volML / 128 / 128;
    if (leftVol > 0xFF) {
        leftVol = 0xFF;
    }
    chan->leftVol = leftVol;
}

void MP2K_event_nxx(uint8_t clock, struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {  // ply_note
    struct SoundInfo *mixer = SOUND_INFO_PTR;
    // A note can be anywhere from 1 to 4 bytes long. First is always the note length...
    track->gateTime = gClockTable[clock];
    if (*track->cmdPtr < 0x80) {
        // Then the note name...
        track->key = *(track->cmdPtr++);
        if (*track->cmdPtr < 0x80) {
            // Then the velocity...
            track->velocity = *(track->cmdPtr++);
            if (*track->cmdPtr < 0x80) {
                // Then a number to add ticks to get exotic or more precise note lengths without TIE.
                track->gateTime += *(track->cmdPtr++);
            }
        }
    }

    // sp14
    int8_t forcedPan = 0;
    // First r4, then r9
    struct ToneData *instrument = &track->instrument;
    // sp8
    uint8_t key = track->key;
    uint8_t type = instrument->type;

    if (type & (TONEDATA_TYPE_RHY | TONEDATA_TYPE_SPL)) {
        uint8_t instrumentIndex;
        if (type & TONEDATA_TYPE_SPL) {
            uint8_t *keySplitTableOffset = instrument->keySplitTable;
            offsetPointer(&keySplitTableOffset);
            instrumentIndex = keySplitTableOffset[track->key];
        } else {
            instrumentIndex = track->key;
        }

        instrument = instrument->group + (instrumentIndex * 12);
        offsetPointer(&instrument);
        if (instrument->type & (TONEDATA_TYPE_RHY | TONEDATA_TYPE_SPL)) {
            return;
        }
        if (type & TONEDATA_TYPE_RHY) {
            if (instrument->panSweep & 0x80) {
                forcedPan = ((int8_t)(instrument->panSweep & 0x7F) - 0x40) * 2;
            }
            key = instrument->key;
        }
    }

    // sp10
    uint_fast16_t priority = player->priority + track->priority;
    if (priority > 0xFF) {
        priority = 0xFF;
    }

    uint8_t cgbType = instrument->type & TONEDATA_TYPE_CGB;
    struct SoundChannel *chan;

    if (cgbType != 0) {
        if (mixer->cgbChans == NULL) {
            return;
        }
        // There's only one SoundChannel of a given type, so we don't need to loop to find it.
        chan = mixer->cgbChans + cgbType - 1;

        // If this channel is running and not stopped,
        if ((chan->status & SOUND_CHANNEL_SF_ON) && (chan->status & SOUND_CHANNEL_SF_STOP) == 0) {
            // then make sure this note is higher priority (or same priority but from a later track).
            if (chan->priority > priority || (chan->priority == priority && chan->track < track)) {
                return;
            }
        }
    } else {
        uint_fast16_t p = priority;
        struct MusicPlayerTrack *t = track;
        uint32_t foundStoppingChannel = false;
        chan = NULL;
        uint8_t numChans = mixer->numChans;
        struct SoundChannel *currChan = mixer->chans;

        for (uint_fast8_t i = 0; i < numChans; i++, currChan++) {
            if ((currChan->status & SOUND_CHANNEL_SF_ON) == 0) {
                // Hey, we found a completely inactive channel! Let's use that.
                chan = currChan;
                break;
            }

            if (currChan->status & SOUND_CHANNEL_SF_STOP && !foundStoppingChannel) {
                // In the absence of a completely finalized channel, we can take over one that's about to
                // finalize. That's a tier above any channel that's currently playing a note.
                foundStoppingChannel = true;
                p = currChan->priority;
                t = currChan->track;
                chan = currChan;
            } else if ((currChan->status & SOUND_CHANNEL_SF_STOP && foundStoppingChannel) || ((currChan->status & SOUND_CHANNEL_SF_STOP) == 0 && !foundStoppingChannel)) {
                // The channel we're checking is on the same tier, so check the priority and track order
                if (currChan->priority < p) {
                    p = currChan->priority;
                    t = currChan->track;
                    chan = currChan;
                } else if (currChan->priority == p && currChan->track > t) {
                    t = currChan->track;
                    chan = currChan;
                } else if (currChan->priority == p && currChan->track == t) {
                    chan = currChan;
                }
            }
        }
    }

    if (chan == NULL) {
        return;
    }
    ClearChain(chan);

    chan->prev = NULL;
    chan->next = track->chan;
    if (track->chan != NULL) {
        track->chan->prev = chan;
    }
    track->chan = chan;
    chan->track = track;

    track->lfoDelayCtr = track->lfoDelay;
    if (track->lfoDelay != 0) {
        ClearModM(track);
    }
    TrkVolPitSet(player, track);

    chan->gateTime = track->gateTime;
    chan->midiKey = track->key;
    chan->velocity = track->velocity;
    chan->priority = priority;
    chan->key = key;
    chan->rhythmPan = forcedPan;
    chan->type = instrument->type;
    chan->wav = instrument->wav;
    offsetPointer(&chan->wav);
    chan->attack = instrument->attack;
    chan->decay = instrument->decay;
    chan->sustain = instrument->sustain;
    chan->release = instrument->release;
    chan->echoVol = track->echoVol;
    chan->echoLen = track->echoLen;
    ChnVolSetAsm(chan, track);

    // Avoid promoting keyShiftCalc to uint8_t by splitting the addition into a separate statement
    int_fast16_t transposedKey = chan->key;
    transposedKey += track->keyShiftCalc;
    if (transposedKey < 0) {
        transposedKey = 0;
    }

    if (cgbType != 0) {
        chan->length = instrument->length;
        if (instrument->panSweep & 0x80 || (instrument->panSweep & 0x70) == 0) {
            chan->sweep = 8;
        } else {
            chan->sweep = instrument->panSweep;
        }

        chan->freq = cgbCalcFreqFunc(cgbType, transposedKey, track->pitchM);
    } else {
        chan->freq = MidiKeyToFreq(chan->wav, transposedKey, track->pitchM);
    }

    chan->status = SOUND_CHANNEL_SF_START;
    track->status &= ~0xF;
}

void MP2K_event_endtie(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    uint8_t key = *track->cmdPtr;
    if (key < 0x80) {
        track->key = key;
        track->cmdPtr++;
    } else {
        key = track->key;
    }

    struct SoundChannel *chan = track->chan;
    while (chan != NULL) {
        if (chan->status & 0x83 && (chan->status & 0x40) == 0 && chan->midiKey == key) {
            chan->status |= 0x40;
            return;
        }
        chan = chan->next;
    }
}

void MP2K_event_lfos(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->lfoSpeed = *(track->cmdPtr++);
    if (track->lfoSpeed == 0) {
        ClearModM(track);
    }
}

void MP2K_event_mod(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->modDepth = *(track->cmdPtr++);
    if (track->modDepth == 0) {
        ClearModM(track);
    }
}

struct AudioCGB gb;
float soundChannelPos[4];
const int16_t *PU1Table;
const int16_t *PU2Table;
uint32_t apuFrame;
uint8_t apuCycle;
uint32_t sampleRate;
uint16_t lfsrMax[2];
float ch4Samples;

void cgb_audio_init(uint32_t rate) {
    gb.ch1Freq = 0;
    gb.ch1SweepCounter = 0;
    gb.ch1SweepCounterI = 0;
    gb.ch1SweepDir = 0;
    gb.ch1SweepShift = 0;
    for (uint8_t ch = 0; ch < 4; ch++) {
        gb.Vol[ch] = 0;
        gb.VolI[ch] = 0;
        gb.Len[ch] = 0;
        gb.LenI[ch] = 0;
        gb.LenOn[ch] = 0;
        gb.EnvCounter[ch] = 0;
        gb.EnvCounterI[ch] = 0;
        gb.EnvDir[ch] = 0;
        gb.DAC[ch] = 0;
        soundChannelPos[ch] = 0;
    }
    soundChannelPos[1] = 1;
    PU1Table = PU0;
    PU2Table = PU0;
    sampleRate = rate;
    gb.ch4LFSR[0] = 0x8000;
    gb.ch4LFSR[1] = 0x80;
    lfsrMax[0] = 0x8000;
    lfsrMax[1] = 0x80;
    ch4Samples = 0.0f;
}

void cgb_set_sweep(uint8_t sweep) {
    gb.ch1SweepDir = (sweep & 0x08) >> 3;
    gb.ch1SweepCounter = gb.ch1SweepCounterI = (sweep & 0x70) >> 4;
    gb.ch1SweepShift = (sweep & 0x07);
}

void cgb_set_wavram() {
    for (uint8_t wavi = 0; wavi < 0x10; wavi++) {
        gb.WAVRAM[(wavi << 1)] = (((*(REG_ADDR_WAVE_RAM0 + wavi)) & 0xF0) >> 4) / 7.5f - 1.0f;
        gb.WAVRAM[(wavi << 1) + 1] = (((*(REG_ADDR_WAVE_RAM0 + wavi)) & 0x0F)) / 7.5f - 1.0f;
    }
}

void cgb_toggle_length(uint8_t channel, uint8_t state) {
    gb.LenOn[channel] = state;
}

void cgb_set_length(uint8_t channel, uint8_t length) {
    gb.Len[channel] = gb.LenI[channel] = length;
}

void cgb_set_envelope(uint8_t channel, uint8_t envelope) {
    if (channel == 2) {
        switch ((envelope & 0xE0)) {
            case 0x00:  // mute
                gb.Vol[2] = gb.VolI[2] = 0;
                break;
            case 0x20:  // full
                gb.Vol[2] = gb.VolI[2] = 4;
                break;
            case 0x40:  // half
                gb.Vol[2] = gb.VolI[2] = 2;
                break;
            case 0x60:  // quarter
                gb.Vol[2] = gb.VolI[2] = 1;
                break;
            case 0x80:  // 3 quarters
                gb.Vol[2] = gb.VolI[2] = 3;
                break;
        }
    } else {
        gb.DAC[channel] = (envelope & 0xF8) > 0;
        gb.Vol[channel] = gb.VolI[channel] = (envelope & 0xF0) >> 4;
        gb.EnvDir[channel] = (envelope & 0x08) >> 3;
        gb.EnvCounter[channel] = gb.EnvCounterI[channel] = (envelope & 0x07);
    }
}

void cgb_trigger_note(uint8_t channel) {
    gb.Vol[channel] = gb.VolI[channel];
    gb.Len[channel] = gb.LenI[channel];
    if (channel != 2) gb.EnvCounter[channel] = gb.EnvCounterI[channel];
    if (channel == 3) {
        gb.ch4LFSR[0] = 0x8000;
        gb.ch4LFSR[1] = 0x80;
    }
}
