#ifndef GUARD_M4A_H
#define GUARD_M4A_H

#include "m4a_internal.h"

void m4aSoundVSync(void);
void m4aSoundVSyncOn(void);

void m4aSoundInit(void);
void m4aSoundMain(void);
void m4aSongNumStart(uint16_t n);
void m4aSongNumStartOrChange(uint16_t n);
void m4aSongNumStop(uint16_t n);
void m4aMPlayAllStop(void);
void m4aMPlayContinue(struct MusicPlayerInfo *mplayInfo);
void m4aMPlayFadeOut(struct MusicPlayerInfo *mplayInfo, uint16_t speed);
void m4aMPlayFadeOutTemporarily(struct MusicPlayerInfo *mplayInfo, uint16_t speed);
void m4aMPlayFadeIn(struct MusicPlayerInfo *mplayInfo, uint16_t speed);
void m4aMPlayImmInit(struct MusicPlayerInfo *mplayInfo);
void cgb_audio_init(uint32_t rate);
void cgb_set_sweep(uint8_t sweep);
void cgb_set_wavram();
void cgb_toggle_length(uint8_t channel, uint8_t state);
void cgb_set_length(uint8_t channel, uint8_t length);
void cgb_set_envelope(uint8_t channel, uint8_t envelope);
void cgb_trigger_note(uint8_t channel);

extern struct MusicPlayerInfo gMPlayInfo_BGM;
extern struct SoundInfo gSoundInfo;

#endif  // GUARD_M4A_H
