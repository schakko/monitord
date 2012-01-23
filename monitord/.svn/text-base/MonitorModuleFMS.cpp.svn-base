/** MonitorModuleFMS.cpp
 *
 *      This file is part of MyMonitor / BOSAssistent / monitor 2.0
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

#include "MonitorModuleFMS.h"
#include <math.h>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include "stdio.h"
#include "convert.h"
#include "time.h"
#include "MonitorLogging.h"

#include <iostream>
using namespace std;

#include "memlock.h"
#include "base64.h"
#include "MonitorConfiguration.h"

#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif

#define MINPREAMBEL 8
#define READ_AHEAD 80*CORRLEN

// Wenn die nachfolgende Variable Definiert ist, wird der mrtty decode f?r FMS genutzt
// Bei MRTTY sollte SUBSAMP auf 1 definiert sein ! Sonst geht NIX !
//#define MRTTY 0

#ifdef WIN32
//#define MON_DEBUG
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MonitorModuleFMS::MonitorModuleFMS(unsigned int sampleRate,XMLNode* pConfig)
{
	int signallevel=10000 ;

	int vorlaufbits=getNodeInt(*pConfig,"syncbits",8) ;
	int crccheck=getNodeInt(*pConfig,"crc-check",1) ;
	int algorithm=getNodeInt(*pConfig,"algorithmus",0) ;

	initialize(	sampleRate,
						vorlaufbits,
						crccheck,
						(((float)(signallevel))/65535.0),
						false,
						false,
						algorithm,
						false,
						false);
}


MonitorModuleFMS::MonitorModuleFMS(int sampleRate, int vorlaufbits, bool crccheck, float signallevel, bool ignore_Wiederholung, bool ignore_Quittung, int algorithm, bool error_correction, bool force_preambel)
{
	initialize(sampleRate, vorlaufbits, crccheck,  signallevel, ignore_Wiederholung, ignore_Quittung, algorithm, error_correction,  force_preambel);
	}

void MonitorModuleFMS::initialize(int sampleRate, int vorlaufbits, bool crccheck, float signallevel, bool ignore_Wiederholung, bool ignore_Quittung, int algorithm, bool error_correction, bool force_preambel)
{
	float f;
	int i;
	FILE_LOG(logDEBUG) << "FMS Parameter" ;
	FILE_LOG(logDEBUG) << "sample : " << sampleRate  ;
	FILE_LOG(logDEBUG) << "syncbits: " << vorlaufbits  ;
	FILE_LOG(logDEBUG) << "crc    : " << crccheck  ;
	FILE_LOG(logDEBUG) << "level  : " << signallevel  ;
	FILE_LOG(logDEBUG) << "Algo   : " << algorithm  ;
	FILE_LOG(logDEBUG) << "ignW   : " << ignore_Wiederholung ;
	FILE_LOG(logDEBUG) << "ignQ   : " << ignore_Quittung ;
	FILE_LOG(logDEBUG) << "ErrC   : " << error_correction ;
	FILE_LOG(logDEBUG) << "force   : " << force_preambel  ;

	m_bErrorCorrection=error_correction ;
	m_iAlgorithmus=algorithm ;
	rxstate=0 ;
	distCounter=0 ;


	bit_phase=0 ;
	m_iLastSyncState=0 ;

	summe=0 ;

	FREQ_SAMP=sampleRate ;

	CORRLEN=(int) (FREQ_SAMP / BAUD) ;
	SPHASEINC=(0x10000u * BAUD * SUBSAMP / FREQ_SAMP) ;
	SPHASEINC_BASE=(0x10000u * BAUD * SUBSAMP / FREQ_SAMP) ;

	m_fSignallevel=signallevel ;

	m_lpszStatusTabelle="status" ;
	memset(&fms,0,sizeof(FMS)) ;
	memset(&lasts,0,sizeof(unsigned int[11])) ;
	memset(&m_LastMessage,0,sizeof(FMS)) ;

	lastout=0 ;
	quittung=0 ;
	subsamp=0 ;
	sphase=0 ;
	//
	memset(&vals, 0, sizeof(VALS));

	rxstate = 0;

	for (f = 0, i = 0; i < CORRLEN; i++) {
		corr_mark_i[i] = cos(f);
		corr_mark_q[i] = sin(f);
		//int_corr_mark_i[i] = (int) corr_mark_i[i]*32768 ;
		//int_corr_mark_q[i] = (int) corr_mark_q[i]*32768 ;

		f += 2.0 * M_PI * (float) FREQ_MARK / (float) FREQ_SAMP;
	}

	for (f = 0, i = 0; i < CORRLEN; i++) {
		corr_space_i[i] = cos(f);
		corr_space_q[i] = sin(f);
		//int_corr_space_i[i] = (int) corr_space_i[i]*32768 ;
		//int_corr_space_q[i] = (int) corr_space_q[i]*32768 ;

		f += 2.0 * M_PI * (float) FREQ_SPACE / (float) FREQ_SAMP;
		m_fZwPuffer[i]=0.0 ;
		m_fZwPuffer[CORRLEN+i]=0.0 ;
	}


	vals.quality=vorlaufbits ;
	vals.ctrl=0 ;

	if (crccheck) // Wenn CRC-Check, dann um 20 erh?hen
		vals.quality+=20 ;

	m_lpszName="FMS" ;
	//m_bSuppressQuittung=true ;
	//m_bSuppressDupes=true ;



	set_filters(FREQ_SPACE,FREQ_MARK,BAUD);


	#ifdef FMSDEBUG
		debugFile1 = fopen("fms_in.raw", "wb");
		debugFile2 = fopen("fms_takt.raw", "wb");
		debugFile3 = fopen("fms_pfd.raw", "wb");
		debugFile4 = fopen("fms_4.raw", "wb");
	#endif
}


MonitorModuleFMS::~MonitorModuleFMS()
{
	#ifdef FMSDEBUG
		fclose(debugFile1) ;
		fclose(debugFile2) ;
		fclose(debugFile3) ;
		fclose(debugFile4) ;
	#endif
}

void MonitorModuleFMS::parseConfigData()
{

}

////////////////////////////////////////////////////////////////////
// Sucht im Bereitgestellten Datenpuffer (*buffer) nach enthaltenen
// Bits gem. FMS Codierung
// Jedes einzelne Bit wird an rxbit ?bergeben und dort im Kontext
// analysiert
//

int counter =0 ;


void MonitorModuleFMS::demod(float *buffer, int length)
{
	m_fSignallevel=0.3 ;

	if (m_iAlgorithmus==1)
	{
		demod_se(buffer,length) ;
		return ;
	} ;

	float f, ampl_mark, ampl_space ;

	if (subsamp) {
		int numfill = SUBSAMP - subsamp;
		if (length < numfill) {
			subsamp += length;
			return;
		}
		buffer += numfill;
		length -= numfill;
		subsamp = 0;
	}

 	for (; length >= max(SUBSAMP, CORRLEN) ; length -= SUBSAMP, buffer += SUBSAMP)
	{
		ampl_mark=	fsqr(mac(buffer, corr_mark_i, CORRLEN)) +
					fsqr(mac(buffer, corr_mark_q, CORRLEN)) ;
		ampl_space=	fsqr(mac(buffer, corr_space_i, CORRLEN)) +
					fsqr(mac(buffer, corr_space_q, CORRLEN));

		f =  ampl_mark - ampl_space ;

		dcd_shreg <<= 1;
		dcd_shreg |= (f > 0);

		// check if transition
		//
		if ((dcd_shreg ^ (dcd_shreg >> 1)) & 1) {
			if (sphase < (0x8000u - (SPHASEINC / 2)))
				sphase += SPHASEINC / 8;
			else
				sphase -= SPHASEINC / 8;
		}
		sphase += SPHASEINC;


		summe=summe + ( (f > 0) ? 1 : -1) ;

		if (sphase>=0x10000u)
		{
			sphase &= 0xffffu;
			// curbit = dcd_shreg & 1;

			if ( (summe >= (int) (m_fSignallevel * ((float) (FREQ_SAMP/SPHASEINC_BASE))) )  || (-summe >= (int) (m_fSignallevel * ((float) (FREQ_SAMP/SPHASEINC_BASE)) ) ))
			//(f >= m_fSignallevel) || (f <=-m_fSignallevel)) // Es muss schon ein wenig dahinterstehen ;-)
			{
				// TRACE1("summe: %d\n",summe) ;
				int bit=(summe >= 0) ? 1 : 0 ;
				rxbit(bit);

			} else {
				// ggf. laufende Auswertung abbrechen, da Signal zu schwach
				//
				//TRACE2("FMS: Low Power Signal %d < %d \n",summe,(int) (m_fSignallevel * ((float)(FREQ_SAMP/SPHASEINC_BASE)) )) ;
				if (rxstate) rxstate=0 ;
			} ;
			summe=0 ;
		}
	}

	subsamp = length;
}

///////////////////////////////////////////////////////////
// Verarbeiten des Empfangenen Bits und Untersuchung
// im Kontext bisher empfangener Daten
//

bool MonitorModuleFMS::rxbit(unsigned char bit, bool test)
{
	// #define ITEMLEN 80
	bool returnValue=false ;

	unsigned int	i = 0, j = 0;
	unsigned int filter= 0xffff >> (15 - (vals.quality >= 20 ? vals.quality - 20 : vals.quality));
	unsigned int sync  = 0xff1a & filter;
	char	c[ITEMLEN], *p;
	char	kontrollString[ITEMLEN] ;


	/*	F?llen des Vorlaufstromes aus dem Overflow vom Rxbitstream und Schieben	*/
	for(;i < 10; i++){
		lasts[i] <<= 1;
		lasts[i] |= lasts[i + 1] >> 31;
	}
	lasts[10] <<= 1;
	lasts[10] |= rxbitstream >> 31;
	rxbitstream <<= 1;
	rxbitstream |= !!bit;

	// CString outstring ;
	// itoa(rxbitstream, outstring.GetBuffer(20),16) ;
	// TRACE(outstring) ;

	unsigned int testen=(rxbitstream & filter) ;
	if (!rxstate && (testen==sync)  ) {
		/*	ein Syncwort wurde gefunden	*/
		// TRACE0 ("->SYNC<-") ;
		if (m_iLastSyncState>0)
		{
			m_bPrintError=true ;
			//TRACE0 ("->SYNC<-") ;
			// //DebugMessage("-> SYNC-WORT <- gefunden") ;
		} else {
			m_bPrintError=false ;
			// //DebugMessage("Ohne Pr?ambel: -> SYNC-WORT <- gefunden") ;
		}
		// counter = -1 ;

		rxstate = 1; // In den Empfangsmodus schalten
		rxptr = rxbuf; // und das nachfolgende Auswerten
		return false ;
	}

	if (rxstate)
	{
		if (rxstate++ <= 48)
		{
			rxbitbuf |= !!bit;
			*rxptr++ = !!bit;
			if (rxstate == 49)
			{
				*fms.stat = (unsigned short) 0;

				/*	Dekodiere das empfangene Codewort	*/
				i = decode(test);

				if (i) returnValue=true ;

				if (!i)
				{
					if (m_iLastSyncState>0)
					{
						ErrorOut() ;
						m_iLastSyncState=0 ;
					}
					/*
					 *	CRC-Fehler
					 *	Zur?ckschieben des Buffers f?r Neutests
					 */
					rxbitstream = lasts[9];
					rxbitstream <<= 16;
					rxbitstream |= lasts[10] >> 16;

					for (i = 10; i > 1; i--) {
						lasts[i] = lasts[i - 2];
						lasts[i] <<= 16;
						lasts[i] |= lasts[i - 1] >> 16;
					}
					lasts[1] = lasts[0] >> 16;
					rxstate--;

					while (rxstate--) {

						for(i = 0; i < 10; i++){
							lasts[i] <<= 1;
							lasts[i] |= lasts[i + 1] >> 31;
						}
						lasts[10] <<= 1;
						lasts[10] |= rxbitstream >> 31;
						p = (char*) rxbuf;
						rxbitstream <<= 1;
						rxbitstream |= *p;
						for (i = 0; i < rxstate; i++, p++) *p = *(p + 1);
						rxptr--;

						if ((rxbitstream & filter) == sync) {

							/*	ein Syncwort wurde gefunden	*/
							p = (char*) rxbuf;
							for (; j < rxstate; j++) {
								for(i = 0; i < 10; i++){
									lasts[i] <<= 1;
									lasts[i] |= lasts[i + 1] >> 31;
								}
								lasts[10] <<= 1;
								lasts[10] |= rxbitstream >> 31;
								rxbitstream <<= 1;
								rxbitstream |= *p++;
							}
							rxstate++;
							return false;
						}
					}
					rxstate = 0;
				}
				else
				{
					returnValue=true ;
					if (i < 2) {
						/*
						 *	Telegramm ok, wurde ausgegeben
						 */
						rxstate = 0;
					}
					else {	/*	i == 2 Text?bertragung	*/
						/*	Buffer vorbereiten	*/
						txtbuf = (Line *) malloc (sizeof(Line));
						memset(txtbuf, 0, sizeof(Line));
						txtbuf->next = (Line *) malloc (sizeof(Line));
						memset(txtbuf->next, 0, sizeof(Line));
					}
				}
				return returnValue ;
			}
		}
		else
		{
			/*	Text?bertragung	*/
			Line *txt;

			*rxptr++ = !!bit;

			if (rxstate == 97)
			{
				memset(c, 0, 5);
				memset(c, 46, 4);
				txt = txtbuf->next;

				while (txt->next != NULL) txt = txt->next;

				if (!vals.txtinc)
				{
					memcpy (kontrollString,c,ITEMLEN) ;

					if (!decode(test))
					{
						rxstate = 0;
						return false ;
					}

					returnValue=true ;
					if (fms.dir[0]==0) // Bein Fz->Lst keine ?bersetzung von Sonderzeichen !
					{
						m_bTranslate=false ; // Fz-Daten nicht ?bersetzen !
						// if (!crc_check(48))
						//{
							fms_bin(48,c);

							fms.txtnr=2 ; // Ausgedachter Wert 0xb schlie?t es ja ab
						//}
					} else {
						if (!crc_check(48))
						{
							m_bTranslate=false ;
							fms_txt(48,kontrollString);
							m_bTranslate=true ;
							fms_txt(48,c);

							fms.txtnr= kontrollString[0];
							//TRACE1("L?nge: %d\n",fms.txtnr) ;
							// *c = ' ';
						}
					} ;

					vals.txtinc = 1;
				}
				else
				{
					if (fms.dir[0]==0)
					{
						fms_bin(0,c) ;
					}else {
						if (!crc_check(0))
						{
							fms_txt(0, c);
						}
					} ;

				} ;

				if (fms.dir[0]==0)
				{
					char tempString [20]="" ;

					char rotateByte[4] ; // =((c[i]>>4) & 0xf) + ((c[i] & 0xf) << 4) ;

					/*
					rotateByte[0]= (((c[3]>>7) &0x1)<<0) + (((c[3]>>6) &0x1)<<1) + (((c[3]>>5) &0x1)<<2) + (((c[3]>>4) &0x1)<<3) + (((c[3]>>3) &0x1)<<4) + (((c[3]>>2) &0x1)<<5) + (((c[3]>>1) &0x1)<<6) + (((c[3]>>0) &0x1)<<7) ;
					rotateByte[1]= (((c[2]>>7) &0x1)<<0) + (((c[2]>>6) &0x1)<<1) + (((c[2]>>5) &0x1)<<2) + (((c[2]>>4) &0x1)<<3) + (((c[2]>>3) &0x1)<<4) + (((c[2]>>2) &0x1)<<5) + (((c[2]>>1) &0x1)<<6) + (((c[2]>>0) &0x1)<<7) ;
					rotateByte[2]= (((c[1]>>7) &0x1)<<0) + (((c[1]>>6) &0x1)<<1) + (((c[1]>>5) &0x1)<<2) + (((c[1]>>4) &0x1)<<3) + (((c[1]>>3) &0x1)<<4) + (((c[1]>>2) &0x1)<<5) + (((c[1]>>1) &0x1)<<6) + (((c[1]>>0) &0x1)<<7) ;
					rotateByte[3]= (((c[0]>>7) &0x1)<<0) + (((c[0]>>6) &0x1)<<1) + (((c[0]>>5) &0x1)<<2) + (((c[0]>>4) &0x1)<<3) + (((c[0]>>3) &0x1)<<4) + (((c[0]>>2) &0x1)<<5) + (((c[0]>>1) &0x1)<<6) + (((c[0]>>0) &0x1)<<7) ;

					int test=(c[0]>>7) &0x1 ;
					test=c[0]>>0 ;
*/
					for (i=0;i<4;i++)
					{
						// Oberes und unteres Nibble vertauschen
						rotateByte[i]=((c[i]>>4) & 0xf) + ((c[i] & 0xf) << 4) ;

						// Nibbles nicht tauschen
						//
						//rotateByte[i]=c[i] & 0xff ;

						sprintf(tempString,"%02x",(unsigned char) rotateByte[i]) ;
						strcat (txt->string,tempString) ;
					} ;
				}
				else
				{
					strcat(txt->string, c);
				} ;

				rxptr = rxbuf;
				fms.txtnr -= (4 >> 1);
				// fms.txtnr -= 4 ;
				rxstate = 49;

				if (vals.txtinc)
				{
					if (rxbuf[32] || (fms.txtnr<=0))
					{
						/*	Status b: Abschluss (bisher Status a,
						 * unterscheidet sich in Bit 32)
						 * txtnr: 0
						 * idealerweise trifft beides zu	*/

						// txt_change(txt->string, s->l1.fms.subst);
						// txt_break(txt, MAXROW);

						txt = txtbuf;
						i = 0;
						std::string outString ;

						while (txt != NULL)
						{
							outString +=txt->string ;
							txt = txt->next;
						}

						if (!test)
						{
							DisplayResult(outString.c_str()) ;
							StoreForDupeCheck() ; // F?r sp?ter sichen, um zu pr?fen, ob was doppelt ist
						}
						vals.txtinc = 0;
						rxstate = 0;
						fms.stat[0]=0xb ;
						returnValue=true ;
					}
				}
			}
		}
	}
	return returnValue ;
}

