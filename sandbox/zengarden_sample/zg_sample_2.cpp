// 
//  zg_sample_2.cpp
//  
//  Created by rafaelvega on 2011-01-23.
// 
//  An ALSA host to test ZenGarden.

#include <iostream>
#include <string>
#include <alsa/asoundlib.h>
#include "ZenGarden.h"

// settings 
#define BLOCK_SIZE 1024 
#define NUM_INPUTS  0 
#define NUM_OUTPUTS 2 
#define SAMPLERATE (44100.0f)

using namespace std;

// interleaved io buffers 
float output[BLOCK_SIZE*NUM_OUTPUTS]; 
float input[BLOCK_SIZE*NUM_INPUTS];

// ===============================
// = Zengarden Callback Function =
// ===============================
extern "C" {
  void zengardenCallback(ZGCallbackFunction function, void *userData, void *ptr) {
    switch (function) {
      case ZG_PRINT_STD: {
        cout << "STD: " << (char *)ptr << endl;
        break;
      }
      case ZG_PRINT_ERR: {
        cout << "ERR: " << (char *)ptr << endl;
        break;
      }
      case ZG_PD_DSP: {
        cout << "DSP: " << (char *)ptr << endl;
        break;
      }
    }
  }
};

// ===============
// = Entry Point =
// ===============
int main(int argc, char * const argv[]) {
  //Parse command line params
  string arg_patch_path;
  int c;
  while((c = getopt(argc, argv, "p:")) != -1){
    switch (c){
      case 'p':
        arg_patch_path = string(optarg);
      break;	    
    }
  }

  if(arg_patch_path.empty()){
    cout << "Usage:" << endl << argv[0] << " -p /path/to/patch.pd" << endl;
    exit(1);
  }

  //Open PCM device for playback.
  snd_pcm_t *output_handle;
  if (snd_pcm_open(&output_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) { // 0 is the standard mode, can also be SND_PCM_NONBLOCK or SND_PCM_NONBLOCK <http://www.suse.de/~mana/alsa090_howto.html>
    cout << "Unable to open default pcm device" << endl;
    exit(1);
  }

  //Set parameters to PCM device
  snd_pcm_hw_params_t *hw_params; // Hardware params struct
  snd_pcm_hw_params_alloca(&hw_params); //Allocate memory for it
  snd_pcm_hw_params_any(output_handle, hw_params); //Start with default PCM params
  snd_pcm_hw_params_set_access(output_handle, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED); //Set noninterleaved mode
  snd_pcm_hw_params_set_format(output_handle, hw_params, SND_PCM_FORMAT_S32); //Set 32 bit floating point samples
  snd_pcm_hw_params_set_channels(output_handle, hw_params, 2); // 2 output channels
  unsigned int samplerate_actual_value = SAMPLERATE;
  int dir;
  snd_pcm_hw_params_set_rate_near(output_handle, hw_params, &samplerate_actual_value, &dir); // Set sample rate
  snd_pcm_uframes_t block_size_actual_size = BLOCK_SIZE;
  snd_pcm_hw_params_set_period_size_near(output_handle, hw_params, &block_size_actual_size, &dir); // Set block size
  if (snd_pcm_hw_params(output_handle, hw_params) < 0) {
    cout << "Unable to set hardware parameters" << endl;
    exit(1);
  }
  
  // A sawtooth wave
  // unsigned char *data;
  // int pcmreturn, l1, l2;
  // short s1, s2;
  // int frames;
  // 
  // data = (unsigned char *)malloc(block_size_actual_size);
  // frames = block_size_actual_size >> 2;
  // for(l1 = 0; l1 < 100; l1++) {
  //   for(l2 = 0; l2 < block_size_actual_size; l2++) {
  //     s1 = (l2 % 128) * 100 - 5000;  
  //     s2 = (l2 % 256) * 100 - 5000;  
  //     data[4*l2] = (unsigned char)s1;
  //     data[4*l2+1] = s1 >> 8;
  //     data[4*l2+2] = (unsigned char)s2;
  //     data[4*l2+3] = s2 >> 8;
  //   }
  //   while ((pcmreturn = snd_pcm_writei(output_handle, data, frames)) < 0) {
  //     snd_pcm_prepare(output_handle);
  //     fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
  //   }
  // }
  
  
  //Create ZG context
  PdContext *context = zg_new_context(0, 2, (int)block_size_actual_size, (int)samplerate_actual_value, zengardenCallback, NULL); // 0 inputs, 2 outputs, samplerate, callback, userData
    
  //  Load pd patch
    char cp[4096];
    getcwd(cp, sizeof(cp));
    string current_path = string(cp);
  
    string patch_path = current_path + "/" + arg_patch_path;
    unsigned int i = patch_path.find_last_of("/");
    string patch_file = patch_path.substr(i+1);
    string patch_dir = patch_path.substr(0,i+1);
    
    PdGraph *graph = zg_new_graph(context, (char*)patch_dir.c_str(), (char*)patch_file.c_str());
    if (graph == NULL) {
      zg_delete_context(context);
      cout << "Could not open patch";
      exit(1);
    }
  
    // Processing loop
    while(1){
      zg_process(context, input, output);
    }
    
    //This will never run...
    zg_delete_context(context);
    free(input);
    free(output);
  
  exit(0);
}
