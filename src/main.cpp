#include <iostream> 
#include <fstream>
#include <string>
#include <stdlib.h>
#include <getopt.h>

#include "z_libpd.h"
#include <jack/jack.h>
#include <jack/transport.h>

#include "main.h"

typedef jack_default_audio_sample_t sample_t;

// #define DEBUG 1
// TODO: write output to an error log instead of cout and cerr

#define XOOKY_VERSION "2011-10-22"
#define LIB_PD_VERSION "2011-10-22"

// ====================
// = GLOBAL_VARIABLES =
// ====================
int sampleRate = 0; // This is set by the JACK server in the initJack function
const int bufferLength = 64; // Always 64 for libpd

jack_client_t *client;
jack_port_t *portO1;
jack_port_t *portO2;
jack_port_t *portI1;
jack_port_t *portI2;

std::string filename;
std::string directory;

void *patchFile; // Pointer to the .pd file

//Interleaved io buffers:
float *output = (float*)malloc(bufferLength*2*sizeof(float));
float *input = (float*)malloc(bufferLength*2*sizeof(float));

// ========
// = MAIN =
// ========
int main (int argc, char *argv[]) {
   parseParameters(argc, argv);

	 initLibPd();
	 initJackAudioIO();

   // Keep the program alive.
	 while(1){
			sleep(1);
	 }

   return 0;
}

void parseParameters(int argc, char *argv[]){
   // Help message:
   std::string help = "Usage: xooky_nabox [-version] [-help] <file>\n\n";

   // Get the flag values and store.
   int showVersion = 0;
   int showHelp = 0;
   const struct option long_options[] = {
			{"version", no_argument, &showVersion, 1},
			{"help", no_argument, &showHelp, 1},
			{ NULL, no_argument, NULL, 0 }
   };
   int option_index;
   int c;
   while((c = getopt_long_only(argc, argv, "i:o:r:b:", long_options, &option_index)) != -1){
      switch (c){
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
   std::cout << "showVersion: " << showVersion << std::endl;
   std::cout << "showHelp: " << showHelp << std::endl;
   std::cout << "file: " << argv[argc-1] << std::endl << std::endl;
   #endif

   if(showHelp == 1){
      std::cout << help;
      exit(0);
   }

   if(showVersion == 1){
      std::cout << "XookyNabox version is " << XOOKY_VERSION << std::endl;
      std::cout << "LibPd version is " << LIB_PD_VERSION << std::endl;
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
	 // The Jack server only seems to run fine at 256 samples per frame and libpd only processess samples
	 // in chunks (ticks) of n*64 samples at a time. We need to set the ticksPerBuffer to 4.


	 // init the pd engine
	 libpd_init();
	 libpd_init_audio(2, 2, sampleRate, 4); //nInputs, nOutputs, sampleRate, ticksPerBuffer
	 
	 // send compute audio 1 message to pd
	 libpd_start_message(1);
	 libpd_add_float(1.0f);
	 libpd_finish_message("pd", "dsp");
	 
	 // load the patch
	 patchFile = libpd_openfile( (char*)filename.c_str(), (char*)directory.c_str() );
	 if (patchFile == NULL) {
			std::cout << "Could not open patch";
			exit(1);
	 }
}


// ========================
// = INITIALIZE AUDIO I/O =
// ========================
void initJackAudioIO(){
	 // Create client:
	 if((client = jack_client_open("xooky_nabox", JackNullOption, NULL)) == NULL){
			std::cout << "jack server not running?\n";
			exit(1);
	 }

	 // Register callbacks:
	 jack_on_shutdown(client, jack_shutdown, 0);
	 jack_set_process_callback(client, process, 0);

	 // Register io ports:
	 portO1 = jack_port_register(client, "out1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	 portO2 = jack_port_register(client, "out2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	 portI1 = jack_port_register(client, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	 portI2 = jack_port_register(client, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

	 // Get sample rate from server
	 sampleRate = jack_get_sample_rate(client);
	 std::cout << "Sample rate:" << sampleRate << "\n"; 

	 //Go!
	 if (jack_activate (client)) {
			std::cout << "Cannot activate client";
			exit(1);
	 }
	 
}

// ========================
// = JACK AUDIO CALLBACKS =
// ========================
int process(jack_nframes_t nframes, void *arg){
	 // Get pointers to the input and output signals
	 sample_t *in1 = (sample_t *) jack_port_get_buffer(portI1, nframes);
	 sample_t *in2 = (sample_t *) jack_port_get_buffer(portI2, nframes);
	 sample_t *out1 = (sample_t *) jack_port_get_buffer(portO1, nframes);
	 sample_t *out2 = (sample_t *) jack_port_get_buffer(portO2, nframes);

	 // Jack uses mono ports and pd expects interleaved stereo buffers.
	 for(unsigned int i=0; i<nframes; i++){
			input[i*2] = *in1;
			input[(i*2)+1] = *in2;
			in1++;
			in2++;
	 }
 
	 // DSP Magic ;)
	 libpd_process_float(input, output);
	 
	 for(unsigned int i=0; i<nframes; i++){
			*out1 = output[i*2];
			*out2 = output[(i*2)+1];
			out1++;
			out2++;
	 }

	 return 0;
}  

void jack_shutdown (void *arg){
	 exit(1);
}

