#ifndef _XOOKY_NABOX_MAIN
#define _XOOKY_NABOX_MAIN

void initLibPd();
void initJackAudioIO();
int process(jack_nframes_t nframes, void *arg);
void parseParameters(int argc, char *argv[]);
void jack_shutdown (void *arg);

#endif
