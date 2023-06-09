#include <stddef.h>
#include <stdint.h>
#include "mp2k_common.h"
#include "m4a_internal.h"

static uint32_t MidiKeyToFreq(struct WaveData *wav, uint8_t key, uint8_t pitch);
extern void * const gMPlayJumpTableTemplate[];
extern const uint8_t gScaleTable[];
extern const uint32_t gFreqTable[];
extern const uint8_t gClockTable[];
extern struct SoundMixerState *SOUND_INFO_PTR;
uint32_t umul3232H32(uint32_t a, uint32_t b) {
    uint64_t result = a;
    result *= b;
    return result >> 32;
}

void SoundMainBTM(void *ptr)
{
    //CpuFill32(0, ptr, 0x40);
}

// Removes chan from the doubly-linked list of channels associated with chan->track.
// Gonna rename this to like "FreeChannel" or something, similar to VGMS
void MP2KClearChain(struct SoundChannel *chan) {
    struct MusicPlayerTrack *track = chan->track;
    if (chan->track == NULL) {
        return;
    }
    struct SoundChannel *nextChannelPointer = chan->nextChannelPointer;
    struct SoundChannel *prevChannelPointer = chan->prevChannelPointer;
    
    if (prevChannelPointer != NULL) {
        prevChannelPointer->nextChannelPointer = nextChannelPointer;
    } else {
        track->chan = nextChannelPointer;
    }
    
    if (nextChannelPointer != NULL) {
        nextChannelPointer->prevChannelPointer = prevChannelPointer;
    }
    
    chan->track = NULL;
}

#define VERIFY_PTR(x) do; while (0)