int MonitorModuleFMS::decode(bool test)
{
	int i = 0, rules_;
	short bit = 0;

	if (error_correction())
	{
		return 0 ;
	}


	// if (crc_check(0)) return 0;

	/*	falls keine laufende Text?bertragung	*/
	if (fms.stat[0] != 0xa) {

		fms.bos[0]	= rxbuf[3] << 3 | rxbuf[2] << 2 | rxbuf[1] << 1 | rxbuf[0];

		fms.land[0]	= rxbuf[7] << 3 | rxbuf[6] << 2 | rxbuf[5] << 1 | rxbuf[4];

		fms.ort[0]  = rxbuf[11] << 3 | rxbuf[10] << 2 | rxbuf[9] << 1 | rxbuf[8];
		fms.ort[1]  = rxbuf[15] << 3 | rxbuf[14] << 2 | rxbuf[13] << 1 | rxbuf[12];

		fms.kfz[0] = rxbuf[19] << 3 | rxbuf[18] << 2 | rxbuf[17] << 1 | rxbuf[16];
		fms.kfz[1] = rxbuf[23] << 3 | rxbuf[22] << 2 | rxbuf[21] << 1 | rxbuf[20];
		fms.kfz[2] = rxbuf[27] << 3 | rxbuf[26] << 2 | rxbuf[25] << 1 | rxbuf[24];
		fms.kfz[3] = rxbuf[31] << 3 | rxbuf[30] << 2 | rxbuf[29] << 1 | rxbuf[28];

		fms.stat[0]	= rxbuf[35] << 3 | rxbuf[34] << 2 | rxbuf[33] << 1 | rxbuf[32];

		fms.bst[0]	= rxbuf[36];
		fms.dir[0]	= rxbuf[37];
		fms.tki[0]	= rxbuf[38] << 1 | rxbuf[39];

		for (i = 0; i < 7; i++) fms.crc[i] = rxbuf[40 + i];

		rules_ = rules();

		if (!(rules_ )) {
			bit = vals.bit;
			if (bit < 49 && bit) rxbuf[bit - 1] ^= 1;
			return 0;
		}
	}
		/*	noch keine Ausgabe falls Text?bertragung (sp?ter mit Zeichennzahl))	*/
	if (fms.stat[0] == 0xa) return 2;

	bit = vals.bit;

	// if(!rxbuf[47]) 	/*	Stoppbit == 0	*/
	{
		if (!test)
		{
			DisplayResult() ; /* Auf dem Bildschirm ausgeben */
			StoreForDupeCheck() ; // F?r sp?ter sichen, um zu pr?fen, ob was doppelt ist
		}

	}
	if (bit < 49 && bit) rxbuf[bit - 1] ^= 1;
	return !rxbuf[47] ;
}



