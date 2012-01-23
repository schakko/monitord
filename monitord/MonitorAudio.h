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

#ifndef MONITORD_AUDIO_H_
#define MONITORD_AUDIO_H_

#include <string>

#include <jthread-1.2.1/src/jthread.h>

/** Samplerate in Hz */
typedef unsigned int tSamplerate;

/** Anzahl Frames (Samples * Kanaele) */
typedef unsigned int tFramecount;

/** Position in Frames (Samples * Kanaele) */
typedef unsigned int tFrameposition;

class CAudioBuffer
{
	public:
		// int len: Länge des Buffers in Frames
		CAudioBuffer();
		CAudioBuffer(tFramecount);
		~CAudioBuffer();

		size_t ByteLen;
		tFramecount SampleLen;
		tFramecount Samples;
		float* Left;
		float* Right;
		float **Ptrs;
};



/** Basisklasse f�r den Zugriff auf Soundkarten */
class MonitorAudio : public JThread {
	public:
		MonitorAudio(const std::string* = NULL, const tSamplerate=0);
		~MonitorAudio();

		virtual void Stop();
		virtual bool Start(void* format=NULL);

		void setCallback (void (* callback)(CAudioBuffer* buffer, void* Owner));
		void setDevice (const std::string name, const tSamplerate rate=0);
		void setOwner (void *owner);

	protected:
		// pointer to callback function
		void (*DataFromSoundIn)(CAudioBuffer* buffer, void* Owner);
		void* m_pOwner;

		CAudioBuffer *audio_buffer;
		std::string pcm_name;
		tSamplerate pcm_rate;
};

#endif

// vim: sw=4 ts=4