static uint8_t SafeDereferenceU8(uint8_t *addr) {
    uint8_t ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static uint32_t SafeDereferenceU32(uint32_t *addr) {
    uint32_t ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static uint8_t *SafeDereferenceU8Ptr(uint8_t **addr) {
    uint8_t *ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static uint32_t *SafeDereferenceU32Ptr(uint32_t **addr) {
    uint32_t *ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static struct WaveData *SafeDereferenceWavDataPtr(struct WaveData **addr) {
    struct WaveData *ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static struct ToneData *SafeDereferenceMP2KInstrumentPtr(struct ToneData **addr) {
    struct ToneData *ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

static void *SafeDereferenceVoidPtr(void **addr) {
    void *ret = *addr;
    VERIFY_PTR(addr);
    return ret;
}

#undef VERIFY_PTR

uint8_t ConsumeTrackByte(struct MusicPlayerTrack *track) {
    uint8_t *ptr = track->cmdPtr++;
    return SafeDereferenceU8(ptr);
}

void MPlayJumpTableCopy(void **mplayJumpTable) {
    for (uint_fast8_t i = 0; i < 36; i++) {
        mplayJumpTable[i] = SafeDereferenceVoidPtr(&gMPlayJumpTableTemplate[i]);
    }
}

// Ends the current track. (Fine as in the Italian musical word, not English)
void MP2K_event_fine(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    struct MusicPlayerTrack *r5 = track;
    for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->nextChannelPointer) {
        if (chan->statusFlags & 0xC7) {
            chan->statusFlags |= 0x40;
        }
        MP2KClearChain(chan);
    }
    track->flags = 0;
}

// Sets the track's cmdPtr to the specified address.
void MP2K_event_goto(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->cmdPtr = *(uint32_t *)track->cmdPtr;
    offsetPointer(&track->cmdPtr);
}

// Sets the track's cmdPtr to the specified address after backing up its current position.
void MP2K_event_patt(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    uint8_t level = track->patternLevel;
    if (level < 3) {
        track->patternStack[level] = track->cmdPtr + 4;  // sizeof(uint8_t *);
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
        uint8_t repeatCount = ++track->repeatCount;
        if (repeatCount < ConsumeTrackByte(track)) {
            MP2K_event_goto(unused, track);
        } else {
            track->repeatCount = 0;
            track->cmdPtr += sizeof(uint8_t) + sizeof(uint8_t *);
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
    player->tempoRawBPM = bpm;
    player->tempoInterval = (bpm * player->tempoScale) / 256;
}

void MP2K_event_keysh(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->keyShift = ConsumeTrackByte(track);
    track->flags |= 0xC;
}

void MP2K_event_voice(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    uint8_t voice = *(track->cmdPtr++);
    struct ToneData *instrument = &player->voicegroup[voice];
    track->instrument = *instrument;
}

void MP2K_event_vol(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->vol = ConsumeTrackByte(track);
    track->flags |= 0x3;
}

void MP2K_event_pan(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->pan = ConsumeTrackByte(track) - 0x40;
    track->flags |= 0x3;
}

void MP2K_event_bend(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->bend = ConsumeTrackByte(track) - 0x40;
    track->flags |= 0xC;
}

void MP2K_event_bendr(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->bendRange = ConsumeTrackByte(track);
    track->flags |= 0xC;
}

void MP2K_event_lfodl(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->lfoDelay = ConsumeTrackByte(track);
}

void MP2K_event_modt(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    uint8_t type = ConsumeTrackByte(track);
    if (type != track->modType) {
        track->modType = type;
        track->flags |= 0xF;
    }
}

void MP2K_event_tune(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    track->tune = ConsumeTrackByte(track) - 0x40;
    track->flags |= 0xC;
}

void MP2K_event_port(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track) {
    // I'm really curious whether any games actually use this event...
    // I assume anything done by this command will get immediately overwritten by cgbMixerFunc?
    track->cmdPtr += 2;
}

void MP2KPlayerMain(void *voidPtrPlayer) {
    struct MusicPlayerInfo *player = (struct MusicPlayerInfo *)voidPtrPlayer;
    struct SoundMixerState *mixer = SOUND_INFO_PTR;
    
    if (player->nextPlayerFunc != NULL) {
        player->nextPlayerFunc(player->nextPlayer);
    }
    
    if (player->status & MUSICPLAYER_STATUS_PAUSE) {
        return;
    }
    FadeOutBody(voidPtrPlayer);
    if (player->status & MUSICPLAYER_STATUS_PAUSE) {
        return;
    }
    
    player->tempoCounter += player->tempoInterval;
    while (player->tempoCounter >= 150) {
        uint16_t trackBits = 0;
        
        for (uint32_t i = 0; i < player->trackCount; i++) {
            struct MusicPlayerTrack *currentTrack = player->tracks + i;
            struct SoundChannel *chan;
            if ((currentTrack->flags & MPT_FLG_EXIST) == 0) {
                continue;
            }
            trackBits |= (1 << i);
            
            chan = currentTrack->chan;
            while (chan != NULL) {
                if ((chan->statusFlags & SOUND_CHANNEL_SF_ON) == 0) {
                    ClearChain(chan);
                } else if (chan->gateTime != 0 && --chan->gateTime == 0) {
                    chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
                }
                chan = chan->nextChannelPointer;
            }
            
            if (currentTrack->flags & MPT_FLG_START) {
                //CpuFill32(0, currentTrack, 0x40);
                currentTrack->flags = MPT_FLG_EXIST;
                currentTrack->bendRange = 2;
                currentTrack->volPublic = 64;
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
                    mixer->mp2kEventNxxFunc(event - 0xCF, player, currentTrack);
                } else if (event >= 0xB1) {
                    void (*eventFunc)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
                    player->cmd = event - 0xB1;
                    eventFunc = mixer->mp2kEventFuncTable[player->cmd];
                    eventFunc(player, currentTrack);
                    
                    if (currentTrack->flags == 0) {
                        goto nextTrack;
                    }
                } else {
                    currentTrack->wait = gClockTable[event - 0x80];
                }
            }
            
            currentTrack->wait--;
            
            if (currentTrack->lfoSpeed != 0 && currentTrack->modDepth != 0) {
                if (currentTrack->lfoDelayCounter != 0U) {
                    currentTrack->lfoDelayCounter--;
                    goto nextTrack;
                }
                
                currentTrack->lfoSpeedCounter += currentTrack->lfoSpeed;
                
                int8_t r;
                if (currentTrack->lfoSpeedCounter >= 0x40U && currentTrack->lfoSpeedCounter < 0xC0U) {
                    r = 128 - currentTrack->lfoSpeedCounter;
                } else if (currentTrack->lfoSpeedCounter >= 0xC0U) {
                    // Unsigned -> signed casts where the value is out of range are implementation defined.
                    // Why not add a few extra lines to make behavior the same for literally everyone?
                    r = currentTrack->lfoSpeedCounter - 256;
                } else {
                    r = currentTrack->lfoSpeedCounter;
                }
                r = FLOOR_DIV_POW2(currentTrack->modDepth * r, 64);
                
                if (r != currentTrack->modCalculated) {
                    currentTrack->modCalculated = r;
                    if (currentTrack->modType == 0) {
                        currentTrack->flags |= MPT_FLG_PITCHG;
                    } else {
                        currentTrack->flags |= MPT_FLG_VOLCHG;
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
        player->tempoCounter -= 150;
    }
    
    uint32_t i = 0;

    do {
        struct MusicPlayerTrack *track = player->tracks + i;

        if ((track->flags & MPT_FLG_EXIST) == 0 || (track->flags & 0xF) == 0) {
            continue;
        }
        TrkVolPitSet(player, track);
        for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->nextChannelPointer) {
            if ((chan->statusFlags & 0xC7) == 0) {
                ClearChain(chan);
                continue;
            }
            uint8_t cgbType = chan->type & 0x7;
            if (track->flags & MPT_FLG_VOLCHG) {
                ChnVolSetAsm(chan, track);
                if (cgbType != 0) {
                    chan->cgbStatus |= 1;
                }
            }
            if (track->flags & MPT_FLG_PITCHG) {
                int32_t key = chan->key + track->keyShiftCalculated;
                if (key < 0) {
                    key = 0;
                }
                if (cgbType != 0) {
                    chan->freq = mixer->cgbCalcFreqFunc(cgbType, key, track->pitchCalculated);
                    chan->cgbStatus |= 0x2;
                } else {
                    chan->freq = MidiKeyToFreq(chan->wav, key, track->pitchCalculated);
                }
            }
        }
        track->flags &= ~0xF;
    }
    while(++i < player->trackCount);
}

void TrackStop(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    if (track->flags & 0x80) {
        for (struct SoundChannel *chan = track->chan; chan != NULL; chan = chan->nextChannelPointer) {
            if (chan->statusFlags != 0) {
                uint8_t cgbType = chan->type & 0x7;
                if (cgbType != 0) {
                    struct SoundMixerState *mixer = SOUND_INFO_PTR;
                    mixer->cgbNoteOffFunc(cgbType);
                }
                chan->statusFlags = 0;
            }
            chan->track = NULL;
        }
        track->chan = NULL;
    }
}

void ChnVolSetAsm(struct SoundChannel *chan, struct MusicPlayerTrack *track) {
    int8_t forcedPan = chan->rhythmPan;
    uint32_t rightVolume = (uint8_t)(forcedPan + 128) * chan->velocity * track->volRightCalculated / 128 / 128;
    if (rightVolume > 0xFF) {
        rightVolume = 0xFF;
    }
    chan->rightVolume = rightVolume;
    
    uint32_t leftVolume = (uint8_t)(127 - forcedPan) * chan->velocity * track->volLeftCalculated / 128 / 128;
    if (leftVolume > 0xFF) {
        leftVolume = 0xFF;
    }
    chan->leftVolume = leftVolume;
}

void MP2K_event_nxx(uint8_t clock, struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) {
    struct SoundMixerState *mixer = SOUND_INFO_PTR;
    
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
        if (instrument->type & TONEDATA_TYPE_SPL) {
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
            key = instrument->drumKey;
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
        // There's only one CgbChannel of a given type, so we don't need to loop to find it.
        chan = mixer->cgbChans + cgbType - 1;
        
        // If this channel is running and not stopped,
        if ((chan->statusFlags & SOUND_CHANNEL_SF_ON) 
        && (chan->statusFlags & SOUND_CHANNEL_SF_STOP) == 0) {
            // then make sure this note is higher priority (or same priority but from a later track).
            if (chan->priority > priority || (chan->priority == priority && chan->track < track)) {
                return;
            }
        }
    } else {
        uint_fast16_t p = priority;
        struct MusicPlayerTrack *t = track;
        uint32_t foundStoppingChannel = 0;
        chan = NULL;
        uint8_t maxChans = mixer->numChans;
        struct SoundChannel *currChan = mixer->chans;
        
        for (uint_fast8_t i = 0; i < maxChans; i++, currChan++) {
            if ((currChan->statusFlags & SOUND_CHANNEL_SF_ON) == 0) {
                // Hey, we found a completely inactive channel! Let's use that.
                chan = currChan;
                break;
            }
            
            if (currChan->statusFlags & SOUND_CHANNEL_SF_STOP && !foundStoppingChannel) {
                // In the absence of a completely finalized channel, we can take over one that's about to
                // finalize. That's a tier above any channel that's currently playing a note.
                foundStoppingChannel = 1;
                p = currChan->priority;
                t = currChan->track;
                chan = currChan;
            } else if ((currChan->statusFlags & SOUND_CHANNEL_SF_STOP && foundStoppingChannel)
                   || ((currChan->statusFlags & SOUND_CHANNEL_SF_STOP) == 0 && !foundStoppingChannel)) {
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
    
    chan->prevChannelPointer = NULL;
    chan->nextChannelPointer = track->chan;
    if (track->chan != NULL) {
        track->chan->prevChannelPointer = chan;
    }
    track->chan = chan;
    chan->track = track;
    
    track->lfoDelayCounter = track->lfoDelay;
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
    chan->echoVolume = track->echoVolume;
    chan->echoLength = track->echoLength;
    ChnVolSetAsm(chan, track);
    
    // Avoid promoting keyShiftCalculated to uint8_t by splitting the addition into a separate statement
    int_fast16_t transposedKey = chan->key;
    transposedKey += track->keyShiftCalculated;
    if (transposedKey < 0) {
        transposedKey = 0;
    }
    
    if (cgbType != 0) {
        //struct CgbChannel *cgbChan = (struct CgbChannel *)chan;
        chan->length = instrument->length;
        if (instrument->panSweep & 0x80 || (instrument->panSweep & 0x70) == 0) {
            chan->sweep = 8;
        } else {
            chan->sweep = instrument->panSweep;
        }
        
        chan->freq = mixer->cgbCalcFreqFunc(cgbType, transposedKey, track->pitchCalculated);
    } else {
        chan->count = track->count;
        chan->freq = MidiKeyToFreq(chan->wav, transposedKey, track->pitchCalculated);
    }
    
    chan->statusFlags = SOUND_CHANNEL_SF_START;
    track->flags &= ~0xF;
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
        if (chan->statusFlags & 0x83 && (chan->statusFlags & 0x40) == 0 && chan->midiKey == key) {
            chan->statusFlags |= 0x40;
            return;
        }
        chan = chan->nextChannelPointer;
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

void m4aSoundVSync(void)
{

}

#if 0
// In:
// - wav: pointer to sample
// - key: the note after being transposed. If pitch bend puts it between notes, then the note below.
// - pitch: how many 256ths of a semitone above `key` the current note is.
// Out:
// - The freq in Hz at which the sample should be played back.

uint32_t MidiKeyToFreq(struct WaveData *wav, uint8_t key, uint8_t pitch) {
    if (key > 178) {
        key = 178;
        pitch = 255;
    }
    
    // Alternatively, note = key % 12 and octave = 14 - (key / 12)
    uint8_t note = gScaleTable[key] & 0xF;
    uint8_t octave = gScaleTable[key] >> 4;
    uint8_t nextNote = gScaleTable[key + 1] & 0xF;
    uint8_t nextOctave = gScaleTable[key + 1] >> 4;
    
    uint32_t baseFreq1 = gFreqTable[note] >> octave;
    uint32_t baseFreq2 = gFreqTable[nextNote] >> nextOctave;
    
    uint32_t freqDifference = umul3232H32(baseFreq2 - baseFreq1, pitch << 24);
    // This is added by me. The real GBA and GBA BIOS don't verify this address, and as a result the
    // BIOS's memory can be dumped.
    uint32_t freq = SafeDereferenceU32(&wav->freq);
    return umul3232H32(freq, baseFreq1 + freqDifference);
}
#endif