int MonitorModuleFMS::crc_check(int offset)
{
/*****************************************************************************
 *	CRC-Pr?froutine und teilweise blindes Korrigieren, offset=[0, 48]
 *****************************************************************************/

	#define	CODE	47

	unsigned char	g[] = {1,0,0,0,1,0,1};
	unsigned char	r[] = {0,0,0,0,0,0,0,0};
	char *p;
	unsigned int	i = 0, j;

	// rxbuf,fms.vals.quality,0, &vals.bit


	/*	CRC-Pr?froutine Begin	*/
	for (i = 0; i < CODE; i++) {
		vals.bit = rxbuf[i + offset] ^ r[0];
		p = (char*) r;
		for (j = 0; j < 7; j++) *p++ = (vals.bit & g[j]) ^ r[j + 1];
	}
	/*	CRC-Pr?froutine Ende	*/

	/*	Pr?fen des Rest-Polynoms	*/
	vals.bit = 0;
	for (i = 0; i < 7; i++) {
		vals.bit <<= 1;
		vals.bit |= r[i];
	}
	/*	ok, falls kein Rest (*bit=0)	*/

	// TRACE1("CRC-Check: %d\n",vals.bit) ;
	if (vals.quality >= 20) return vals.bit;	/*	crc_check on	*/
	else{
		if (vals.bit) {
			vals.bit = crc_table[vals.bit];
			if (vals.bit < 48) rxbuf[(vals.bit)++] ^= 1;
			else return vals.bit;
		}
	}
	return 0;
}

