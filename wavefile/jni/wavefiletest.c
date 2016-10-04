#include <android/log.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/portaudio.h"

#define APPNAME "portaudio_test_blocking_sine"
#define NUM_SECONDS         (250)
#define SAMPLE_RATE         (44100)
#define FRAMES_PER_BUFFER   (512)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (512)

int main() {
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    short buffer[FRAMES_PER_BUFFER][2];
    short sine[TABLE_SIZE];
    int left_phase = 0;
    int right_phase = 0;
    int left_inc = 1;
    int right_inc = 3;
    int i, j, k;
    int bufferCount;

    for( i=0; i<TABLE_SIZE; i++ )
    {
        sine[i] = (short) (((float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. )) * SHRT_MAX);
    }
    err = Pa_Initialize();
    if (err != paNoError) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "error during initialization %d", err);
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "no default output device");
    }
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
                &stream,
                NULL,
                &outputParameters,
                SAMPLE_RATE,
                FRAMES_PER_BUFFER * 2,
                paClipOff,
                NULL,
                NULL );

    
    FILE *f = fopen("test.wav", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    unsigned long current = 44;

    for( k=0; k < 1; ++k )
    {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "starting playback");
        err = Pa_StartStream( stream );
        if( err != paNoError )
            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "error starting stream %d", err);

        bufferCount = 40;//((/*NUM_SECONDS * SAMPLE_RATE*/ fsize) / FRAMES_PER_BUFFER);

        for( i=0; i < bufferCount && current <= fsize-24; i++ )
        {
           __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "fsize|current: %d\t%d", fsize, current);
           for( j=0; j < 512; ++j)
            {
 
                short sample1 = (string[current+1] << 8) | string[current];
                current += 2;
                short sample2 = (string[current+1] << 8) | string[current];
                current += 2;
                buffer[j][0] = sample1;
                buffer[j][1] = sample2;
            }

            err = Pa_WriteStream( stream, buffer, 512);

        }

        err = Pa_StopStream( stream );
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "stopped stream, err %d", err);

        ++left_inc;
        ++right_inc;


    }
    err = Pa_CloseStream( stream );
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "closed stream, err %d", err);
    Pa_Terminate();
}
