#ifndef _XOOKY_NABOX_MAIN
#define _XOOKY_NABOX_MAIN

void initAudioIO();
void stopAudioIO();
static int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData );
void initZenGarden();
void zengardenCallback(ZGCallbackFunction function, void *userData, void *ptr);
void stopZengarden();

#endif