#ifdef __cplusplus
extern "C"{
#endif 

void m4aSoundInit(uint32_t freq, uint8_t *_music, uint32_t _songTableAddress);
void m4aSongNumStart(uint16_t n);
uint8_t RunMixerFrame(void *audioBuffer, int32_t samplesPerFrame);

#ifdef __cplusplus
}
#endif
