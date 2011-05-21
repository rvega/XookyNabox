#include "portaudio.h"
#include <iostream>
#include <string>
using namespace std;

/*
 * TYPE DEFINITIONS
 */
typedef struct {
    float left_phase;
    float right_phase;
}
paTestData;

/*
 * CONSTANT DEFINITIONS
 */
#define SAMPLE_RATE (44100)

/*
 * GLOBAL VARIABLES
 */
static paTestData data;

/*
 * PORT AUDIO CALLBACK
 */
static int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ){
    /* Cast data passed through stream to our structure. */
    paTestData *data = (paTestData*)userData; 
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */

    for( i=0; i<framesPerBuffer; i++ ){
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}

/*
 * ENTRY POINT
 */
int main (int argc, char const* argv[]){
    PaError err;

    err = Pa_Initialize();
    if(err!= paNoError){
        cout << "PortAudio error:" << Pa_GetErrorText( err );
        return 1;
    }

    /* Open an audio I/O stream. */
    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream,
            0,          /* no input channels */
            2,          /* stereo output */
            paFloat32,  /* 32 bit floating point output */
            SAMPLE_RATE,
            256,        /* frames per buffer, i.e. the number of sample frames that PortAudio will request from the callback. Many apps may want to use paFramesPerBufferUnspecified, which tells PortAudio to pick the best, possibly changing, buffer size.*/
            paCallback, /* this is your callback function */
            &data /*This is a pointer that will be passed to your callback*/
            );
    if(err!= paNoError){
        cout << "PortAudio error:" << Pa_GetErrorText( err );
        return 1;
    }

    /* Start the stream */
    err = Pa_StartStream( stream );
    if(err!= paNoError){
        cout << "PortAudio error:" << Pa_GetErrorText( err );
        return 1;
    }

    Pa_Sleep(60000);		

    err = Pa_StopStream( stream );
    if(err!= paNoError){
        cout << "PortAudio error:" << Pa_GetErrorText( err );
        return 1;
    }

    return 0;
}
