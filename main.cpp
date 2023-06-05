#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <portaudio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "m4play.h"

#define MIXER_FREQ 42048
int song;
uint32_t songTableAddress;
unsigned char music[0x8000000];
FILE *musicFile = NULL;
char *filename;
uint8_t isRunning = 1;

typedef struct
{
    float left_phase;
    float right_phase;
} paTestData;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    // Cast data passed through stream to our structure.
    paTestData *data = (paTestData *)userData;
    (void)inputBuffer;  // Prevent unused variable warning.
    RunMixerFrame(outputBuffer, framesPerBuffer);
    return 0;
}

static paTestData data;
static float *out;

int main(int argc, char **argv)
{
    song = 400;  // 13
    filename = "emerald.gba";  // sa2.gba
    songTableAddress = 7031280;  // 11358028
    if (argc > 1) filename = argv[1];
    if (argc > 2) songTableAddress = atoi(argv[2]);
    if (argc > 3) song = atoi(argv[3]);
    musicFile = fopen(filename, "rb");
    if (0 != fseek(musicFile, 0, SEEK_END)) return 0;
    int sizef = ftell(musicFile);
    if (0 != fseek(musicFile, 0, SEEK_SET)) return 0;
    if (sizef != fread(music, 1, sizef, musicFile)) return 0;
    fclose(musicFile);
    
    m4aSoundInit(MIXER_FREQ, music, songTableAddress);
    m4aSongNumStart(song);
    // Initialize library before making any other calls.
    PaStream *stream;
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) goto error;

    // Open an audio I/O stream.
    err = Pa_OpenDefaultStream(&stream,
                               0,          // no input channels
                               2,          // stereo output
                               paFloat32,  // 32 bit output
                               MIXER_FREQ,
                               MIXER_FREQ / 60,  // frames per buffer
                               patestCallback,
                               &data);
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;
    while (1) {
        sleep(2);
        if (!isRunning) {
            err = Pa_StopStream(stream);
            // stop();
            return 0;
        }
    }
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;
    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;
    Pa_Terminate();
    printf("Done!\n");
    return err;
error:
    Pa_Terminate();
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return err;
}