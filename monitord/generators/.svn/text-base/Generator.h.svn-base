// Generator.h: Schnittstelle für die Klasse CGenerator.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERATOR_H__E60BFF43_52FB_470A_875A_C1D63174C27A__INCLUDED_)
#define AFX_GENERATOR_H__E60BFF43_52FB_470A_875A_C1D63174C27A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Buffer.h"

#define SAMPLE_RATE 22050
#define MS(x) ((float)(x) * SAMPLE_RATE / 1000)

#define COSTABSIZE 0x400

extern int costabi[0x400];

#define COS(x) costabi[(((x)>>6)&0x3ffu)]

class CGenerator
{
public:
	int GeneratePause(CBuffer *buffer, int offset, int ms_delay);
	void SetAmplitude(int ampl);
	void SetSampleRate(int rate);
	bool IsValidSound();
	virtual int Generate(CBuffer* buffer);
	CGenerator();
	virtual ~CGenerator();

protected:
	int m_iSampleRate;
	bool m_bValidSound;
	int m_iAmpl;
};

#endif // !defined(AFX_GENERATOR_H__E60BFF43_52FB_470A_875A_C1D63174C27A__INCLUDED_)
