// GeneratorZVEI.cpp: Implementierung der Klasse CGeneratorZVEI.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GeneratorZVEI.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define PHINC(x)   ((float)(x)*0x10000/SAMPLE_RATE)

#pragma warning( disable : 4244 )

static const unsigned int zvei_freq[16] = {
	PHINC(2400), PHINC(1060), PHINC(1160), PHINC(1270),
	PHINC(1400), PHINC(1530), PHINC(1670), PHINC(1830),
	PHINC(2000), PHINC(2200), PHINC(2800), PHINC(810),
	PHINC(970), PHINC(886), PHINC(2600), PHINC(0)
};

#pragma warning( default : 4244 )
//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGeneratorZVEI::CGeneratorZVEI()
{

}

CGeneratorZVEI::~CGeneratorZVEI()
{

}

bool CGeneratorZVEI::SetZVEI(CString zveiCode)
{
	m_sInputString=zveiCode ;
	return true ;
}

int CGeneratorZVEI::Generate(CBuffer* buffer)
{
	int offset ;
	int frequenz_weckton=0 ;
	bool bKanalbelegungston=false ;
	CString weckton ;

	// ist ein Weckton zu erzeugen ?
	// Wenn ja, welcher ?
	if (m_sInputString.GetLength()<5)
	{
		return 0 ;
	}

	CString zvei=m_sInputString.Left(5) ;

	if (m_sInputString.GetLength()>5)
	{
		weckton=m_sInputString.GetAt(5) ;

		switch (weckton.GetAt(0))
		{
		case 'B':
		case 'b' :
			frequenz_weckton=0 ;
			bKanalbelegungston=true ;
			break ;

		case 'F':
		case 'f' :
			frequenz_weckton=1240 ;
			break ;
		case 'P':
		case 'p' :
			frequenz_weckton=1860 ;
			break ;
		case 'W':
		case 'w' :
			frequenz_weckton=2280 ;
			break ;
		case 'Z':
		case 'z' :
			frequenz_weckton=825 ;
			break ;
		case 'E':
		case 'e' :
			frequenz_weckton=1010 ;
			break ;
		default:
			frequenz_weckton=0 ;
			m_sInputString+="X" ;
			break ;

		}
	} else
	{
		frequenz_weckton=0 ;
		m_sInputString+="X" ;
	}

	offset=GeneratePause(buffer,0,600) ;
	offset+=GenerateSequence(buffer,offset) ;
	offset+=GeneratePause(buffer,offset,600) ;
	offset+=GenerateSequence(buffer,offset) ;
	offset+=GeneratePause(buffer,offset,600) ;

	if (frequenz_weckton>0)
	{
		offset+=GenerateWeckton(buffer,offset,frequenz_weckton) ;
	}
	else
	{
		if (bKanalbelegungston)
		{
			offset+=GenerateBelegungston(buffer,offset) ;
			offset+=GenerateBelegungston(buffer,offset) ;
			offset+=GenerateBelegungston(buffer,offset) ;
			offset+=GenerateBelegungston(buffer,offset) ;
		}
	}
	offset+=GeneratePause(buffer,offset,70) ;

	return offset ;
}

int CGeneratorZVEI::GenerateSequence(CBuffer* buffer, int offset)
{
	// Standardwerte setzen
	ch_idx=0 ;
	ph=0 ;
	phinc=0 ;
	time=0 ;
	time2=0 ;
	duration=MS(70);
	pause=MS(1) ;

	char c;
	int num = 0, i;

	int buflen=(buffer->ByteLen-offset)/2 ;
	signed short *buf=(signed short *) buffer->ptr.s  ;
	buf += offset/2 ;

	for (; ((buflen > 0) && (ch_idx<m_sInputString.GetLength())) ; buflen-- , buf++, num++) {
		if (time <= 0) {
			c = m_sInputString.GetAt(ch_idx); // c ist das aktuelle Zeichen
			if (!c) // Wenn c==0, dann exit mit der aktuellen Anzahl von ?
				return num ;
			ch_idx++; // Nächstes Zeichen
			if (!isxdigit(c)) {
				time = time2 = 1;
				fprintf(stderr, "gen: zvei; invalid char '%c'\n", c);
			}
			else {
				time = duration + pause; //s->s.zvei.time ist die gesamtdauer (mit Pause)
				time2 = duration; // s->s.zvei.time2 ist die Dauer des Tones an sich
				if (c >= '0' && c <= '9')
					i = c - '0';
				else if (c >= 'A' && c <= 'F')
					i = c - 'A' + 10;
				else
					i = c - 'a' + 10;
				phinc = zvei_freq[i & 0xf]; // Die Schwingungslänge 2pi=1 Periode ist wie lange ?
			}                                         // 10000 "Einheiten". Wobei 10000 Einheiten ja abhängig von der Samplerate sind
													  // phinc ist ähnlich wie Omega (Kreisgeschwindigkeit)
		}
		else if (!time2) {
			phinc = 0;   // Phi-Increment
			ph = 0xc000; // Phi
		}
		time--;
		time2--;
		//*buf += (m_iAmpl * COS(ph)) >> 15; // Nun ja, es ist eigentlich dann wohl eine COS-Schwingung ;-)
		*buf=0 ;
		*buf += (m_iAmpl * COS(ph)) >> 15; // Nun ja, es ist eigentlich dann wohl eine COS-Schwingung ;-)
		ph += phinc;
	}
	return num*2 ;

}

