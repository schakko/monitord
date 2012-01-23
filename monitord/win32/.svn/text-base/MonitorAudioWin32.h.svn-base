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

#ifndef MONITORD_POSIX_MONITORAUDIOWIN32_H_
#define MONITORD_POSIX_MONITORAUDIOWIN32_H_

#include <monitord/MonitorAudio.h>

#define MAX_BUFFERS 2
#define DATABLOCK_SIZE 8192*4

class MonitorAudioWin32 : public MonitorAudio {
	public:
		MonitorAudioWin32(const std::string* = NULL, const tSamplerate = 0);
		~MonitorAudioWin32();

		void Stop();
		bool Start(void* format=NULL);

		static void CALLBACK waveInProc(	HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance,  DWORD_PTR dwParam1, DWORD_PTR dwParam2 );

	private:
		bool run;

		int InitDevice();
		int CloseDevice();

		void* Thread();

		HWAVEIN		hwi;
		HANDLE		handle;
		LPWAVEHDR	whin[MAX_BUFFERS];
		int			next_buffer;
		bool		m_bBufferInUse ;
		void waveInErrorMsg(MMRESULT result, std::string addstr);
};

#endif

// vim: sw=4 ts=4
