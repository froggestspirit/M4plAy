#ifdef __cplusplus
extern "C"{
#endif 

void m4aSoundInit(uint32_t freq);
void m4aSongNumStart(uint16_t n);
float *RunMixerFrame(uint16_t samplesPerFrame);

#ifdef __cplusplus
}
#endif
