// GeneratorFMS.h: Schnittstelle für die Klasse CGeneratorFMS.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERATORFMS_H__2A6424EB_1110_40D4_A377_D9AA3985E5DA__INCLUDED_)
#define AFX_GENERATORFMS_H__2A6424EB_1110_40D4_A377_D9AA3985E5DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Generator.h"

class CGeneratorFMS : public CGenerator
{
public:
	unsigned char EncodeChar(unsigned char curchr);
	void SetText(CString text);
	bool BuildPktArray(bool folgetelegramm=false);
	void SetBaustufe(int baustufe);
	void SetRichtung(int richtung);
	void SetTKI(int tki);
	void SetStatus(int status);
	void SetKodierung(CString kodierung);
	virtual int Generate(CBuffer* buffer);
	CGeneratorFMS();
	virtual ~CGeneratorFMS();

protected:
	unsigned char ConvertChar(char c);
	CString m_sText;
	int GenerateTelegramm(CBuffer *buffer, int offset, bool folgetelegramm=false);
	void crc_check(unsigned char *bp);
	int m_iRichtung ;
	int m_iBaustufe ;
	char m_sKodierung[10] ;
	int m_iStatus ;
	int m_iTKI ;

	char convert(char c);
	static unsigned char reverse(unsigned char curchr);
	// Verlaufsinformationen
	//
	int ch_idx;
	int ph_row, ph_col, ph, phinc;
	int time, time2;
	unsigned char data[512];
	int lastb;

	// Parameter
	//
	unsigned char pkt[256];
	int pktlen;
	int pause;
	float duration;
};

#endif // !defined(AFX_GENERATORFMS_H__2A6424EB_1110_40D4_A377_D9AA3985E5DA__INCLUDED_)
