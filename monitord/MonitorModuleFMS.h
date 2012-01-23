/* MyMonModulesFMS.h
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
 *          Stephan Effertz (mail@stephan-effertz.de)
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

#if !defined(AFX_MYMONMODULEFMS_H__680AB4C3_EAB0_4D03_A913_869FB97EA730__INCLUDED_)
#define AFX_MYMONMODULEFMS_H__680AB4C3_EAB0_4D03_A913_869FB97EA730__INCLUDED_

#if _MSC_VER > 1000
// #pragma once
#endif // _MSC_VER > 1000

#include "MonitorModules.h"
#include "math.h"
//#include "resource.h"

// #define FMSDEBUG

#define FREQ_MARK  1200
#define FREQ_SPACE 1800
#define BAUD       1200
#define SUBSAMP    1
// war:2

#define	LINELEN	240
#define	ITEMLEN	50

// F�r die SE - FMS Auswertung
// kann bis 44100 Hz eine ganze FMS Aussendung erfassen
// (44100Hz/1200Bit/s)*56Bit=2058 + Auswertungsspielraum



#define FMS_BUFFERSIZE 10000

struct LINE {
	char	string[LINELEN], subst[ITEMLEN];
	long	value;
	struct LINE	*next;
};

typedef struct LINE	Line ;

//

/* ---------------------------------------------------------------------- */

#ifdef WIN32
#ifndef M_PI
const float M_PI = 3.14159265358979 ;
#endif
#endif

static const unsigned short crc_table[] = {
	48, 48, 48, 48, 48, 48, 48, 27,
	48, 48, 48, 18, 48,  5, 26, 48,
	48, 48, 48, 37, 48,  1, 17, 48,
	48,  8,  4, 48, 25, 48, 48, 48,
	48, 48, 48, 42, 48, 22, 36, 48,
	48, 30,  0, 48, 16, 48, 48, 13,
	48, 48,  7, 48,  3, 48, 48, 10,
	24, 48, 48, 32, 48, 48, 48, 48,
	48, 48, 48, 48, 48, 46, 41, 48,
	48, 40, 21, 48, 35, 48, 48, 45,
	48, 20, 29, 48, 48, 48, 48, 39,
	15, 48, 48, 44, 48, 34, 12, 48,
	48, 28, 48, 48,  6, 48, 48, 19,
	 2, 48, 48, 38, 48, 48,  9, 48,
	23, 48, 48, 43, 48, 14, 31, 48,
	48, 11, 48, 48, 48, 48, 48, 33,
};

/** Auswerter f�r FMS Aussendungen.
 *  Werte die �bergebenen Tondaten aus und sucht nach FMS Aussendungen
 *  Gefundene Daten werden per XML an den Hauptproze� zur�ckgegeben
 */
class MonitorModuleFMS : 
	public MonitorModule {
public:
	bool IsValid();
	bool test_rxbit(unsigned char bit);
	void ClearRXBuf();
	int PruefeTelegramm(int start);
	int SucheSync(int start);
	void demod_neu(float *buffer, int length);
	void demod_se(float *buffer, int length);
	virtual void demod(float* buffer, int length);
	MonitorModuleFMS(int sampleRate=22050,int vorlaufbits=8, bool crccheck=0, float signallevel=0.4, bool ignore_Wiederholung=true, bool ignore_Quittung=true, int algorithm=1,bool error_correction=false, bool force_preambel=false) ;
	MonitorModuleFMS(unsigned int sampleRate,XMLNode *pConfig) ;
	virtual ~MonitorModuleFMS();

protected:
	int distCounter ;
	int rules();
	int crc_check(int offset);
	int decode(bool test=false);
	int dcd_buffer[FMS_BUFFERSIZE+100] ;
	int CORRLEN ;
	long SPHASEINC ;
	
	virtual void parseConfigData() ;
	bool rxbit(unsigned char bit, bool test=false);
	void initialize(int sampleRate, int vorlaufbits, bool crccheck, float signallevel, bool ignore_Wiederholung, bool ignore_Quittung, int algorithm, bool error_correction, bool force_preambel);
	
	/*******************************************************
	Decoder Variables & Coef. calculation
	*******************************************************/
	float lp1_c[3],lp2_c[3],bp0_c[3],bp1_c[3];
	float lp1_b[4],lp2_b[4],bp0_b[4],bp1_b[4];

	unsigned short bit_phase ;

	// Das geht bis max 44100 Hz Samplerate (MaxWert=Samplerate/Baud) (44100/1200=37) ;
	float corr_mark_i[40];
	float corr_mark_q[40];
	float corr_space_i[40];
	float corr_space_q[40];

	float m_fZwPuffer[80] ; // Um den Bereich der L�nge CORRLEN am Ende des Blocks zwischenzuspeichern

	//short int int_corr_mark_i[40];
	//short int int_corr_mark_q[40];
	//short int int_corr_space_i[40];
	//short int int_corr_space_q[40];

	unsigned int dcd_shreg ;
	unsigned int sphase ;
	unsigned int lasts[11] ;
	unsigned int lastout ;
	unsigned int subsamp ;
	short	quittung ;
	
	struct FMS {
		unsigned short
			bos[1],
			land[1],
			ort[2],
			kfz[4],
			stat[1],
			bst[1],
			dir[1],
			tki[1],
			crc[7]; 
			short txtnr ;
	} fms ;


	struct VALS{
		short	quality ;
		unsigned short txtinc, bit, ctrl ;
	} vals ;

	
protected:
	void fms_bin(short offset, char *c);
	void DisplayResult(const std::string input="");
	bool CheckForQuittung();
	bool CheckForDupes();
	void StoreForDupeCheck();
	void fms_txt(short offset,char *c);
	bool error_correction();
	bool IsRelatedMessage();
	void StoreForReverseCheck();
	void ErrorOut();
	bool IsSyncWord(unsigned long);
	bool feedBuffer(int & start);
	bool RMExists();
	bool rx_sample(int x);
	void set_filters(float f0, float f1, float dr);
	void gen_coef(int tipo, float f0, float Q, float *pcoef);
	float biq_hp(float x,float *pcoef,float *buf);
	float biq_bp(float x,float *pcoef,float *buf);
	float biq_lp(float x,float *pcoef,float *buf);
	
	//int m_iSkipCounter;
	bool m_bErrorCorrection;
	int m_iLastSyncState, m_bPrintError ;
	int m_iAlgorithmus;
	unsigned int	m_iLastrxstate ;
	float m_fSignallevel;
	Line* txtbuf;
	std::string m_lpszStatusTabelle;
	float m_PLLFaktor;
	int SPHASEINC_BASE;
	int summe;
	struct FMS m_LastMessage;
	struct FMS m_ReverseCheckMessage ;
	

	float m_fPLLFaktor ;
	float Ud ;
	
	#ifdef FMSDEBUG
	FILE *debugFile1, *debugFile2, *debugFile3, *debugFile4;
	float fileBuffer1[50000],fileBuffer2[50000], fileBuffer3[50000] ;
	float *pBuffer1, *pBuffer2, *pBuffer3 ;
	long bufferLen1,bufferLen2, bufferLen3 ;
	#endif
	
};

#endif // !defined(AFX_MYMONMODULEFMS_H__680AB4C3_EAB0_4D03_A913_869FB97EA730__INCLUDED_)
