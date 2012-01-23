	#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <iostream>

#include "MonitorAudioOSS.h"
#include "../MonitorLogging.h"

using namespace std ;

MonitorAudioOSS::MonitorAudioOSS(const std::string* name, tSamplerate rate)
: MonitorAudio (name, rate) {
	run = false;
}

MonitorAudioOSS::~MonitorAudioOSS() {
	CloseDevice();
}

bool MonitorAudioOSS::Start(void *format) {
	if (InitDevice() < 0) {
		FILE_LOG(logERROR) << "Error initializing PCM device " << pcm_name ;
		exit(10);
	}

	run = true;
	JThread::Start();
	return true;
}

void MonitorAudioOSS::Stop() {
	FILE_LOG(logERROR) << "stopping " << pcm_name ;
	run = false;
}

int MonitorAudioOSS::InitDevice() {
	int sndparam;

	if ((pcm_name.length() == 0) || (pcm_rate == 0)) {
		return -1;
	}

	if ((dev_handle = open(pcm_name.c_str(), O_RDONLY)) < 0) {
		FILE_LOG(logERROR) << "open" ;
		return -1;
	}
	sndparam = AFMT_S16_LE; /* we want 16 bits/sample signed */
	/* little endian; works only on little endian systems! */
	if (ioctl(dev_handle, SNDCTL_DSP_SETFMT, &sndparam) == -1) {
		FILE_LOG(logERROR) << "ioctl: SNDCTL_DSP_SETFMT";
		return -1;
	}

	sndparam = 1;   /* 2 Kanäle */
	if (ioctl(dev_handle, SNDCTL_DSP_STEREO, &sndparam) == -1) {
		FILE_LOG(logERROR) << ("ioctl: SNDCTL_DSP_STEREO");
		return -1;
	}
	if (sndparam != 1) {
		/*	Monovariante?	*/
		FILE_LOG(logERROR) << "soundif: Error, cannot set the channel " <<
				"number to 2";
		return -1;
	}

	ioctl(dev_handle, SNDCTL_DSP_SPEED, &pcm_rate);
	return 0;
}

int MonitorAudioOSS::CloseDevice() {
	close(dev_handle);
	return 0;
}

void *MonitorAudioOSS::Thread() {
	int bytes;
	short *temp_buffer;
	float *left, *right;
	if ((temp_buffer = (short*) malloc(audio_buffer->SampleLen * sizeof (short) * 2)) == NULL) {
	FILE_LOG(logERROR) << "cannot allocate temporary audio buffer";
		exit (-1);
	}

	JThread::ThreadStarted();
	FILE_LOG(logINFO) << "AudioThread " << pcm_name << " is running" ;

	while(run) {
		bytes = read(dev_handle, temp_buffer, audio_buffer->SampleLen * sizeof (short) * 2);
		if (bytes > 0) {
			left = audio_buffer->Left;
			right = audio_buffer->Right;

			/* bytes / Kan�le / bytes pro sample */
			audio_buffer->Samples = bytes / sizeof(short) / 2;

			for (tFramecount i=0; i < audio_buffer->Samples; i++) {
				left[i] = temp_buffer[i*2] / 32768.0;
				right[i] = temp_buffer[i*2+1] / 32768.0;
			}

			DataFromSoundIn(audio_buffer, m_pOwner);
		} else if (bytes < 0) {
			/*	Schliessen des Sounddevices und neustarten	*/
			CloseDevice();
			sleep(1);
			if (InitDevice() < 0) {
				FILE_LOG(logERROR) << "Error initializing PCM device "<< pcm_name ;
			}
		}
	}

	FILE_LOG(logINFO) << "AudioThread " << pcm_name << " has stopped" ;

	free(temp_buffer);
	return NULL;
}

// vim: sw=4 ts=4
