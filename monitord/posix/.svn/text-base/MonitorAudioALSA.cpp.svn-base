#include <alsa/asoundlib.h>
#include "MonitorAudioALSA.h"
#include "../MonitorLogging.h"

MonitorAudioALSA::MonitorAudioALSA(const std::string* name, tSamplerate rate)
: MonitorAudio(name, rate) {
	run = false;
}

MonitorAudioALSA::~MonitorAudioALSA() {
	CloseDevice();
}

bool MonitorAudioALSA::Start(void *format) {
	if (InitDevice() < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error initializing PCM device " << pcm_name;
		exit(10);
	}

	run = true;
	JThread::Start();
	return true;
}

void MonitorAudioALSA::Stop() {
	run = false;
}

int MonitorAudioALSA::InitDevice() {
	if ((pcm_name.length() == 0) || (pcm_rate == 0)) {
		FILE_LOG(logERROR) << "[ALSA] InitDevice Argument Error: pcm_name=" << pcm_name << " pcm_rate=" << pcm_rate ;
		return -1;
	}


	int ret;

	snd_pcm_uframes_t pcm_buffer_size = audio_buffer->SampleLen;
	snd_pcm_stream_t pcm_stream = SND_PCM_STREAM_CAPTURE;

	unsigned int periods = 2;
	int direction = 0;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	/* Open PCM. The last parameter of this function is the mode. */
	/* If this is set to 0, the standard mode is used. Possible   */
	/* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */
	/* If SND_PCM_NONBLOCK is used, read / write access to the    */
	/* PCM device will return immediately. If SND_PCM_ASYNC is    */
	/* specified, SIGIO will be emitted whenever a period has     */
	/* been completely processed by the soundcard.                */
	ret = snd_pcm_open(&pcm_handle, pcm_name.c_str(), pcm_stream, 0);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error opening PCM device " << pcm_name << " ret:" << ret << snd_strerror(ret);
		return -1;
	}
	/* Init hwparams with full configuration space */
	ret = snd_pcm_hw_params_any(pcm_handle, hwparams);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Can not configure this PCM device " << pcm_name << ". " << ret << "(" << snd_strerror(ret) << ")" ;
		return -1;
	}
	/* Set access type. This can be either    */
	/* SND_PCM_ACCESS_RW_INTERLEAVED or       */
	/* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
	/* There are also access types for MMAPed */
	/* access, but this is beyond the scope   */
	/* of this introduction.                  */
	ret = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting access " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
		return -1;
	}
	/* Set number of channels */
	ret = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting channels " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
		return -1;
	}
	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	ret = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &pcm_rate, &direction);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting rate " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
		return -1;
	}
	/* Set sample format */
	/* FLOAT LE -1.0 .. 1.0 */
	ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_FLOAT_LE);
	//ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting format " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
		return -1;
	}
	/* Set buffer size */
	ret = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams, &pcm_buffer_size);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting buffer size " <<	pcm_name << ". " <<  (int)pcm_buffer_size << " - " << ret <<"(" << snd_strerror(ret)<<")";
	}
	/* Set number of periods. Periods used to be called fragments. */
	ret = snd_pcm_hw_params_set_periods_near(pcm_handle, hwparams, &periods, &direction);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting periods  " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
	}
	FILE_LOG(logINFO) << "[ALSA] Using pcm_buffer_size=" <<(int)pcm_buffer_size << " periods=" << periods ;

	/* Apply HW parameter settings to */
	/* PCM device and prepare device  */
	ret = snd_pcm_hw_params(pcm_handle, hwparams);
	if (ret < 0) {
		FILE_LOG(logERROR) << "[ALSA] Error setting HW params " <<	pcm_name << ". " << ret <<"(" << snd_strerror(ret)<<")";
		return -1;
	}

	return 1;
}

int MonitorAudioALSA::CloseDevice() {
	snd_pcm_close(pcm_handle);
	return 0;
}

void* MonitorAudioALSA::Thread() {
	signed int num_samples;

	JThread::ThreadStarted();

	while(run) {
		num_samples = snd_pcm_readn(pcm_handle, (void**)audio_buffer->Ptrs, audio_buffer->SampleLen);
		if (num_samples > 0) {
			audio_buffer->Samples = num_samples;

			DataFromSoundIn(audio_buffer, m_pOwner);
		} else if (num_samples == -EPIPE) {
			int err = snd_pcm_prepare(pcm_handle);
			if (err < 0) {
				FILE_LOG(logERROR) << "[ALSA] Can't recovery from underrun, prepare failed: " << snd_strerror(err);
			}
		} else if (num_samples < 0) {
			FILE_LOG(logERROR) <<  "[ALSA] Read error " <<  num_samples << "(" << snd_strerror(num_samples) << ")" ;
			/*	Schliessen des Sounddevices und neustarten	*/
			CloseDevice();
			sleep(1);
			if (InitDevice() < 0) {
				FILE_LOG(logERROR) << "[ALSA] Error initializing PCM device " << pcm_name ;
			}
		}
	}

	return NULL;
}

// vim: sw=4 ts=4 cindent