int MonitorModuleFMS::rules()
{
	// ung?ltige Telegramme:
	// Ort : > 9* && > *9
	// BOS : 0 ;
	// SE: geändert auf Ort bis F* und *F, da in Bayern so gebräuchlich

	if (fms.ort[0] > 0xf) return 0 ;
	if (fms.ort[1] > 0xf) return 0 ;
	if (fms.bos==0) return 0 ;
	return 1 ;
}

void MonitorModuleFMS::DisplayResult(std::string input)
{
	//

	// TRACE2("Ergebnisse: Dupes: %d - Quittung: %d\n", (int) CheckForDupes(), (int) CheckForQuittung()) ;
	if (!CheckForDupes()  && !CheckForQuittung() )
	{
		char statusString[5] ; // war: traceString
		char kfzString[10] ; // war: traceString1
		char ortString[10] ; // war: traceString2
		char bosString[3] =" " ;
		char landString[3]=" " ;
		char richtungString[3]=" " ;
		char tkiString[3] = " " ;
		char baustufeString[3] = " " ;
		char dateStr[9];
		char timeStr[9];

		std::string fahrzeugKennung;
		std::string bosDezimalString ;
		std::string landDezimalString ;
		std::string statusDezimalString ;
		std::string jetzt ;

		sprintf(statusString,"%01x",fms.stat[0]) ;
		sprintf(kfzString, "%04x", fms.kfz[0]*0x1000+fms.kfz[1]*0x100+fms.kfz[2]*0x10+fms.kfz[3]*0x1) ;
		sprintf(ortString, "%02x", fms.ort[0]*0x10+fms.ort[1]*0x1) ;


      	// kfzString in Uppercase umwandeln (ist immer 4 stellig)
      	kfzString[0]=toupper( kfzString[0] );
      	kfzString[1]=toupper( kfzString[1] );
      	kfzString[2]=toupper( kfzString[2] );
      	kfzString[3]=toupper( kfzString[3] );

      	// ortString
      	ortString[0]=toupper( ortString[0] );
      	ortString[1]=toupper( ortString[1] );

      	// das gleiche für den StatusString
   		statusString[0]=toupper(statusString[0]) ;

		bosDezimalString=convertIntToString(fms.bos[0]) ;
		landDezimalString=convertIntToString(fms.land[0]) ;
		statusDezimalString=convertIntToString(fms.stat[0]) ;

		richtungString[0] = '0'+fms.dir[0] ;
		tkiString[0] = '0'+fms.tki[0] ;
		baustufeString[0]= '0' + fms.bst[0] ;

		bosString[0]= fms.bos[0]<=9 ? '0'+fms.bos[0] : ('A'+fms.bos[0]-10) ;
		landString[0]= fms.land[0]<=9 ? '0'+fms.land[0] : ('A'+fms.land[0]-10) ;
		fahrzeugKennung=std::string(bosString)+landString+ortString+kfzString ;

		currentTime(jetzt) ; // aktuelle Uhrzeit holen
		struct tm* tm_time= localtime(&m_time) ;
		strftime(dateStr,9,"%d.%m.%y" ,tm_time) ;
		strftime(timeStr,9,"%H:%M:%S" ,tm_time) ;

		ModuleResultBase *pRes =new ModuleResultBase() ;

		pRes->set("timestamp",jetzt);
		pRes->set("uhrzeit",timeStr) ;
		pRes->set("datum",dateStr) ;
		pRes->set("servernamehex",m_serverNameHex);
		pRes->set("channelnamehex",m_channelNameHex);
		pRes->set("channelnum",convertIntToString(m_iChannelNum));

		pRes->set("typ","fms");
		pRes->set("fmskennung",fahrzeugKennung);
		pRes->set("status",statusString);
		pRes->set("baustufe",baustufeString);
		pRes->set("timestamp",jetzt);
		pRes->set("richtung",richtungString);
		pRes->set("tki",tkiString);
		pRes->set("bosdezimal",bosDezimalString) ;
		pRes->set("landdezimal",landDezimalString) ;
		pRes->set("statusdezimal",statusDezimalString) ;
		pRes->set("bos",bosString) ;
		pRes->set("land",landString) ;
		pRes->set("ort",ortString) ;
		pRes->set("kfz",kfzString) ;
		pRes->set("textuebertragung",input) ;

		FILE_LOG(logDEBUG) << (*pRes) << "-----" ;
		GlobalDispatcher->addResult(pRes) ;
	} ;
}

