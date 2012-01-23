// GeneratorZVEI.h: Schnittstelle für die Klasse CGeneratorZVEI.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERATORZVEI_H__8F64A74E_D08C_4183_953E_646740D6F8A0__INCLUDED_)
#define AFX_GENERATORZVEI_H__8F64A74E_D08C_4183_953E_646740D6F8A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Generator.h"

class CGeneratorZVEI : public CGenerator
{
public:
	int GenerateBelegungston(CBuffer *buffer, int offset, int dauer=800, int pause=300);
	int GenerateWeckton(CBuffer* buffer, int offset, int freq);
	int GenerateSequence(CBuffer* buffer, int offset=0);
	virtual int Generate(CBuffer* buffer);
	bool SetZVEI(CString zveicode);
	CGeneratorZVEI();
	virtual ~CGeneratorZVEI();

protected:
	int ph;
	int phinc;
	int pause;
	int duration;
	int time2;
	int time;
	int ch_idx;
	CString m_sInputString;
};

#endif // !defined(AFX_GENERATORZVEI_H__8F64A74E_D08C_4183_953E_646740D6F8A0__INCLUDED_)
