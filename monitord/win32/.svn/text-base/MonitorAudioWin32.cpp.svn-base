
#include <iostream>

#include "MonitorAudioWin32.h"
#include "math.h"
#include "../convert.h"
#include "../MonitorExceptions.h"
#include "../MonitorLogging.h"

#define usleep Sleep
#define short2float (1.0/32768.0)

using namespace std ;

MonitorAudioWin32::MonitorAudioWin32(const std::string* name, tSamplerate rate)
: MonitorAudio(name, rate) {
	run = false;
	hwi=NULL ;
	m_bBufferInUse=false ;
}

MonitorAudioWin32::~MonitorAudioWin32() {

}

bool MonitorAudioWin32::Start(void *format) {
	JThread::Start();
	return true;
}

void MonitorAudioWin32::Stop() {
	run = false;
	usleep(2000) ;
	CloseDevice();
}

int MonitorAudioWin32::InitDevice() {
	if ((pcm_name.length() == 0) || (pcm_rate == 0)) {
		FILE_LOG(logERROR) << "[WINMM] InitDevice Argument Error: pcm_name="
			<< pcm_name << ", pcm_rate=" << pcm_rate ;
		return -1;
	}

	/* FIXME es wird einfach die erste Soundkarte geoeffnet */

	WAVEINCAPS   wic;
	WAVEFORMATEX wfx;
	UINT         nDevId;
	MMRESULT     rc;
	//UINT         nMaxDevices = waveInGetNumDevs();

	hwi = NULL;

	handle = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (handle == NULL) {
		return -1;
	}
	std::string devString="/dev/dsp" ;
	std::string deviceName = pcm_name;

	nDevId = 0;
	if (deviceName.substr(0,devString.size())==devString)
	{
		nDevId=convertToInt(deviceName.substr(devString.size()));
		FILE_LOG(logINFO) << "using windows device #" << nDevId  ;
	}
	if ((nDevId<0))
	{
		nDevId=0 ;
	}


	rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));

	if (rc == MMSYSERR_NOERROR) {
		FILE_LOG(logINFO) << "starting wavein for sounddevice: \"" << wic.szPname << "\"" ;
		wfx.nChannels      = 2;      // stereo
		wfx.nSamplesPerSec = 22050;  // 22.05 kHz (22.05 * 1000)
		wfx.wFormatTag      = WAVE_FORMAT_PCM;
		wfx.wBitsPerSample  = 16;
		wfx.nBlockAlign     = wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize          = 0;

		//	rc = waveInOpen(&hwi, nDevId, &wfx, (DWORD)handle, 0, CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
		rc = waveInOpen(&hwi, nDevId, &wfx, (DWORD_PTR)MonitorAudioWin32::waveInProc, (DWORD_PTR) this, CALLBACK_FUNCTION ); // | WAVE_FORMAT_DIRECT
		if (rc == MMSYSERR_NOERROR) {
			// break;
		} else {
			waveInErrorMsg (rc, "waveInOpen");
			return -1;
		}
	} else {
		waveInErrorMsg (rc, "waveInGetDevCaps");
		return -1 ;
	}


	// device not found, error condition
	//..................................

	if (hwi == NULL) {
		return -1;
	}

	// allocate two WAVEHDR buffer blocks
	//...................................
	for (int i = 0; i < MAX_BUFFERS; i++) {
		whin[i] = new WAVEHDR;
		if (whin[i]) {
			whin[i]->lpData = new char[DATABLOCK_SIZE];
			whin[i]->dwBufferLength = DATABLOCK_SIZE;
			whin[i]->dwFlags = 0;
		}
	}
	next_buffer = 0;

	return 1;
}

int MonitorAudioWin32::CloseDevice() {
	FILE_LOG(logINFO) << "Closing sound device..."  ;
	int i=0 ;
	// waveInReset Stops the device and sets all the pending buffers to zero.
	if (hwi != NULL) {

		waveInStop(hwi) ;
		// waveInReset friert den laufenden prozeß ein. Deswegen waveInStop
		//waveInReset(hwi);

		// Unprepare headers
		//..................

		for (i = 0; i < MAX_BUFFERS; i++) {
			waveInUnprepareHeader(hwi, whin[i], sizeof(WAVEHDR));
		}

		waveInClose(hwi);

		// I do not know if cleanup is really necessary since the buffers were dynamically allocated.
		// I think the memory is freed once the process is destroyed.

		// free the WAVEHDR buffer blocks
		//...............................

		for (i = 0; i < MAX_BUFFERS; i++) {
			if (whin[i] != NULL) {
				delete whin[i]->lpData;
				delete whin[i];
				whin[i] = NULL;
			}
		}
	}
	FILE_LOG(logINFO) << "sound device closed"  ;
	return 0;
}