void MonitorModuleFMS::fms_txt(short offset, char *c)
{
	int j, letters = 4, tlen;
	char	*str = c, *bpp;
	const char * tstr;

	bpp = (char*) &rxbuf[offset + 6];

	for (; letters; letters--) {
		*str = 0x20 ;
		for (j = 7; j; j--) {
			*str <<= 1;
			*str |= *bpp--;
		}

		if (m_bTranslate) tstr=translate_alpha(*str) ;
		else tstr=(const char *)NULL ;

		if (tstr != NULL) {
			/*	Steuerzeichen ausgeben?	*/
			if (vals.ctrl
			/*	Umlaute	*/
			|| ((*str & 0x58) == 0x58 && (*str & 0x7) >= 3 && (*str && 0x7) <= 5)
			/* ?	*/
			|| *str == 0x7e) {
				tlen= strlen(tstr);
				memcpy(str, tstr, tlen);
				str += tlen - 1;
			}
			else
			{
				/*	Enter	*/
				if (*str == 13 || *str == 10) {
					*str = '\n';
				} else {
					str--;
				}
			}

		} ;
		str++;
		bpp += 15;
	} ;
	*str = 0;
	/*	fms_txt*/
}


void MonitorModuleFMS::fms_bin(short offset, char *c)
{
	int j, letters = 4 ;
	char	*str = c, *bpp ;

	bpp = (char*) &rxbuf[offset + 7];

	for (; letters; letters--) {
		*str = 0x00 ;
		for (j = 8; j; j--) {
			*str <<= 1;
			*str |= *bpp--;
		}


		str++;
		bpp += 16; // ?????????
	} ;
	*str = 0;
	/*	fms_txt*/
}

