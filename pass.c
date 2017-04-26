#include <stdio.h>
#include <stdlib.h>

#include "portaudio.h"
#include "CCRing.h"

#define SAMPLE_RATE (44100)

typedef struct
{
  CCRing* ring;
  unsigned long tap;
}   
paTestData;

float counter = 0;


/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/ 
static int patestCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
  float* in = (float*)inputBuffer;
  float* out = (float*)outputBuffer;

  paTestData* data = (paTestData*) userData;

  // CCRing* ring = data->ring;
  // ccAppend(ring, in, framesPerBuffer);
  // counter++; if (counter == 15) plot(ring);

  for (int i; i< framesPerBuffer; i++){
    // if (i < 128) *out++ = -0.1;
    // else *out++ = 0.1;

    *out++ = *in++;
    *out++ = *in++;
  }
  return 0;
}


int main(int argc, char** argv) {

  paTestData data;
  PaError err;
  PaStream *stream;
  
  CCRing* ring = createRing(SAMPLE_RATE * 0.1);
  if (ring == NULL){
    printf("%s", "Failed to create ring");
    return EXIT_FAILURE;
  }

  data.ring = ring;

  // get and display the Port audio version in use
  const PaVersionInfo* info = Pa_GetVersionInfo();
  printf("%s\n", info->versionText);

  err = Pa_Initialize();
  if (err != paNoError) {
    goto error;
  }

  // print out list of devices
  const PaDeviceInfo * deviceInfo;
  PaDeviceIndex numDevices = Pa_GetDeviceCount();
  printf("Found %d device(s)\n", numDevices);
  for (int i=0; i<numDevices; i++) {
    deviceInfo = Pa_GetDeviceInfo(i);
    printf("D%d: %s\n", i, deviceInfo->name);
  }

  int inDevice, outDevice;
  if (argc == 1) {
    inDevice = Pa_GetDefaultInputDevice();
    outDevice = Pa_GetDefaultOutputDevice();
  } else if (argc == 2) {
    inDevice = atoi(argv[1]);
    outDevice = inDevice;
  } else if (argc >= 3) {
    inDevice = atoi(argv[1]);
    outDevice = atoi(argv[2]);
  }

  PaStreamParameters in, out;
  const PaDeviceInfo *inInfo;
  const PaDeviceInfo *outInfo;

  inInfo = Pa_GetDeviceInfo(inDevice);
  in.device = inDevice;
  in.channelCount = 1;
  in.sampleFormat = paFloat32;
  in.suggestedLatency = inInfo->defaultLowInputLatency;
  in.hostApiSpecificStreamInfo = NULL;

  outInfo = Pa_GetDeviceInfo(outDevice);
  out.device = outDevice;
  out.channelCount = 1;
  out.sampleFormat = paFloat32;
  out.suggestedLatency = outInfo->defaultLowOutputLatency;
  out.hostApiSpecificStreamInfo = NULL;

  printf("Input:  %s\n", inInfo->name);
  printf("Output: %s\n", outInfo->name);

  /* Open an audio I/O stream. */
  err = Pa_OpenStream(&stream,
                      &in,
                      &out,
                      SAMPLE_RATE,
                      512,        /* frames per buffer, i.e. the number
                                  of sample frames that PortAudio will
                                  request from the callback. Many apps
                                  may want to use
                                  paFramesPerBufferUnspecified, which
                                  tells PortAudio to pick the best,
                                  possibly changing, buffer size.*/
                      paNoFlag,
                      patestCallback, /* this is your callback function */
                      &data); /* will be passed to your callback*/
  
  if (err != paNoError) {
    goto error;
  }

  // Start playback
  err = Pa_StartStream(stream);
  if (err != paNoError) {
    goto error;
  }

  // Wait
  Pa_Sleep(5 * 1000);
  // Stop playback
  err = Pa_StopStream(stream);
  if (err != paNoError) {
    goto error;
  }

  err = Pa_CloseStream(stream);
  if (err != paNoError) {
    goto error;
  }
  
  err = Pa_Terminate();
  if (err != paNoError) {
    goto error;
  }

  return EXIT_SUCCESS;

  // Report any errors to our user
  error: {
    printf("PortAudio error: %s\n", Pa_GetErrorText(err));
  }
  return EXIT_FAILURE;
}
