// MyMonModulesPocsag512.cpp: Implementierung der Klasse CMyMonModulesPocsag512.
//
//////////////////////////////////////////////////////////////////////


#include "MonitorModulePocsag512.h"
 

#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif

#define BAUD       512
//#define POC512SUBSAMP    1
// es wird nur jedes x-te Sample ausgewertet (Auch PLLFaktor und Detektorbreite anpassen)
#define SUBSAMP    1
// max. Prozent-ï¿½nderungen Frq.-ï¿½nderung am PLL
// und Schmitt-Trigger
// und Breite der Rechtecke des Flankendetektors
//  1.0 ,0 ,7 - ganz gut
//  0.3 , 0.0001 , 7 - gut
//  0.1 , 0.0001 , 7 - ?
// 0.15 , 0.0001, 3 - schaut gut aus
#define PLLFaktor (0.25/100.0) 
#define SchmittTriggerLevel (0.0000)
#define DETEKTOR_BREITE 5
// war SUBSAMP = 5 


/* ---------------------------------------------------------------------- */
 

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MonitorModulePocsag512::MonitorModulePocsag512(int sampleRate,XMLNode *pConfig)
{
	bool crccheck=getNodeBool(*pConfig,"crc-check",true) ;
	bool errorcorrection=getNodeBool(*pConfig,"ecc",false) ;
	int minpreambel=getNodeInt(*pConfig,"minpreambel",300) ;
	int maxerrors=getNodeInt(*pConfig,"maxerrors",10) ;
	int algorithmus=getNodeInt(*pConfig,"algorithm",1) ;
	FILE_LOG(logINFO) << "(1) sample - crc - ecc - minpreambel - maxerrors - algo:"
				<< sampleRate << " - "
				<< crccheck << " - "
				<< errorcorrection << " - "
				<< minpreambel << " - "
				<< maxerrors << " - "
				<< algorithmus ;
	
	MonitorModulePocsag512(sampleRate,1,0) ;
	/*
	MonitorModulePocsag512(	sampleRate,
								crccheck == true ? 1:0 ,
								errorcorrection== true ? 1:0,								
								minpreambel,
								maxerrors,
								algorithmus);
	*/
}

MonitorModulePocsag512::MonitorModulePocsag512(int sampleRate, bool crccheck, bool errorcorrection, int minpreambel, int maxerrors, int algorithmus)
{
	FILE_LOG(logINFO) << "(2) sample - crc - ecc - minpreambel - maxerrors - algo:"
				<< sampleRate << " - "
				<< crccheck << " - "
				<< errorcorrection << " - "
				<< minpreambel << " - "
				<< maxerrors << " - "
				<< algorithmus ;
	m_bErrorCorrection=errorcorrection ;
	
	MAX_RX_ERRORS=maxerrors ;
	PREAMBEL_MINLEN=minpreambel ;

	m_iAlgorithmus=algorithmus ;
	FILE_LOG(logINFO) << "Algorithmus:" << m_iAlgorithmus ;

	FREQ_SAMP=sampleRate ;

	SPHASEINC=(0x10000u * BAUD * SUBSAMP) / FREQ_SAMP ;
	SPHASEINC_BASE=(0x10000u * BAUD * SUBSAMP) / FREQ_SAMP ;

	FILE_LOG(logINFO) << "SPHASE_INC is:" << SPHASEINC_BASE ;
	m_lpszName="POC 512" ;
 
	m_fPLLFaktor=PLLFaktor ; // max. Prozent-ï¿½nderungen Frq.-ï¿½nderung am PLL
	m_fTrigger=SchmittTriggerLevel ;	
	// debugging
	
	#ifdef POCDEBUG512	 
		debugFile1 = fopen("_in.raw", "wb");
		debugFile2 = fopen("_takt.raw", "wb");
		debugFile3 = fopen("_trigger.raw", "wb");
		debugFile4 = fopen("_pfd.raw", "wb");
	#endif

	set_filters(1200,1800,BAUD);	
}