void MonitorModuleFMS::StoreForDupeCheck()
{
	if (!CheckForDupes()  && !CheckForQuittung() )
	{
		memcpy (&m_LastMessage,&fms,sizeof(FMS)) ;
		// TRACE1("Gespeichert als letzte Nachricht %d\n",fms.bos[0]) ;
	}
	StoreForReverseCheck() ;
}

bool MonitorModuleFMS::CheckForDupes()
{
	if (	(m_LastMessage.bos[0]==fms.bos[0]) &&
			(m_LastMessage.bst[0]==fms.bst[0]) &&
			(m_LastMessage.tki[0]==fms.tki[0]) &&
			(m_LastMessage.dir[0]==fms.dir[0]) &&
			(m_LastMessage.kfz[0]==fms.kfz[0]) &&
			(m_LastMessage.kfz[1]==fms.kfz[1]) &&
			(m_LastMessage.kfz[2]==fms.kfz[2]) &&
			(m_LastMessage.kfz[3]==fms.kfz[3]) &&
			(m_LastMessage.land[0]==fms.land[0]) &&
			(m_LastMessage.ort[0]==fms.ort[0]) &&
			(m_LastMessage.ort[1]==fms.ort[1]) &&
			(m_LastMessage.stat[0]==fms.stat[0])
		)
	{
		return false ; /* Doppelte nicht verwerfen */
	}

	return false ;
}

bool MonitorModuleFMS::CheckForQuittung()
{
	if (	(m_LastMessage.bos[0]==fms.bos[0]) &&
			(m_LastMessage.bst[0]==fms.bst[0]) &&
			(m_LastMessage.dir[0]==(1-fms.dir[0])) &&
			(m_LastMessage.kfz[0]==fms.kfz[0]) &&
			(m_LastMessage.kfz[1]==fms.kfz[1]) &&
			(m_LastMessage.kfz[2]==fms.kfz[2]) &&
			(m_LastMessage.kfz[3]==fms.kfz[3]) &&
			(m_LastMessage.land[0]==fms.land[0]) &&
			(m_LastMessage.ort[0]==fms.ort[0]) &&
			(m_LastMessage.ort[1]==fms.ort[1]) &&
			(fms.stat[0]==15)
		)
	{
		/*if (m_bSuppressQuittung)
		{
			//TRACE0 ("Quittung raus raus !\n") ;
		} ;
		return m_bSuppressQuittung ;*/
	}

	return false ;
}


// *********************************************************
// ** IIR Filter
// *********************************************************

inline float MonitorModuleFMS::biq_lp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x+buf[0]+buf[0]+buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

inline float MonitorModuleFMS::biq_bp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x-buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

inline float MonitorModuleFMS::biq_hp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x-buf[0]-buf[0]+buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

void MonitorModuleFMS::gen_coef(int tipo, float f0, float Q, float *pcoef)
{
	float w0,a,d;

	w0=2.*M_PI*f0;

	//Prewharping
	w0=2.0*FREQ_SAMP*tan(w0/FREQ_SAMP/2.0);

	a=FREQ_SAMP/w0;
	d=4.*a*a +2.*a/Q +1.;

	switch(tipo){
	case 0:	(*pcoef++)=1.0/d;	break;
	case 1: (*pcoef++)=2.*a/Q/d;	break;
	case 2: (*pcoef++)=4.*a*a/d;	break;
	}

	(*pcoef++)=(8.*a*a -2.)/d;
	(*pcoef++)=-(4.*a*a -2.*a/Q +1.)/d;
}

void MonitorModuleFMS::set_filters(float f0, float f1, float dr)
{
	gen_coef(1,f0,f0/dr/2.0,bp0_c);	/* Space filter */
	gen_coef(1,f1,f1/dr/2.0,bp1_c);	/* Mark filter  */
	gen_coef(0,dr,0.5412,lp1_c);	/* Low-Pass order-4 Butt. filter */
	gen_coef(0,dr,1.3066,lp2_c);

	for (int i=0;i<4;i++) lp1_b[i]=lp2_b[i]=bp0_b[i]=bp1_b[i]=0.0;
}

