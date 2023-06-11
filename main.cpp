#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <portaudio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "m4play.h"

#define MIXER_FREQ 48000
int song;
uint32_t songTableAddress;
uint32_t musicFileOffset;
uint32_t m4aMode;
unsigned char music[0x8000000];
uint32_t songFileSize;
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

uint32_t scan(uint32_t *mode, uint32_t *offset){
    uint32_t pos = 0;
    uint32_t temp;

    if(music[pos + 0] == 0x41  // AM4p format
    && music[pos + 1] == 0x4D
    && music[pos + 2] == 0x34
    && music[pos + 3] == 0x70){
        *mode = (music[pos + 39] << 24) | (music[pos + 38] << 16) | (music[pos + 37] << 8) | music[pos + 36];
        temp = (music[pos + 11] << 24) | (music[pos + 10] << 16) | (music[pos + 9] << 8) | music[pos + 8];
        *offset = ((music[pos + 15] << 24) | (music[pos + 14] << 16) | (music[pos + 13] << 8) | music[pos + 12]);
        return (((music[pos + 35] << 24) | (music[pos + 34] << 16) | (music[pos + 33] << 8) | music[pos + 32])) & 0x7FFFFFF;
    }

    while(pos < (songFileSize - 35)){  // isn't AM4p, scan the rom
        if((music[pos + 0] & 0xBF) == 0x89
        && music[pos + 1] == 0x18
        && music[pos + 2] == 0x0A
        && music[pos + 3] == 0x68
        && music[pos + 4] == 0x01
        && music[pos + 5] == 0x68
        && music[pos + 6] == 0x10
        && music[pos + 7] == 0x1C
        && (music[pos + 23] & 0xFE) == 0x08){
            break;
        }
        pos += 4;
    }
    //printf("pos: 0x%x (%d)\n", pos, pos);
    if(music[pos - 61] == 0x03
    && music[pos - 57] == 0x04){
        *mode = (music[pos - 45] << 24) | (music[pos - 46] << 16) | (music[pos - 47] << 8) | music[pos - 48];
    }else{
        *mode = (music[pos - 61] << 24) | (music[pos - 62] << 16) | (music[pos - 63] << 8) | music[pos - 64];
    }
    return ((music[pos + 23] << 24) | (music[pos + 22] << 16) | (music[pos + 21] << 8) | music[pos + 20]) & 0x7FFFFFF;
}

int main(int argc, char **argv)
{
    song = 0;
    filename = "";
    songTableAddress = 0;
    musicFileOffset = 0;
    if (argc > 1) filename = argv[1];
    if (argc > 2) song = atoi(argv[2]);
    if (argc > 3) songTableAddress = atoi(argv[3]);
    musicFile = fopen(filename, "rb");
    if (0 != fseek(musicFile, 0, SEEK_END)) return 0;
    songFileSize = ftell(musicFile);
    if (0 != fseek(musicFile, 0, SEEK_SET)) return 0;
    if (songFileSize != fread(music, 1, songFileSize, musicFile)) return 0;
    fclose(musicFile);
    
    if(songTableAddress >= songFileSize || songTableAddress == 0){
        songTableAddress = scan(&m4aMode, &musicFileOffset);
    }else{
        scan(&m4aMode, &musicFileOffset);
    }
    printf("songTableAddress: 0x%x (%d)\n", songTableAddress, songTableAddress);
    printf("Max Channels: %d\n", (m4aMode >> 8) & 0xF);
    printf("Volume: %d\n", (m4aMode >> 12) & 0xF);
    printf("Original Rate: %.2fhz\n", getOrigSampleRate(((m4aMode >> 16) & 0xF) - 1) * 59.727678571);
    m4aSoundInit(MIXER_FREQ, music, songTableAddress, m4aMode, musicFileOffset);
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