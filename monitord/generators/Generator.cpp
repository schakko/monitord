// Generator.cpp: Implementierung der Klasse CGenerator.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "Generator.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGenerator::CGenerator()
{
	m_bValidSound=false ;
	m_iAmpl=16384 ;
	m_iSampleRate=22050 ;
}

CGenerator::~CGenerator()
{

}

int CGenerator::Generate(CBuffer* buffer)
{
	return 0 ;
}

bool CGenerator::IsValidSound()
{
	return m_bValidSound ;
}

void CGenerator::SetSampleRate(int rate)
{
	m_iSampleRate=rate ;
}

void CGenerator::SetAmplitude(int ampl)
{
	m_iAmpl=ampl ;
}

int CGenerator::GeneratePause(CBuffer *buffer, int offset, int ms_delay)
{
	int pause=MS(ms_delay) ;
	int num = 0, i;

	int buflen=(buffer->ByteLen-offset)/2 ;
	signed short *buf=(signed short *) buffer->ptr.s  ;
	buf+=offset/2 ;

	for (; ((buflen > 0) && (pause>0)) ; buflen-- , buf++, num++) {
		pause-- ;

		*buf=0 ;
	}
	return num*2 ;
}