void MonitorModuleFMS::demod_se(float *buffer, int length)
{
	float xs,xm,y, sample;

	/* This function implements a digital PLL for data recovery */
	static int x0;
	static int x_se ;

	static int x,f=0;


	#ifdef FMSDEBUG
		// Startwerte
		fwrite(buffer, sizeof(float), length,debugFile1);
		pBuffer1=fileBuffer1 ;
		pBuffer2=fileBuffer2 ;
		pBuffer3=fileBuffer3 ;
		bufferLen1=0 ;
		bufferLen2=0 ;
		bufferLen3=0 ;
	#endif


	for (; length >= SUBSAMP; length -= SUBSAMP, buffer += SUBSAMP)
	{
		// rtty (2) decoder
		//
		sample=(*buffer)*128 ;

		xs=biq_bp(sample,bp0_c,bp0_b);
		xm=biq_bp(sample,bp1_c,bp1_b);
		xs*=xs;		// xs RMS
		xm*=xm;		// xm RMS
		y=biq_lp(biq_lp(xm-xs,lp1_c,lp1_b),lp2_c,lp2_b);


		x=(y>0.0)?1:0 ;

		if ((x^x0)) 	// Data Change
		{
			if (!f)
			{
				if (bit_phase>0x8000)
				{ // Late
					bit_phase+=SPHASEINC_BASE*1.5; // war: /8
				} else 			// Early
				{
					bit_phase-=SPHASEINC_BASE*1.5 ;// war: /8
				}

				f=1;
			}
		}

		x0=x;

		x_se=bit_phase;

		bit_phase+=SPHASEINC_BASE ;


		#ifdef FMSDEBUG
			/*
			debugFile1 = fopen("fms_in.raw", "wb");
			debugFile2 = fopen("fms_takt.raw", "wb");
			debugFile3 = fopen("fms_pfd.raw", "wb");
			debugFile4 = fopen("fms_4.raw", "wb");

			pBuffer1=debugFile2 ;
			pBuffer2=debugFile3
			pBuffer3=debugFile4 ;
			*/

			*pBuffer1=(bit_phase <0x8000)?0.7:-0.7 ;
			*pBuffer2=y ;
			*pBuffer3=(Ud*0.75)/0x8000 ;

			pBuffer1++ ;
			pBuffer2++ ;
			pBuffer3++ ;
			bufferLen1++ ;
			bufferLen2++ ;
			bufferLen3++ ;
		#endif

			if (((bit_phase & 0x8000)>0) && ((x_se & 0x8000)==0)) // in der "Mitte" bit_phase=0x8000 wird ein einzelnes Bit "empfangen", wichtig wg. der early/late - nicht erst am Ende !
			{
				f=0 ;

				//if ( (!m_bForcePreambel) || rxstate  ) // rxstate bei lfd. Text?bertragung
				{
					#ifdef FMSDEBUG
					pBuffer3-- ;
					*pBuffer3= x0==1 ? 0.7:-0.7 ;
					pBuffer3++ ;
					#endif

					if (rxbit(x0<<7)) // Gibt true zur?ck, wenn ein korrektes Wort empfangen worden ist
					{
						// Daten korrekt empfagen
						ErrorOut() ;
					}
				}

				// This function returns in the middle of a bit
			} ;
		}
	#ifdef FMSDEBUG
		fwrite(fileBuffer1, sizeof(float), bufferLen1,debugFile2);
		fwrite(fileBuffer2, sizeof(float), bufferLen2,debugFile3);
		fwrite(fileBuffer3, sizeof(float), bufferLen3,debugFile4);
	#endif

}

