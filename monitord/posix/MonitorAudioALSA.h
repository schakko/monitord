/*
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------- */

#ifndef MONITORD_POSIX_MONITORAUDIOALSA_H_
#define MONITORD_POSIX_MONITORAUDIOALSA_H_

#include <alsa/asoundlib.h>

#include <monitord/MonitorAudio.h>

class MonitorAudioALSA : public MonitorAudio {
	public:
		// const char *device_name: Devicename
		// unsigned int sample_rate: Sample Rate in Hz
		MonitorAudioALSA(const std::string* = NULL, tSamplerate = 0);
		~MonitorAudioALSA();

		void Stop();
		bool Start(void* format=NULL);
	private:
		bool run;
		snd_pcm_t *pcm_handle;
		int InitDevice();
		int CloseDevice();
		void* Thread();
};

#endif

// vim: sw=4 ts=4