MonitorModulePocsag512::~MonitorModulePocsag512()
{
	#ifdef POCDEBUG512	 
		fclose(debugFile1) ;
		fclose(debugFile2) ;
		fclose(debugFile3) ;
		fclose(debugFile4) ;
	#endif
}


void MonitorModulePocsag512::demod_se(float *buffer, int length)
{
	double y ;
	
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
	
	
	#ifdef POCDEBUG512
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
		
		y=biq_lp(biq_lp( (lastbit==1? 1.0:-1.0) ,lp1_c,lp1_b),lp2_c,lp2_b);

		//dcd_shreg |= (lastbit==1 ? 1 : 0) ;
		lastbit= (y>0 ? 1:0) ;
		dcd_shreg |= (lastbit==1 ? 1 : 0) ;

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
			if (sphase >0x8000)
			{
				Ud= (0x10000-sphase) ;
			} else
			{
				Ud = sphase  ;
			}
		}
 
		#ifdef POCDEBUG512
			*pBuffer1=Uphase*0.75 ;
			*pBuffer2=Udiff*0.75 ;
			*pBuffer3=(Ud*0.75)/0x8000 ;
			//*pBuffer3=m_LastFilterVal ;
			
			if ((rx_buff+1)->rx_sync) // inSync
			{
			*pBuffer2=Udiff*0.45+0.5 ;	
			}
			pBuffer1++ ;
			pBuffer2++ ;
			pBuffer3++ ;
			bufferLen1++ ;
			bufferLen2++ ;
			bufferLen3++ ;
		#endif
	
				
		SPHASEINC=(unsigned int) (( (double) SPHASEINC_BASE) * (1.0 + ( m_fPLLFaktor*Ud)/0x8000 )) ;
		sphase += SPHASEINC ;

		lastUdiff = Udiff ;
		
		if ((sphase>=0x8000u) && (didBit==false))
		{
			#ifdef POCDEBUG512
				*(pBuffer2-1)=-0.3 - 0.3*lastbit ;
			#endif
			rxbit( lastbit ) ;
			didBit = true ;
		}
		
		if (sphase>=0x10000u)
		{
			// FILE_LOG(logINFO) << "sphase:" << sphase << " -> " << (sphase & (0xffff)) ;
			sphase = sphase & 0xffffu ;
			didBit = false ;
		}
	}
	
	#ifdef POCDEBUG512
		
		fwrite(fileBuffer1, sizeof(float), bufferLen1,debugFile2);
		fwrite(fileBuffer2, sizeof(float), bufferLen2,debugFile3);
		fwrite(fileBuffer3, sizeof(float), bufferLen3,debugFile4);
	#endif
	subsamp = length;
}


// --------- old function -----------
void MonitorModulePocsag512::demod_mg(float *buffer, int length)
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


// *********************************************************
// ** IIR Filter
// *********************************************************

inline float MonitorModulePocsag512::biq_lp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x+buf[0]+buf[0]+buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

inline float MonitorModulePocsag512::biq_bp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x-buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

inline float MonitorModulePocsag512::biq_hp(float x, float *pcoef, float *buf)
{
	float y;
	y =(*pcoef++)*(x-buf[0]-buf[0]+buf[1]);
	y+=(*pcoef++)*buf[2];
	y+=(*pcoef)*buf[3];

	buf[1]=buf[0]; buf[0]=x;
	buf[3]=buf[2]; buf[2]=y;

	return y;
}

void MonitorModulePocsag512::gen_coef(int tipo, float f0, float Q, float *pcoef)
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

void MonitorModulePocsag512::set_filters(float f0, float f1, float dr)
{
	gen_coef(1,f0,f0/dr/2.0,bp0_c);	/* Space filter */
	gen_coef(1,f1,f1/dr/2.0,bp1_c);	/* Mark filter  */
	gen_coef(0,dr,0.5412,lp1_c);	/* Low-Pass order-4 Butt. filter */
	gen_coef(0,dr,1.3066,lp2_c);

	for (int i=0;i<4;i++) lp1_b[i]=lp2_b[i]=bp0_b[i]=bp1_b[i]=0.0;
}