void MonitorModuleFMS::ErrorOut()
{
	if (fms.stat[0] != 0xa) {

		fms.bos[0]	= rxbuf[3] << 3 | rxbuf[2] << 2 | rxbuf[1] << 1 | rxbuf[0];

		fms.land[0]	= rxbuf[7] << 3 | rxbuf[6] << 2 | rxbuf[5] << 1 | rxbuf[4];

		fms.ort[0]  = rxbuf[11] << 3 | rxbuf[10] << 2 | rxbuf[9] << 1 | rxbuf[8];
		fms.ort[1]  = rxbuf[15] << 3 | rxbuf[14] << 2 | rxbuf[13] << 1 | rxbuf[12];

		fms.kfz[0] = rxbuf[19] << 3 | rxbuf[18] << 2 | rxbuf[17] << 1 | rxbuf[16];
		fms.kfz[1] = rxbuf[23] << 3 | rxbuf[22] << 2 | rxbuf[21] << 1 | rxbuf[20];
		fms.kfz[2] = rxbuf[27] << 3 | rxbuf[26] << 2 | rxbuf[25] << 1 | rxbuf[24];
		fms.kfz[3] = rxbuf[31] << 3 | rxbuf[30] << 2 | rxbuf[29] << 1 | rxbuf[28];

		fms.stat[0]	= rxbuf[35] << 3 | rxbuf[34] << 2 | rxbuf[33] << 1 | rxbuf[32];

		fms.bst[0]	= rxbuf[36];
		fms.dir[0]	= rxbuf[37];
		fms.tki[0]	= rxbuf[38] << 1 | rxbuf[39];

		for (int i = 0; i < 7; i++) fms.crc[i] = rxbuf[40 + i];

	}

	char traceString [5] ;
	// itoa( fms.stat[0], traceString,16) ;
	sprintf(traceString,"%01x",fms.stat[0]) ;

	char traceString1[10] ;
	sprintf(traceString1, "%04x", fms.kfz[0]*0x1000+fms.kfz[1]*0x100+fms.kfz[2]*0x10+fms.kfz[3]*0x1) ;

	char traceString2[10] ;
	sprintf(traceString2, "%02x", fms.ort[0]*0x10+fms.ort[1]*0x1) ;

#ifdef MON_DEBUG_ERR
	TRACE0("KFZ: ") ;
	TRACE0(traceString1) ;
	TRACE0 (" ") ;

	TRACE0("Ort: ") ;
	TRACE0(traceString2) ;
	TRACE0(" ") ;

	TRACE0("Status: ") ;
	TRACE0(traceString) ;
	TRACE0("\n") ;
#endif
/*
	COleDateTime now ;
	now=COleDateTime::GetCurrentTime() ;

	CString outString=now.Format("%H:%M:%S - ") ;
	*/
	std::string outString ;
	outString+="received: " ;

	char tempString[10]=" " ;
	tempString[0]= fms.bos[0]<=9 ? '0'+fms.bos[0] : ('A'+fms.bos[0]-10) ;
	//	itoa(fms.bos[0], tempString,16) ; // BOS
	outString += tempString ;
	outString +=" " ;

	tempString[0]= fms.land[0]<=9 ? '0'+fms.land[0] : ('A'+fms.land[0]-10) ;
	//itoa(fms.land[0], tempString,16) ; // LAND
	outString += tempString ;

	outString += traceString2 ; // ORT
	outString +=" " ;
	outString += traceString1 ; // KFZ

	std::transform (outString.begin(), outString.end(), outString.begin(),
               (int(*)(int)) toupper);

	outString +=" : " ;
	outString += traceString ; // Status ;

	outString += " - " ;
	tempString[0]= fms.bst[0]<=9 ? '0'+fms.bst[0] : ('A'+fms.bst[0]-10) ;
	//	itoa(fms.bst[0], tempString,16) ; // BST
	outString += tempString ;

	outString += "/" ;
	if (fms.dir[0]==0)
		outString += "Kfz->Lst" ;
	else
		outString += "Lst->Kfz" ;

	// itoa(fms.dir[0], tempString,16) ; // DIR
	// outString += tempString ;

	outString += "/" ;
	tempString[0]= fms.tki[0]<=9 ? '0'+fms.tki[0] : ('A'+fms.tki[0]-10) ;
	//	itoa(fms.tki[0], tempString,16) ; // TKI
	outString += tempString ;


	////DebugMessage(outString) ;

	//TRACE1("%s\n",outString) ;
}

void MonitorModuleFMS::StoreForReverseCheck()
{

	memcpy (&m_ReverseCheckMessage,&fms,sizeof(FMS)) ;
			// TRACE1("Gespeichert als letzte Nachricht %d\n",fms.bos[0]) ;

}

bool MonitorModuleFMS::IsRelatedMessage()
{
	if (	(m_ReverseCheckMessage.bos[0]==fms.bos[0]) &&
			(m_ReverseCheckMessage.kfz[0]==fms.kfz[0]) &&
			(m_ReverseCheckMessage.kfz[1]==fms.kfz[1]) &&
			(m_ReverseCheckMessage.kfz[2]==fms.kfz[2]) &&
			(m_ReverseCheckMessage.kfz[3]==fms.kfz[3]) &&
			(m_ReverseCheckMessage.land[0]==fms.land[0]) &&
			(m_ReverseCheckMessage.ort[0]==fms.ort[0]) &&
			(m_ReverseCheckMessage.ort[1]==fms.ort[1])
		)
	{
		return true ;
	} else
	{
		return false ;
	}
}

bool MonitorModuleFMS::error_correction()
{
	#define	CODE	47
	// Gibt true zur?ck, wenn keine Fehlerkorrektur o.?. helfen konnte
	//
	if (crc_check(0))
	{
		//
		if (m_bPrintError && m_bErrorCorrection)
		{
		//	TRACE0("CRC Check failed...\n") ;
			// CRC-Check war nicht ok ...
			// Deswegen

			// Ein Bit Fehler versuchen zu korrigieren
			//
			// gr?ssere Hamming-Dist m?glich ? (48 Bit, davon 7 Bit CRC = 42 Bit Nutzdaten)
			//
			int pos1=0 ;
			do
			{
				rxbuf[pos1] = (1-rxbuf[pos1]) ; // Bit invertieren
				if (!crc_check(0)  )
				{
					if (RMExists())
					{
						//std::string outString ;
						//outString.Format("1) CRC corrected at %d.",pos1) ;
						////DebugMessage(outString) ;
						ErrorOut() ;

						return false ;
					}
				}
				rxbuf[pos1] = (1-rxbuf[pos1]) ; // Bit invertieren
				pos1++ ;
			} while (pos1<(CODE)) ;
		}
		return true ;
	} else
	{
		return false  ;
	}
}

bool MonitorModuleFMS::RMExists()
{
	return true ;
}

bool MonitorModuleFMS::IsSyncWord(unsigned long rxbitstream)
{
	int minbits= vals.quality >= 20 ? vals.quality - 20 : vals.quality;
	int i,bitcounter=0 ;

	unsigned int sync  = 0xfff1a ;

	for (i=0;i<20;i++)
	{
		unsigned int bit1=rxbitstream & 0x1 ;
		unsigned int bit2=sync & 0x1 ;
		if (bit1==bit2)
		{
			bitcounter++ ;
		}
		rxbitstream>>=1 ;
		sync>>=1 ;
	}

	return (bitcounter >= minbits) ;
}


void MonitorModuleFMS::demod_neu(float *buffer, int length)
{

}


bool MonitorModuleFMS::IsValid()
{
	return !(fms.bos[0]==0) ;
}
