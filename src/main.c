#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "cgb_audio.h"
#include "io_reg.h"

uint8_t music[0x8000000];
uint32_t songTableAddress;
FILE *musicFile = NULL;
char *filename;

unsigned char REG_BASE[0x400] __attribute__ ((aligned (4)));

struct SoundInfo *SOUND_INFO_PTR;

SDL_atomic_t isFrameAvailable;
bool isRunning = 1;
bool paused = 0;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;

int song;

int main(int argc, char **argv)
{
    song = 400;  // 13
    filename = "emerald.gba";  // sa2.gba
    songTableAddress = 0x6B49F0;  // 11358028
    if (argc > 1) filename = argv[1];
    if (argc > 2) songTableAddress = atoi(argv[2]);
    if (argc > 3) song = atoi(argv[3]);
    musicFile = fopen(filename, "rb");
    if (0 != fseek(musicFile, 0, SEEK_END)) return false;
    int sizef = ftell(musicFile);
    if (0 != fseek(musicFile, 0, SEEK_SET)) return false;
    if (sizef != fread(music, 1, sizef, musicFile)) return false;
    fclose(musicFile);


    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 42048;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    cgb_audio_init(want.freq);


    if (SDL_OpenAudio(&want, 0) < 0)
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    else
    {
        if (want.format != AUDIO_F32) /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
        SDL_PauseAudio(0);
    }
    
    m4aSoundInit(music, songTableAddress);
    m4aSongNumStart(song);

    double accumulator = 0.0;

    while (isRunning)
    {
        if (!paused)
        {
            SDL_AtomicSet(&isFrameAvailable, 1);
            double dt = fixedTimestep / timeScale; // TODO: Fix speedup

            curGameTime = SDL_GetPerformanceCounter();
            double deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
            if (deltaTime > (dt * 5))
                deltaTime = dt;
            lastGameTime = curGameTime;

            accumulator += deltaTime;

            while (accumulator >= dt)
            {
                if (SDL_AtomicGet(&isFrameAvailable))
                {
                    m4aSoundVSync();
                    m4aSoundMain();
                    accumulator -= dt;
                }
            }
        }
    }
    SDL_Quit();
    return 0;
}
