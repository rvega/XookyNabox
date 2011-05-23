#include <iostream> 
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <portaudio.h>
#include <ZenGarden.h>

#include "main.h"

// #define DEBUG 1

#define XOOKY_VERSION "2011-05-22"
#define ZEN_VERSION "2011-05-20"

// ====================
// = GLOBAL_VARIABLES =
// ====================
int audioInDev = 0;
int audioOutDev = 2;
int sampleRate = 44100;
int bufferLength = 32;

std::string filename;
std::string directory;

PaStream *stream;
PdContext *context;

//Interleaved io buffers:
float *output = (float*)malloc(bufferLength*2*sizeof(float));
float *input = (float*)malloc(bufferLength*2*sizeof(float));

// ========
// = MAIN =
// ========
int main (int argc, char *argv[]) {
   parseParameters(argc, argv);

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

void parseParameters(int argc, char *argv[]){
   // Help message:
   std::string help = "Usage: xooky_nabox [-flags] <file>\n\n";
   help += "Configuration flags:\n";
   help += "-audioindev <n>      -- specify audio in device\n";
   help += "-audiooutdev <n>     -- specify audio out device\n";
   help += "-r <n>               -- specify sample rate\n";
   help += "-blocksize <n>       -- specify audio I/O block size in sample frames\n";
   help += "-listdev             -- don't run xooky, just print list of audio devices\n";
   help += "-version             -- don't run xooky, just print which version it is\n";
   help += "-help                -- don't run xooky, just print this message\n\n";

   // Get the flag values and store.
   int showVersion = 0;
   int showHelp = 0;
   int showDevices = 0;
   const struct option long_options[] = {
      {"audioindev", required_argument, NULL, 'i'},
         {"audiooutdev", required_argument, NULL, 'o'},
         {"r", required_argument, NULL, 'r'},
         {"blocksize", required_argument, NULL, 'b'},
         {"listdev", no_argument, &showDevices, 1},
         {"version", no_argument, &showVersion, 1},
         {"help", no_argument, &showHelp, 1},
         { NULL, no_argument, NULL, 0 }
   };
   int option_index;
   int c;
   while((c = getopt_long_only(argc, argv, "i:o:r:b:", long_options, &option_index)) != -1){
      switch (c){
         case 'i':
         audioInDev = atoi(optarg);
         break;
         case 'o':
         audioOutDev = atoi(optarg);
         break;
         case 'r':
         sampleRate = atoi(optarg);
         break;
         case 'b':
         bufferLength = atoi(optarg);
         break;
         case '0':
            // One of the flags was entered, nothing to do.
         break;
         default:
         break;
      }
   }

   // If no params
   if(argc<=1){
      std::cout << help;
      exit(1);
   }

   #ifdef DEBUG
   std::cout << "audioInDev: " << audioInDev << std::endl;
   std::cout << "audioOutDev: " << audioOutDev << std::endl;
   std::cout << "sampleRate: " << sampleRate << std::endl;
   std::cout << "bufferLength: " << bufferLength << std::endl;
   std::cout << "showVersion: " << showVersion << std::endl;
   std::cout << "showHelp: " << showHelp << std::endl;
   std::cout << "showDevices: " << showDevices << std::endl;
   std::cout << "file: " << argv[argc-1] << std::endl << std::endl;
   #endif

   if(showHelp == 1){
      std::cout << help;
      exit(0);
   }

   if(showVersion == 1){
      std::cout << "XookyNabox version is " << XOOKY_VERSION << std::endl;
      std::cout << "ZenGarden version is " << ZEN_VERSION << std::endl;
      exit(0);
   }

   if(showDevices){
      listDevices();
      exit(0);
   }

   //Last argument must be a filename. Validate and store.
   char* arg_patch = argv[argc-1];
   std::ifstream patch_file(arg_patch);
   if(!patch_file.good()){
      std::cout << "The file " << arg_patch <<" is not valid.\n";
      std::cout << help;
      exit(1);		
   }
   char patch_path[1024];
   realpath(arg_patch, patch_path);
   std::string patch_path_str = std::string(patch_path);
   unsigned int pos = patch_path_str.find_last_of('/');
   directory = patch_path_str.substr(0,pos+1);
   filename = patch_path_str.substr(pos+1);
}

// =========================
// = INITIALIZE ZEN GARDEN =
// =========================
void initZenGarden(){
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

// ========================
// = INITIALIZE AUDIO I/O =
// ========================
void initAudioIO(){
   PaError err;

   err = Pa_Initialize();
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }

   // Open an audio I/O stream.
   PaStreamParameters outParameters;
   memset(&outParameters, '\0', sizeof(outParameters));
   outParameters.channelCount = 2;
   outParameters.device = audioOutDev;
   outParameters.sampleFormat = paFloat32;

   PaStreamParameters inParameters;
   memset(&inParameters, '\0', sizeof(inParameters));
   inParameters.channelCount = 2;
   inParameters.device = audioInDev;
   inParameters.sampleFormat = paFloat32;

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

void listDevices(){
   PaError err;

   err = Pa_Initialize();
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }

   // Show available devices
   PaDeviceIndex ndev = Pa_GetDeviceCount();
   if(ndev<0){
      std::cout << "PortAudio error:" << Pa_GetErrorText( ndev );
      exit(1);      
   }
   std::cout << "\nAudio output devices:\n";
   for(int i=0; i<ndev; i++){ 
      const PaDeviceInfo *info = Pa_GetDeviceInfo((PaDeviceIndex) i);
      if (info->maxOutputChannels > 0){
         std::cout << "(" << i << ") " << info->name << std::endl;
      } 
   }	

   std::cout << "\nAudio input devices:\n";
   for(int i=0; i<ndev; i++){ 
      const PaDeviceInfo *info = Pa_GetDeviceInfo((PaDeviceIndex) i);
      if (info->maxInputChannels > 0){
         std::cout << "(" << i << ") " << info->name << std::endl;
      } 
   }	
   std::cout << "\n";
}

void stopAudioIO(){
   // Stop the stream
   PaError err = Pa_StopStream( stream );
   if(err!= paNoError){
      std::cout << "PortAudio error:" << Pa_GetErrorText( err );
      exit(1);
   }
}



// =======================
// = PORT AUDIO CALLBACK =
// =======================
static int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ){
   PdContext *ctx = (PdContext*) userData;
   float *out = (float*)outputBuffer;
   float *in = (float*)inputBuffer;

   //Zengarden works with noninterleaved buffers, portaudio works with interleaved. Changing input buffer to non-interleaved.
   for (int i=0; i<bufferLength; i++) {
      for (int channel = 0; channel < 2; channel++) {
         input[i + channel*bufferLength] = in[i*2 + channel]; 
      }
   }

   // Magic :)
   zg_process(ctx, input, output);

   //Zengarden works with noninterleaved buffers, portaudio works with interleaved. Changing output buffer to interleaved.
   for (int i=0; i<bufferLength; i++) {
      for (int channel = 0; channel < 2; channel++) {
         out[i*2 + channel] = output[i + channel*bufferLength]; 
      }
   }
   return 0;
}

// ======================
// = ZENGARDEN_CALLBACK =
// ======================
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