void MonitorAudioWin32::waveInProc(HWAVEIN hwin, UINT uMsg, DWORD_PTR dwInstance,  DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
	MonitorAudioWin32* me=(MonitorAudioWin32*) dwInstance ;
	WAVEHDR* pwhin= (WAVEHDR*) dwParam1;
	MMRESULT rc;

	switch (uMsg)
	{
	case WIM_DATA:
		rc=waveInUnprepareHeader(hwin,pwhin,sizeof(WAVEHDR));
		me->waveInErrorMsg (rc, "waveInUnprepareHeader");


    	/*
    	 while ((me->m_bBufferInUse)==true)
    	{
    		printf("Win32AudioThread waiting\n") ;
    		Sleep(10) ;
    	}
    	*/

    	me->m_bBufferInUse=true ;

		/* convert buffer and send off to pipe */
		me->audio_buffer->Samples = pwhin->dwBytesRecorded / sizeof (short) / 2;
		short *temp_buffer = (short *) pwhin->lpData;

		for (unsigned int i=0; i< (me->audio_buffer->Samples); i++) {
			me->audio_buffer->Left[i]  = temp_buffer[i*2]   * short2float;
			me->audio_buffer->Right[i] = temp_buffer[i*2+1] * short2float;
		}
		SetEvent(me->handle) ;
		/** Hauptprozess mitteilen, daß er einen Datenblock verarbeiten kann
		 * wir verlassen aber die Callback Funktion trotzdem schonmal
		 * FIXME: Koennte hier eine Race-Condition auftreten ?
		 */

		/* recycle buffer */
		rc = waveInPrepareHeader(hwin, pwhin, sizeof(WAVEHDR));
		if (rc == MMSYSERR_NOERROR) {
			rc = waveInAddBuffer(hwin, pwhin, sizeof(WAVEHDR));
			me->waveInErrorMsg (rc, "waveInAddBuffer");
		} else {
			me->waveInErrorMsg (rc, "waveInPrepareHeader");
		}
    }
}

void* MonitorAudioWin32::Thread() {
	int rc;

	rc=InitDevice() ;
	if ( rc < 0) {
		FILE_LOG(logERROR) << "[WINMM] Error initializing PCM device " << pcm_name;

		return NULL ;
	}

	/* prepare buffers */
	for (int i = 0; i < MAX_BUFFERS; i++) {
		rc = waveInPrepareHeader(hwi, whin[i], sizeof(WAVEHDR));

		// add buffers to the input queue
		if (rc == MMSYSERR_NOERROR) {
			rc = waveInAddBuffer(hwi, whin[i], sizeof(WAVEHDR));
			waveInErrorMsg (rc, "waveInAddBuffer");
		} else {
			waveInErrorMsg (rc, "waveInPrepareHeader");
		}
	}

	/* start waveIn */
	rc = waveInStart(hwi);
	waveInErrorMsg (rc, "waveInStart");

	/* check for ready buffers on every event */

	JThread::ThreadStarted();
	run = true;

	while(run) {
		int res = WaitForSingleObject(handle,1000);
		if (res==WAIT_OBJECT_0) {
			/**
			 * Wenn die Callbackfunktion den audio_buffer gefuellt hat
			 * schickt sie ein Event.
			 * Erst dann landen wir hier und verarbeiten dann auch den
			 * Audio_buffer
			 * Sinn dahinter ist, die Callbackfunktion schnell wieder zu
			 * verlassen
			 */
			ResetEvent(handle) ;
			DataFromSoundIn(audio_buffer, m_pOwner);
			m_bBufferInUse=false ;
		}
	}

	FILE_LOG(logINFO) << "stopping soundcard" ;
	/* stop waveIn */
	waveInStop(hwi);
	Sleep(1000) ;
	FILE_LOG(logINFO) << "soundcard stopped"  ;

	return NULL;
}

void MonitorAudioWin32::waveInErrorMsg(MMRESULT result, std::string addstr)
{
	if (result != MMSYSERR_NOERROR) {
		char errorbuffer[100];
		waveInGetErrorText (result, errorbuffer, 100);
		FILE_LOG(logERROR) << "[WINMM] Error " << result << ":" << errorbuffer << " (" << addstr << ")" ;
	}
}

// vim: sw=4 ts=4 cindent
