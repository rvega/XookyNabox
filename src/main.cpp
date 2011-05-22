#include <iostream>
#include <string>
#include <portaudio.h>
#include <ZenGarden.h>

#include "main.h"

#pragma mark GLOBAL_VARIABLES
///////////////////////////////////////////////////////////////////////////////
// GLOBAL_VARIABLES
///////////////////////////////////////////////////////////////////////////////
int sampleRate = 44100;
int bufferLength = 32;
PaStream *stream;
PdContext *context;

//Interleaved io buffers:
float *output = (float*)malloc(bufferLength*2*sizeof(float));
float *input = (float*)malloc(bufferLength*2*sizeof(float));

#pragma mark MAIN
///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////
int main (int argc, char const* argv[]) {
   //TODO: Parse command line parameters to set buffer size, etc. Look at puredata --nogui params.

   initZenGarden();
   initAudioIO();

   // Keep the program alive.
   char c;
   std::cout << "Running, press any key and then press <Enter> to terminate.";
   std::cin >> c;

   stopAudioIO();
   stopZengarden();

   return 0;
}

#pragma mark INITIALIZE ZEN GARDEN
///////////////////////////////////////////////////////////////////////////////
// INITIALIZE ZEN GARDEN
///////////////////////////////////////////////////////////////////////////////
void initZenGarden(){
   //TODO: get from command line params instead
   // std::string filename = "00_wobbly_mono.pd";
   // std::string filename = "01_wobbly_stereo.pd";   
   std::string filename = "02_HelloWorld1.pd";
   std::string directory = "/Users/rafaelvega/Projects/Active/XookyNabox/repo/patches/";

   context = zg_new_context(2, 2, bufferLength, (float)sampleRate, zengardenCallback, NULL); // # inputs, # outputs, samplerate, callback, userData
   PdGraph *graph = zg_new_graph(context, (char*)directory.c_str(), (char*)filename.c_str());
   if (graph == NULL) {
      zg_delete_context(context);
      std::cout << "Could not open patch";
      exit(1);
   }

   zg_attach_graph(context, graph);
}

void stopZengarden(){
   zg_delete_context(context);
}

#pragma mark INITIALIZE AUDIO I/O
///////////////////////////////////////////////////////////////////////////////
// INITIALIZE AUDIO I/O
///////////////////////////////////////////////////////////////////////////////
void initAudioIO(){
   PaError err;

   err = Pa_Initialize();
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }

   // Show available devices
   // PaDeviceIndex ndev = Pa_GetDeviceCount();
   // for(int i=0; i<ndev; i++){ 
   //    const PaDeviceInfo *info = Pa_GetDeviceInfo((PaDeviceIndex) i);
   //    if (info->maxOutputChannels > 0) std::cout << "output device: ";
   //    if (info->maxInputChannels > 0) std::cout << "input device: "; 
   //    std::cout << i << ": " << info->name << std::endl;
   // }

   // Open an audio I/O stream.
   PaStreamParameters outParameters;
   bzero(&outParameters, sizeof(outParameters));
   outParameters.channelCount = 2;
   outParameters.device = 2; // 2 is "Built in output" on my laptop. TODO: get from command line param or something.
   outParameters.sampleFormat = paFloat32;

   PaStreamParameters inParameters;
   bzero(&inParameters, sizeof(inParameters));
   inParameters.channelCount = 2;
   inParameters.device = 0; // 0 is "Built in microphone" on my laptop. TODO: get from command line param or something.
   inParameters.sampleFormat = paFloat32;

   // err = Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, sampleRate, bufferLength, paCallback, context); //2 inputs, 2 outputs, 32bit float samples, sample rate, buffer length, callback function, user data to pass to callback function
   err = Pa_OpenStream(&stream, &inParameters, &outParameters, sampleRate, bufferLength, paNoFlag, paCallback, context);
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }

   // Start the stream
   err = Pa_StartStream( stream );
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }
}

void stopAudioIO(){
   // Stop the stream
   PaError err = Pa_StopStream( stream );
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }
}

#pragma mark PORT AUDIO CALLBACK
///////////////////////////////////////////////////////////////////////////////
// PORT AUDIO CALLBACK
///////////////////////////////////////////////////////////////////////////////
static int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ){
   PdContext *ctx = (PdContext*) userData;
   float *out = (float*)outputBuffer;
   float *in = (float*)inputBuffer;

   //Zengarden works with noninterleaved buffers, portaudio works with interleaved. Changing buffer to interleaved.
   for (int i=0; i<bufferLength; i++) {
      for (int channel = 0; channel < 2; channel++) {
         input[i + channel*bufferLength] = in[i*2 + channel]; 
      }
   }

   // Magic :)
   zg_process(ctx, input, output);

   //Zengarden works with noninterleaved buffers, portaudio works with interleaved. Changing buffer to interleaved.
   for (int i=0; i<bufferLength; i++) {
      for (int channel = 0; channel < 2; channel++) {
         out[i*2 + channel] = output[i + channel*bufferLength]; 
      }
   }
   return 0;
}

#pragma mark ZENGARDEN_CALLBACK
///////////////////////////////////////////////////////////////////////////////
// ZENGARDEN_CALLBACK
///////////////////////////////////////////////////////////////////////////////
void zengardenCallback(ZGCallbackFunction function, void *userData, void *ptr) {
  switch (function) {
    case ZG_PRINT_STD: {
      std::cout << "ZG_PRINT_STD: " << (char *)ptr << std::endl;
      break;
    }
    case ZG_PRINT_ERR: {
      std::cout << "ZG_PRINT_ERR: " << (char *)ptr << std::endl;
      break;
    }
    case ZG_PD_DSP: {
      std::cout << "ZG_PD_DSP: " << (char *)ptr << std::endl;
      break;
    }
  }
}

