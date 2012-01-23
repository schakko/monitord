/* MyMonModulesPocsag1200.h
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
// MyMonModulePocsag1200.cpp: Implementierung der Klasse MonitorModulePocsag1200.
//
//////////////////////////////////////////////////////////////////////


#include "MonitorModulePocsag1200.h"

#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif

#define BAUD       1200
// es wird nur jedes x-te Sample ausgewertet (Auch PLLFaktor und Detektorbreite anpassen)
#define SUBSAMP    2
// max. Prozent-ï¿½nderungen Frq.-ï¿½nderung am PLL
// und Schmitt-Trigger
// und Breite der Rechtecke des Flankendetektors
//
#define PLLFaktor (5.0/100.0) 
#define SchmittTriggerLevel (0.05/1000.0)
#define DETEKTOR_BREITE 7


#define FILTLEN    1

#define GRENZWERT	0.3

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MonitorModulePocsag1200::MonitorModulePocsag1200(XMLNode *pConfig)
{
	int sampleRate=22050 ;
	bool crccheck=getNodeBool(*pConfig,"crc-check",true) ;
	bool errorcorrection=getNodeBool(*pConfig,"ecc",false) ;
	int minpreambel=getNodeInt(*pConfig,"minpreambel",0) ;
	int maxerrors=getNodeInt(*pConfig,"maxerrors",0) ;
	int algorithmus=getNodeInt(*pConfig,"algorithm",0) ;
	MonitorModulePocsag1200(	sampleRate,
								crccheck,
								errorcorrection,
								minpreambel,
								maxerrors,
								algorithmus);

}

MonitorModulePocsag1200::MonitorModulePocsag1200(int sampleRate, bool crccheck, bool errorcorrection, int minpreambel, int maxerrors, int algorithmus)
{
	m_bErrorCorrection=errorcorrection ;
	// m_bCRCCheck=crccheck ;
	MAX_RX_ERRORS=maxerrors ;
	PREAMBEL_MINLEN=minpreambel ;
	
	m_iAlgorithmus=algorithmus ;
	//m_iAlgorithmus=1;

	FREQ_SAMP=sampleRate ;


	SPHASEINC=(0x10000u * BAUD * SUBSAMP) / FREQ_SAMP ;
	SPHASEINC_BASE=(0x10000u * BAUD * SUBSAMP) / FREQ_SAMP ;
	m_lpszName="POC 1200" ;
	 
	m_fPLLFaktor=PLLFaktor ; // max. Prozent-ï¿½nderungen Frq.-ï¿½nderung am PLL
	m_fTrigger=SchmittTriggerLevel ;	
	// debugging
	 
	#ifdef POCDEBUG1200	 
		debugFile1 = fopen("_in.raw", "wb");
		debugFile2 = fopen("_sphase.raw", "wb");
		debugFile3 = fopen("_trigger.raw", "wb");
		debugFile4 = fopen("_rx.raw", "wb");
	#endif
}

MonitorModulePocsag1200::~MonitorModulePocsag1200()
{
	#ifdef POCDEBUG1200	 
		fclose(debugFile1) ;
		fclose(debugFile2) ;
		fclose(debugFile3) ;
		fclose(debugFile4) ;
	#endif
}

void MonitorModulePocsag1200::demod(float *buffer, int length)
{
	switch (m_iAlgorithmus)
	{
	case 0:
		demod_mg(buffer, length) ; // war demod_se ...
		break ;
	case 1:
		demod_se(buffer, length) ; // war demod_se ...
		break ;
	default:
		demod_mg(buffer, length) ;
	} ;
}


void MonitorModulePocsag1200::demod_se(float *buffer, int length)
{
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
	
	
	#ifdef POCDEBUG1200
		// Startwerte
		fwrite(buffer, sizeof(float), length,debugFile1);
		pBuffer1=fileBuffer1 ;
		pBuffer2=fileBuffer2 ;
		pBuffer3=fileBuffer3 ;
		bufferLen1=0 ;
		bufferLen2=0 ;
		bufferLen3=0 ;
	#endif
	

	for (; length >= SUBSAMP; length -= SUBSAMP, buffer += SUBSAMP) {
		dcd_shreg <<= 1;


		//m_LastFilterVal= m_weight * ((double)(*buffer)) + ( 1.0 - m_weight ) * m_LastFilterVal;
	
		//m_LastFilterVal= m_LastFilterVal + (m_weight * ( ((double)(*buffer)) - m_LastFilterVal)) ;
	
		//	m_LastFilterVal=*buffer ;
		
		
		// Mit einem Schmitt-Trigger die Flanken ansteilen und Rauschen
		// Eliminieren
		//
		if (lastbit==0)
		{
			if ((*buffer) > m_fTrigger)
			{
				lastbit=1;
			} ;
		} else {
			if ((*buffer) < -m_fTrigger)
			{
				lastbit=0;
			} ;
		} ;

		dcd_shreg |= (lastbit==1 ? 1 : 0) ;
		//dcd_shreg |= lastbit;

		for (int ming=40;ming>0;ming--)
		{
			m_lastVal[ming] = m_lastVal[ming-1] ;
		}
		
		m_lastVal[0] = lastbit ;
		
		if ((m_lastVal[0] + m_lastVal[DETEKTOR_BREITE]) == 1 )
		//if ( ((dcd_shreg >>9) &0x01) ^ (dcd_shreg & 0x01)) // Flankendetektor
		{
			Udiff=1	;
		}
		else
		{
			Udiff=0 ;
		}
		
		// Udiff = Eingang zum PD
		//
		// PD ist ein Exor ->
		// SPHASE 0...0x8000 = positive Halbwelle (1)
		// SPHASE 0x8000...0x10000 = negative Halbwelle (0)
		// 0x8 ist die Hälfte von 0x10 -> Hexadezimal !! :)

		// text darueber ist wertlos
		// 0x0000 ... 0xFFFF: Beschreibt ein Bit (eine HALBwelle)
		//--> 0x8000 ist der Punk im Rechteck genau zwischen zwei Flanken (zumindest bei der Präambel)
		
		
		if (sphase <= 0x8000) 
		{
			Uphase=1 ;
		} else
		{
			Uphase=0 ;
		}
		
		if (Udiff > lastUdiff)
		{
			// Flanke im Schmitt-Trigger (u1)
			//ah
			if (sphase >0x8000)
			{
				//sphase is too slow
				Ud= (0x10000-sphase) ;
				m_negativeCorrection = false;
			} else
			{
				//sphase is too fast
				Ud = sphase  ;
				m_negativeCorrection = true;
			}
		}
	
		if(m_negativeCorrection)
			SPHASEINC=(unsigned int) (( (double) SPHASEINC_BASE) * (1.0 - ( m_fPLLFaktor*Ud)/0x8000 )) ;
		else		
			SPHASEINC=(unsigned int) (( (double) SPHASEINC_BASE) * (1.0 + ( m_fPLLFaktor*Ud)/0x8000 )) ;
		
		//Ud=(Ud*99) /100 ;
		sphase += SPHASEINC ;

		lastUdiff = Udiff ;
		//lastUphase = Uphase ;
		
		#ifdef POCDEBUG1200
			//*pBuffer1=Uphase*0.75 ;
			//*pBuffer2=Udiff*0.75 ;
			//*pBuffer3=(Ud*0.75)/0x8000 ;
			//*pBuffer3=m_LastFilterVal ;
			//*pBuffer1 = (*buffer);
			*pBuffer1 = ((sphase) * 0.5) / 0x10000;
			*pBuffer2 = (lastbit==1 ? 0.5 : -0.5);
			//*pBuffer3 = 0.0f;
			*pBuffer3 = Ud / 0x8000;

			
			//if ((rx_buff+1)->rx_sync) // inSync
			//{
			//*pBuffer2=Udiff*0.45+0.5 ;	
			//}
			pBuffer1++ ;
			pBuffer2++ ;
			pBuffer3++ ;
			bufferLen1++ ;
			bufferLen2++ ;
			bufferLen3++ ;
		#endif
		
		
		if ((sphase>=0x8000u) && (didBit==false))
		{
			#ifdef POCDEBUG1200
				//*(pBuffer2-1)=-0.3 - 0.3*lastbit ;
				//*(pBuffer2-1)= -1.0;
			#endif
			rxbit( lastbit ) ; 
			didBit = true ;
		}
		
		if (sphase>=0x10000u)
		{
			// FILE_LOG(logINFO) << "sphase:" << sphase << " -> " << (sphase & (0xffff)) ;
			sphase = sphase & 0xffff ;
			didBit = false ;
		}
		

	}
	
	#ifdef POCDEBUG1200
		fwrite(fileBuffer1, sizeof(float), bufferLen1,debugFile2);
		fwrite(fileBuffer2, sizeof(float), bufferLen2,debugFile3);
		fwrite(fileBuffer3, sizeof(float), bufferLen3,debugFile4);
	#endif
	subsamp = length;
}




// ------------------ old version -------------------------

void MonitorModulePocsag1200::demod_mg(float *buffer, int length)
{
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
	for (; length >= SUBSAMP; length -= SUBSAMP, buffer += SUBSAMP) {
		dcd_shreg <<= 1;
		dcd_shreg |= ((*buffer) > 0);

		//	check if transition
		if ((dcd_shreg ^ (dcd_shreg >> 1)) & 1) {
			if (sphase < (0x8000u - (SPHASEINC / 2)))
				sphase += SPHASEINC / 8;
			else
				sphase -= SPHASEINC / 8;
		}
		sphase += SPHASEINC;

		if (sphase >= 0x10000u) {
			sphase &= 0xffffu;
			rxbit(dcd_shreg & 1);
		}
	}
	subsamp = length;
}
