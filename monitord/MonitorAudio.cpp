#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "math.h"
#include <iostream>

using namespace std ;

#include "MonitorAudio.h"
#include "MonitorLogging.h"

CAudioBuffer::CAudioBuffer() {
	CAudioBuffer(16384) ;
}
CAudioBuffer::CAudioBuffer(tFramecount len) {
	SampleLen = len;
	Samples = 0;
	/* Laenge pro Kanal */
	ByteLen = SampleLen * sizeof(float);

	Left = (float*) malloc(ByteLen);
	Right = (float*) malloc(ByteLen);

	Ptrs = (float**) malloc(sizeof(float*) * 2);
	Ptrs[0] = Left;
	Ptrs[1] = Right;

	for (unsigned int i=0; i< len; i++) {
		Left[i]  = sin(((float) i)*3.14159265/180.0 ) ;
		Right[i]  = sin(((float) i)*3.14159265/180.0 ) ;
	}
}

CAudioBuffer::~CAudioBuffer() {
	free(Ptrs);
	free(Left);
	free(Right);
	Samples = 0;
	ByteLen = 0;
}


/*****************************************************************************/

MonitorAudio::MonitorAudio(const std::string *name, tSamplerate rate) {
	if (name != NULL) {
		pcm_name = *name;
	}
	pcm_rate = rate;
	/* Audiopuffer ca. 1/3 Sekunden Laenge */
	audio_buffer = new CAudioBuffer(16384);
}

MonitorAudio::~MonitorAudio() {
	delete audio_buffer;
}

void MonitorAudio::Stop(){}

bool MonitorAudio::Start(void* format){return true;}

void MonitorAudio::setDevice (const std::string name, tSamplerate rate) {
	pcm_name = name;
	if (rate != 0) {
		pcm_rate = rate;
	}
}

void MonitorAudio::setCallback (void (* callback)(CAudioBuffer* buffer, void* Owner)) {
	this->DataFromSoundIn = callback;
}


void MonitorAudio::setOwner (void *owner) {
	this->m_pOwner = owner;
}

// vim: sw=4 ts=4 cindent
