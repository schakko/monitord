/* MonitorModules.h
 *
 *      This file is part of MyMonitor
 *
 *		Copyright (C) 1996
 *          Thomas Sailer (sailer@ife.ee.ethz.ch, hb9jnx@hb9w.che.eu)
 *
 *      Copyright (C) 1998-2002
 *          Markus Grohmann (markus_grohmann@gmx.de)
 *
 *      Copyright (c) 2002
 *          Stephan Effertz (info@stephan-effertz.de)
 *
 *
 *		(Demodulation parts taken from monitor (c) Markus Grohmann)
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
 */

/* ---------------------------------------------------------------------- */

#if !defined(AFX_MYMONMODULES_H__452DBCB3_6D8D_4B6B_9A1D_081EBE3308B1__INCLUDED_)
#define AFX_MYMONMODULES_H__452DBCB3_6D8D_4B6B_9A1D_081EBE3308B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define MAX_SAMPLES 4096
//65536*2   //>>>>>>>> must also be defined in CFft
#define MAX_VOIE 2
#define MAX_SIZE_SAMPLES  1  // WORD
#define MAX_SIZE_INPUT_BUFFER   MAX_SAMPLES*MAX_VOIE*MAX_SIZE_SAMPLES
// #define MAX_SIZE_INPUT_BUFFER   1024
#define DEFAULT_CAL_OFFSET 0x80 // >>>>> depends of you sound card
#define DEFAULT_CAL_GAIN   256.0



/////////////////////////////////////////////////////////////////////////
// Die hdlc Dekodierung scheint eine Art Grundlage zu sein.
// Zumindest wird deren Init-Routine z.B. vom FMS Module aufgerufen
// Deswegen nun dieses "Modul" hier als Basisklasse definiert
//
/////////////////////////////////////////////////////////////////////////

#include "time.h"
#include <vector>
#include <string>
#include "memlock.h"

#include "MonitorModulesResults.h"
#include "xmltools.h"
#include "MonitorLogging.h"

/* ---------------------------------------------------------
   BEGIN: Helper Functions
   ---------------------------------------------------------*/

inline float mac( float *a,  float *b, unsigned int size) {
	float sum = 0;
	unsigned int i;

	FILE_LOG(logDEBUG2) << "processing mac() Block with size=" << size ;
	for (i = 0; i < size; i++)
		sum += (*a++) * (*b++);
	return sum;
}


inline float fsqr(float f) {
	return f*f;
}


/* ---------------------------------------------------------
   END: Helper Functions
   ---------------------------------------------------------*/

/*
 *      translat.c -- Charakter printable translation
 *
 *      Copyright (C) 1996
 *          Thomas Sailer (sailer@ife.ee.ethz.ch, hb9jnx@hb9w.che.eu)
 *
 *      Copyright (C) 1998-2002
 *          Markus Grohmann (markus_grohmann@gmx.de)
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
 * ---------------------------------------------------------------------- */


#define CHARSET_LATIN1

/* ---------------------------------------------------------------------- */

class MonitorModule ;

typedef std::vector<MonitorModule*> MonitorModuleArray ;

class MonitorModule
{
public:
	void SetFunkkanal(int kanal);
	void SetFilter(std::string filter, int modus);
	void SetSampleRate(int rate);
	void SetName(std::string Name);
	virtual void demod(float *buffer, int length);
	MonitorModule();
	virtual ~MonitorModule();

	void setChannelName(std::string name) ;
	void setServerName(std::string name) ;
	void setChannelNum(int num) ;
	std::string getChannelNameHex() ;
	int getChannelNum() ;
	void setConfigData(XMLNode *pConfigNode) ;


protected:
	virtual void parseConfigData(){} ;
	void getCurrentTime() ;
	bool currentTime(std::string & time) ;
	bool makeResponseHeader(int code) ;
	void DebugMessage(std::string message);
	bool MatchFilter(std::string string);
	const char* translate_alpha(unsigned char chr);

	int m_iChannelNum ;
	int m_iFunkkanal;
	int FREQ_SAMP;
	std::string m_lpszName;
	time_t	m_time ; //< Aktuelle Uhrzeit zwischenspeichern
	int m_iFilterModus;
	std::string m_lpszFilter;
	std::string m_serverNameHex ;
	std::string m_channelNameHex ;
	bool m_bTranslate;
	XMLNode* m_pXMLConfig ;
	unsigned char	rxbuf[512], *rxptr;
	unsigned int	rxstate, rxbitbuf;
	unsigned long	rxbitstream;
	MEMLOCK 		m_queueLock ;

};

#endif // !defined(AFX_MYMONMODULES_H__452DBCB3_6D8D_4B6B_9A1D_081EBE3308B1__INCLUDED_)
