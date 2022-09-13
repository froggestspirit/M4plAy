#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
//#include <portaudio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "sound_mixer.h"

#define MIXER_FREQ 48000
int song;
unsigned char music[0x8000000];
FILE *musicFile = NULL;
char *filename;

unsigned char REG_BASE[0x400] __attribute__((aligned(4)));
struct SoundInfo *SOUND_INFO_PTR;

SDL_Thread *mainLoopThread;
SDL_atomic_t isFrameAvailable;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0;  // 16.666667ms
double timeScale = 1.0;

float *audio;
float *lastAudio;

/*typedef struct
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
    float *out = (float *)outputBuffer;
    static unsigned int i;
    (void)inputBuffer; // Prevent unused variable warning.

    for (i = 0; i < framesPerBuffer; i++) {
        audio = RunMixerFrame(1);
        if (isRunning) {
            lastAudio = audio;
        } else {
            audio = lastAudio;
        }
        *out++ = *(audio);
        *out++ = *(audio + 1);
    }
    return 0;
}*/

// static paTestData data;
// static float *out;
int main(int argc, char **argv) {
    song = 0;
    bool inf = false;
    if (argc > 1) {
        filename = argv[1];
    } else {
        filename = "SA2.bin";
    }
    if (argc > 2) song = atoi(argv[2]);
    if (argc > 3) inf = true;
    musicFile = fopen(filename, "rb");
    if (0 != fseek(musicFile, 0, SEEK_END)) return false;
    int sizef = ftell(musicFile);
    if (0 != fseek(musicFile, 0, SEEK_SET)) return false;
    if (sizef != fread(music, 1, sizef, musicFile)) return false;
    printf("music Size: %X\n", sizef);
    fclose(musicFile);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    isFrameAvailable.value = 0;

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = MIXER_FREQ;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;

    if (SDL_OpenAudio(&want, 0) < 0)
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    else {
        if (want.format != AUDIO_F32)
            SDL_Log("We didn't get Float32 audio format.");
        SDL_PauseAudio(0);
    }

    m4aSoundInit(MIXER_FREQ);
    m4aSongNumStart(12);

    // Initialize library before making any other calls.
    /*PaStream *stream;
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) goto error;

    // Open an audio I/O stream.
    err = Pa_OpenDefaultStream(&stream,
                               0,         // no input channels
                               2,         // stereo output
                               paFloat32, // 32 bit output
                               MIXER_FREQ,
                               0x1000, // frames per buffer
                               patestCallback,
                               &data);
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;
    while (1) {
        sleep(2);
        if (!isRunning) {
            err = Pa_StopStream(stream);
            stop();
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
}*/
    double accumulator = 0.0;
    while (isRunning) {
        if (!paused) {
            double dt = fixedTimestep / timeScale;  // TODO: Fix speedup

            curGameTime = SDL_GetPerformanceCounter();
            double deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
            if (deltaTime > (dt * 5))
                deltaTime = dt;
            lastGameTime = curGameTime;

            accumulator += deltaTime;

            while (accumulator >= dt) {
                SDL_QueueAudio(1, RunMixerFrame(MIXER_FREQ / 60), ((MIXER_FREQ / 60) * 8));
                accumulator -= dt;
            }
        }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                isRunning = false;
        }
    }
    SDL_Quit();
    return 0;
}
