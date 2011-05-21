#include <iostream>
#include <cstdlib>
#include <string>
#include <time.h>
using namespace std;

#include "RtAudio.h"
#include "ZenGarden.h"

// Settings 
#define BLOCK_SIZE 1024 
#define NUM_INPUTS  0 
#define NUM_OUTPUTS 2 
#define SAMPLERATE 44100

PdContext *context;

// ================================================================================
// = RtAudio callback function. Fires when the hardware output buffer needs data. =
// ================================================================================
int rtAudioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
  cout << "RtCallback" << endl;
  cout << time(NULL) << endl;  
  
  zg_process(context, (float *)inputBuffer, (float *)outputBuffer);

  return 0;
}

// =======================================================================================
// = Zengarden Callback Function. Fires whenever Zengarden wants to communicate with us. =
// =======================================================================================
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

// ==================================================
// = Load patch into Zengarden and init ZG instance =
// ================================================== 
void initZengardenWithPatch(string patch_dir, string patch_file){
  context = zg_new_context(0, 2, BLOCK_SIZE, SAMPLERATE, zengardenCallback, NULL); // 0 inputs, 2 outputs, samplerate, callback, userData  
  
  PdGraph *graph = zg_new_graph(context, (char*)patch_dir.c_str(), (char*)patch_file.c_str());
  if (graph == NULL) {
    zg_delete_context(context);
    cout << "Could not open patch";
    exit(1);
  }
  
  zg_attach_graph(context, graph);
}

// ===============
// = Entry Point =
// ===============
int main(int argc, char * const argv[]){
  // Parse command line params
  string arg_patch_path;
  int c;
  while((c = getopt(argc, argv, "p:")) != -1){
    switch (c){
      case 'p':
        arg_patch_path = string(optarg);
      break;	    
    }
  }
  
  // Wrong params, exit.
  if(arg_patch_path.empty()){
    cout << "Usage:" << endl << argv[0] << " -p /path/to/patch.pd" << endl;
    exit(1);
  }
  
  // Full path and filename for patch
  char cp[4096];
  getcwd(cp, sizeof(cp));
  string current_path = string(cp);

  string patch_path = current_path + "/" + arg_patch_path;
  unsigned int i = patch_path.find_last_of("/");
  string patch_file = patch_path.substr(i+1);
  string patch_dir = patch_path.substr(0,i+1);
  
  // Init Zengarden
  initZengardenWithPatch(patch_dir, patch_file);
  
  // ============================================
  // = Init audio hardware output using RtAudio =
  // ============================================  
  // I wanted to put the init code in a subroutine but callback wouldn't fire :-|
  RtAudio dac;
  dac.showWarnings(true);
  if ( dac.getDeviceCount() < 1 ) {
    cout << "\nNo audio devices found!\n";
    exit(1);
  }

  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = NUM_OUTPUTS;
  parameters.firstChannel = NUM_INPUTS;
  unsigned int sampleRate = SAMPLERATE;
  unsigned int bufferFrames = BLOCK_SIZE;
  double data[2];

  try {
    dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &rtAudioCallback, (void *)&data );
    dac.startStream();
  }
  catch ( RtError& e ) {
    e.printMessage();
    exit(1);
  }
  
  
  // Wait
  char input;
  cout << "\nPlaying ... press <enter> to quit.\n";
  cin.get( input );

  // Terminate Audio Out
    // 
    // try {
    //   // Stop the stream
    //   dac.stopStream();
    // }
    // catch (RtError& e) {
    //   e.printMessage();
    // }
    // 
    // if ( dac.isStreamOpen() ) dac.closeStream();  
  
  
  // Terminate Zengarden Instance
  
  // Chao!
  return 0;


  

}





// Two-channel sawtooth wave generator.
// int saw( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
//   cout << time(NULL) << endl;
//   
//   unsigned int i, j;
//   double *buffer = (double *) outputBuffer;
//   double *lastValues = (double *) userData;
// 
//   if ( status )
//     cout << "Stream underflow detected!" << endl;
// 
//   // Write interleaved audio data.
//   for ( i=0; i<nBufferFrames; i++ ) {
//     for ( j=0; j<2; j++ ) {
//       *buffer++ = lastValues[j];
// 
//       lastValues[j] += 0.005 * (j+1+(j*0.1));
//       if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
//     }
//   }
// 
//   return 0;
// }

