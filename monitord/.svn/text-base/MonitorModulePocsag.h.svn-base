/* MyMonModulePocsag.h
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
 *		(Demodulation parts taken from monitor (c) Markus Grohmann, Thomas Sailor)
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
#if !defined(AFX_MYMONMODULEPOCSAG_H__B679D6FA_C954_4D9C_BDBE_920B4374E572__INCLUDED_)
#define AFX_MYMONMODULEPOCSAG_H__B679D6FA_C954_4D9C_BDBE_920B4374E572__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// --------------------------------------------
// 1. POCDEBUG definieren
// 2. POCDEBUG512 oder POCDEBUG1200 definieren
// = Aufname der Audiodaten, des Schmitt-Triggers, des PD-Outputs und des regen. Taktsignals
// als RAW, 32Bit Float, Mono
// --------------------------------------------

//#define POCDEBUG
#ifdef POCDEBUG
	// ACHTUNG ! Dateiname ist fest vorgegeben NUR EINE BAUDRATE DEBUGGEN !
	#define POCDEBUG1200
	//#define POCDEBUG512
#endif

#include "MonitorModules.h"
//#include "regexp.h"
#include "math.h"

#define MAXSAMPLEVALUES 350

class MonitorModulePocsag : public MonitorModule
{
public:
	void SetTrigger(float trigger);
	void demod(float *buffer, int length);
	virtual void demod_se(float *buffer, int length);
	virtual void demod_mg(float *buffer, int length);

	MonitorModulePocsag();
	virtual ~MonitorModulePocsag();

	struct rx {
		unsigned char rx_sync, rx_word, rx_bit, numnibbles;
		char func;
		unsigned long adr;
		unsigned char buffer[128];
		int receiving;
		// bool m_bRXmode ;
		// int m_iPreambelLen
		// bool m_bPreambel_detected ;
		// COleDateTime m_dtPreambel_detected ;
	}  ;

protected:
	bool isSync(unsigned long rxdata);
	float maxVal;
	bool isnumeric(struct rx *rx, std::string & message);
	int lastbit;
	unsigned int dcd_shreg;
	unsigned int sphase;
	unsigned int subsamp;
	int m_iReceivedBits;
	int m_iAlgorithmus;
	
	// -- Fier  SE demod Algorithmus
	float m_fTrigger; // Schmitt-Trigger Grenzwert
	float m_fPLLFaktor; // Max. "Geschwindigkeitsnderung des PLL
	int m_lastVal[50] ; // letzte Werte fr Edgedetektor, Laenge = ?
	int Ud ; // Ausgang Phasendetektor
	int Udiff ; // "Differenziertes Eingangssignal"
	bool didBit ; // Fr aktuelle Tankwelle ein Bit schon empfagen (fallend Flanke im Taksignal)
	int Uphase ; // Taktsignal selbst (1 oder -1)
	int lastUdiff ;// Fr Schmitt-Trigger
	int SPHASEINC_BASE; 

	bool m_negativeCorrection; //Kennzeichner ob SPAHASE ins positive oder ins negative korrigiert werden muss
	
	// Debugging:
	#ifdef POCDEBUG
		FILE *debugFile1, *debugFile2, *debugFile3, *debugFile4;
		float fileBuffer1[50000],fileBuffer2[50000], fileBuffer3[50000] ;
		float *pBuffer1, *pBuffer2, *pBuffer3 ;
		long bufferLen1,bufferLen2, bufferLen3 ;
	#endif
	
	
	void StoreResult(struct rx *rx);
	void RotateString(std::string & buffer,struct rx *rx) ;

	bool m_bRXmode;
	int m_iRXErrorCount ;
	int m_iPreambelLen ;
	bool m_bErrorCorrection;
	int PREAMBEL_MINLEN ;
	int MAX_RX_ERRORS ;
	//COleDateTime m_dtPreambel_detected;
	bool m_bPreambel_detected;
	bool error_correction(unsigned long & rx_data);
	unsigned int SPHASEINC ;
	unsigned long global_rx_data;
	// unsigned short	ctrl;
	struct rx rx_buff[2] ;

	static inline unsigned char even_parity(unsigned long data);
	unsigned int syndrome(unsigned long data);
	void printmessage(struct rx *rx);
	void do_one_bit(struct rx *rx, unsigned long rx_data);
	void rxbit(int bit);
};

#endif // !defined(AFX_MYMONMODULEPOCSAG_H__B679D6FA_C954_4D9C_BDBE_920B4374E572__INCLUDED_)