//DEL int CGeneratorZVEI::GeneratePause(CBuffer *buffer, int offset, int ms_delay)
//DEL {
//DEL
//DEL }



int CGeneratorZVEI::GenerateWeckton(CBuffer *buffer, int offset, int freq)
{
	// Standardwerte setzen
	ch_idx=0 ;
	int ph_Basis=0 ;
	ph=0 ;
	phinc=0 ;
	int phinc_Basis ;
	time=0 ;
	time2=0 ;
	duration=MS(5000);
	pause=MS(1) ;

	char c;
	int num = 0, i;

	int buflen=(buffer->ByteLen-offset)/2 ;
	signed short *buf=(signed short *) buffer->ptr.s  ;
	buf += offset/2 ;

	time = duration + pause; //s->s.zvei.time ist die gesamtdauer (mit Pause)
	time2 = duration; // s->s.zvei.time2 ist die Dauer des Tones an sich

	phinc_Basis=PHINC(675) ;
	phinc = PHINC(freq) ; // Die Schwingungslänge 2pi=1 Periode ist wie lange ?

	for (; ((buflen > 0) && time>0) ; buflen-- , buf++, num++) {

		if (time2<=0) {
			phinc = 0;   // Phi-Increment
			phinc_Basis = 0;   // Phi-Increment
			ph_Basis=0xc000 ; // Phi Basiston
			ph = 0xc000; // Phi
		}
		time--;
		time2--;
		//*buf += (m_iAmpl * COS(ph)) >> 15; // Nun ja, es ist eigentlich dann wohl eine COS-Schwingung ;-)
		*buf=0 ;
		*buf += ((m_iAmpl * COS(ph_Basis)) + (m_iAmpl * COS(ph)) ) >> 16; // Nun ja, es ist eigentlich dann wohl eine COS-Schwingung ;-)

		ph += phinc;
		ph_Basis += phinc_Basis ;
	}
	return num*2 ;

}

int CGeneratorZVEI::GenerateBelegungston(CBuffer *buffer, int offset, int dauer, int pause)
{
	// Standardwerte setzen
	ph=0 ;
	phinc=0 ;
	time=0 ;
	time2=0 ;
	duration=MS(dauer);
	pause=MS(pause) ;

	char c;
	int num = 0, i;

	int buflen=(buffer->ByteLen-offset)/2 ;
	signed short *buf=(signed short *) buffer->ptr.s  ;
	buf += offset/2 ;

	time = duration + pause; //s->s.zvei.time ist die gesamtdauer (mit Pause)
	time2 = duration; // s->s.zvei.time2 ist die Dauer des Tones an sich

	phinc = PHINC(2600) ; // Die Schwingungslänge 2pi=1 Periode ist wie lange ?

	for (; ((buflen > 0) && time>0) ; buflen-- , buf++, num++) {

		if (time2<=0) {
			phinc = 0;   // Phi-Increment
			ph = 0xc000; // Phi
		}
		time--;
		time2--;
		*buf=0 ;
		*buf += (m_iAmpl * COS(ph)) >> 15; // Nun ja, es ist eigentlich dann wohl eine COS-Schwingung ;-)

		ph += phinc;

	}
	return num*2 ;



}
