#include <iostream> 
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <portaudio.h>
#include "z_libpd.h"

#include "main.h"

// #define DEBUG 1

// TODO: write output to an error log instead of cout and cerr

#define XOOKY_VERSION "2011-10-22"
#define LIB_PD_VERSION "2011-10-22"

// ====================
// = GLOBAL_VARIABLES =
// ====================
int audioInDev = 0;
int audioOutDev = 2;
int sampleRate = 44100;
int bufferLength = 64;

std::string filename;
std::string directory;

PaStream *stream;
void *patchFile;

//Interleaved io buffers:
float *output = (float*)malloc(bufferLength*2*sizeof(float));
float *input = (float*)malloc(bufferLength*2*sizeof(float));

// ========
// = MAIN =
// ========
int main (int argc, char *argv[]) {
   parseParameters(argc, argv);

   initLibPd();
   initAudioIO();

   // Keep the program alive.
   while(1){
      sleep(100);
   }

   // This is obviously never reached, so far no problems with that...
   stopAudioIO();
   stopLibPd();

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
      std::cout << "LibPD version is " << LIB_PD_VERSION << std::endl;
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

// =====================
// = INITIALIZE LIB PD =
// =====================
void initLibPd(){
   libpd_init();
   libpd_init_audio(2, 2, sampleRate, 1); //nInputs, nOutputs, sampleRate, ticksPerBuffer

   // send compute audio 1 message to pd
   libpd_start_message(1);
   libpd_add_float(1.0f);
   libpd_finish_message("pd", "dsp");

   patchFile = libpd_openfile( (char*)filename.c_str(), (char*)directory.c_str() );
   if (patchFile == NULL) {
      std::cout << "Could not open patch";
      exit(1);
   }
}

void stopLibPd(){
   libpd_closefile(patchFile);
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

   err = Pa_OpenStream(&stream, &inParameters, &outParameters, sampleRate, bufferLength, paNoFlag, paCallback, NULL);
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
   float *out = (float*)outputBuffer;
   float *in = (float*)inputBuffer;
   
   libpd_process_float(in, out);

   return 0;
}
