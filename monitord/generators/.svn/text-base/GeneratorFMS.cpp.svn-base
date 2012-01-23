// GeneratorFMS.cpp: Implementierung der Klasse CGeneratorFMS.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GeneratorFMS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


#define PHINC(x)   ((float)(x)*0x10000/SAMPLE_RATE)

static const unsigned int fms_freq[2] = {
	PHINC(1800), PHINC(1200)
};


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGeneratorFMS::CGeneratorFMS()
{
	m_iAmpl = 16384;
	duration = MS((float)1000/1200);
	pause = MS(0);
	ch_idx=0 ;
	memset(pkt,0,256) ;
	memset(data,0,512) ;
	ph_col=0 ;
	ph_row=0 ;
	ph=0 ;
	phinc=0 ;
	pktlen=0 ;
	time=0 ;
	time2=0 ;

	pkt[0] = 0xf;
	pkt[1] = 0xf;
	pkt[2] = 0xf;
	pkt[3] = 0xf;
	pkt[4] = 0x1;
	pkt[5] = 0xa;
	strncpy((char*) (pkt + 6), "B97168615111",
				sizeof(pkt) - 6);
			pktlen = 6 + strlen((char*)(pkt + 6));
}

CGeneratorFMS::~CGeneratorFMS()
{

}

unsigned char CGeneratorFMS::reverse(unsigned char curchr)
{
/*****************************************************************************
 *	dreht die Reihenfolge von 4 bits (1010 -> 0101)
 *****************************************************************************/

	curchr = ((curchr & 0xc) >> 2) | ((curchr & 0x3) << 2);
	curchr = ((curchr & 0xa) >> 1) | ((curchr & 0x5) << 1);
	return curchr;
}

char CGeneratorFMS::convert(char c)
{

	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'A' && c <= 'F') return (c - 55);
	if (c >= 'a' && c <= 'f') return (c - 87);
	return (-1);
}

int CGeneratorFMS::Generate(CBuffer *buffer)
{
	CString SendungsString, temp ;
	int i ;

	int offset=0 ;
	offset+=GeneratePause(buffer,offset,100) ;
	offset+=GenerateTelegramm(buffer,offset) ;
	offset+=GeneratePause(buffer,offset,100) ;
	// offset+=GeneratePause(buffer,offset,20) ;
	// offset+=GenerateTelegramm(buffer,offset) ;

	// Jetzt als "Kodierung" die Daten einpflegen und als Folgetelegramm anfügen
	//
/*
	temp=m_sKodierung ;

	m_sKodierung[0]='6' ;
	m_sKodierung[1]='6' ;

	m_sKodierung[2]=EncodeChar(ConvertChar('Q') >>4)  ;
	m_sKodierung[3]=EncodeChar(ConvertChar('Q') & 0xf) ;

	m_sKodierung[4]=EncodeChar(ConvertChar('b') >>4)  ;
	m_sKodierung[5]=EncodeChar(ConvertChar('b') & 0xf) ;

	m_sKodierung[6]=EncodeChar(ConvertChar('c') >>4)  ;
	m_sKodierung[7]=EncodeChar(ConvertChar('c') & 0xf) ;


	offset+=GenerateTelegramm(buffer,offset,true) ;


	m_iStatus=11 ;

	m_sKodierung[2]=EncodeChar(ConvertChar('d') >>4)  ;
	m_sKodierung[3]=EncodeChar(ConvertChar('d') & 0xf) ;

	m_sKodierung[2]=EncodeChar(ConvertChar('e') >>4)  ;
	m_sKodierung[3]=EncodeChar(ConvertChar('e') & 0xf) ;

	m_sKodierung[4]=EncodeChar(ConvertChar('f') >>4)  ;
	m_sKodierung[5]=EncodeChar(ConvertChar('f') & 0xf) ;

	m_sKodierung[6]=EncodeChar(ConvertChar('g') >>4)  ;
	m_sKodierung[7]=EncodeChar(ConvertChar('g') & 0xf) ;


	offset+=GenerateTelegramm(buffer,offset,true) ;


	// offset+=GenerateTelegramm(buffer,offset,true) ;

*/
	return offset ;
}

void CGeneratorFMS::SetKodierung(CString kodierung)
{
	strcpy(m_sKodierung,kodierung) ;
}

void CGeneratorFMS::SetStatus(int status)
{
	m_iStatus=status ;
}

void CGeneratorFMS::SetTKI(int tki)
{
	m_iTKI=tki ;
}

void CGeneratorFMS::SetRichtung(int richtung)
{
	m_iRichtung=richtung ;
}

void CGeneratorFMS::SetBaustufe(int baustufe)
{
	m_iBaustufe=baustufe ;
}

void CGeneratorFMS::crc_check(unsigned char *bp)
{
/*****************************************************************************
 *	CRC-Prüfroutine und teilweise blindes Korrigieren, offset=[0, 48]
 *****************************************************************************/

#define	CODE	40

	unsigned char	g[] = {1,0,0,0,1,0,1};
	unsigned char	r[] = {0,0,0,0,0,0,0,0}, *rest;
	char *p;
	unsigned int	i = 0, j, bit;

	/*	CRC-Prüfroutine Begin	*/
	for (i = 0; i < CODE; i++) {
		bit = bp[i] ^ r[0];
		p = (char*) r;
		for (j = 0; j < 7; j++) *p++ = (bit & g[j]) ^ r[j + 1];
	}
	/*	CRC-Prüfroutine Ende	*/

	/*	Schreiben des Rest-Polynoms	*/
	rest = bp + 40;
	for (i = 0; i < 7; i++) {
		*rest++ = r[i] & 0x1;
	}
}

int CGeneratorFMS::GenerateTelegramm(CBuffer *buffer, int offset, bool folgetelegramm)
{
	#define	MOD	8
	#define START	6
	unsigned char c, tx[1000], *bp;
	int num = 0, i=0, j=0;
	int char_counter ;
	ch_idx=0 ;
	ph=0 ;

	ph_col=0 ;
	ph_row=0 ;
	ph=0 ;
	phinc=0 ;
	pktlen=0 ;
	time=0 ;
	time2=0 ;
	unsigned int temp ;

	BuildPktArray(folgetelegramm) ;

	char_counter=pktlen ;
	// TRACE1("Grösse: %d\n",strlen((char*) pkt)) ;

	memset(tx, 0, 1000 * sizeof(char));

	if (!folgetelegramm)
	{
		// Kodierung (Land/Ort/Fzg = 8 Nibbles)
		//
		/* Halbbytes drehen	*/
		for (i = START; i <= START + 8; i++)
			pkt[i]= reverse(convert(pkt[i]));

		/*	Zusatzinfos sammeln	*/
		for (i = START + 9; i <= START + 11; i++)
			pkt[i]= convert(pkt[i]);


		pkt[START + 9]=
			(pkt[START + 9] << 3) |
			(pkt[START + 10] << 2) |
			pkt[START + 11];
	}
	else
	{
		/* Halbbytes drehen	*/
		 for (i = START; i <= START + 8; i++)
		 	 pkt[i]=convert(pkt[i]);

		/*	Zusatzinfos sammeln	*/
		for (i = START + 9; i <= START + 11; i++)
			pkt[i]= convert(pkt[i]);

		pkt[START + 9]=
			(pkt[START + 9] << 3) |
			(pkt[START + 10] << 2) |
			pkt[START + 11];
	}


	bp = tx;
	for (i = START; i < START + 10; i++)
		for (j = 3; j >= 0; j--)
			*bp++ = (pkt[i] >> j) & 0x1;

	crc_check(tx);

	if (folgetelegramm!=true) // Folgetelegramme werden ohne Vorlauf direkt am Anschluß versende !
	{
		/*	Platz für Vorlauf schaffen	*/
		memmove(tx + 24, tx, 48);

		bp = tx;
		/*	Vorlauf eintragen	*/
		for (i = 0; i < START; i++)
			for (j = 3; j >= 0; j--)
				*bp++ = (pkt[i] >> j) & 0x1;
	}

	bp = tx;
	for (i = 0; i < 1000; i++) *bp++ += '0';

	if (folgetelegramm)
	{
		int hier_bleiben=0 ;
	}
	int buflen=(buffer->ByteLen-offset)/2 ;
	signed short *buf=(signed short *) buffer->ptr.s  ;
	buf += offset/2 ;

	for (; ((buflen > 0) && (char_counter > ch_idx)) ; buflen--, buf++, num++) {
		if (time <= 0) {
			c = tx[ch_idx];
			if (!c)
				return 2*num;
			ch_idx++;

			if (!isxdigit(c)) {
				time = time2 = 1;
				// fprintf(stderr, "gen: fms; invalid char nr %i '%c'\n", s->s.fms.ch_idx, c);
			}
			else {
			int dur		= (int)(duration * ch_idx),
				dur2 	= (int)(duration * ch_idx + .5);

				if (dur == dur2) dur = (int)duration;
				else dur = (int)duration + 1;
				time = dur + pause;
				time2 = dur;
				// phinc = fms_freq[c & 0x1];
				phinc = fms_freq[c=='0' ? 0 : 1];
			}
		}
		else if (!time2) {
			phinc = 0;
			ph = 0xc000;
		}
		time--;
		time2--;

		*buf=0 ;
		if (phinc>0)
		{
			temp=m_iAmpl * COS(ph) ;
			*buf += temp >> 15;
		}
		ph += phinc;
	}
	return 2*num;
}

bool CGeneratorFMS::BuildPktArray(bool folgetelegramm)
{
	int i ;

	pkt[0]=0x0 ;
	pkt[1]=0x7 ;
	pkt[2]=0xf ;
	pkt[3]=0xf ; // 12x 1 als Vorlauf
	pkt[4]=0x1 ; // a1 = 00011010 <=> SYNC
	pkt[5]=0xa ;

	{

		CString SendungsString ;

		SendungsString.Format("        %x%d%d%d",
			m_iStatus,
			m_iBaustufe,
			m_iRichtung ,
			m_iTKI
			) ;

		for (i=0;i<8;i++)
		{
			pkt[6+i]=m_sKodierung[i] ;
		}

		pktlen=49+6*4 ; // (Datenbits+Vorlauf+Sync)

		for (i=8; i < SendungsString.GetLength();i++)
		{
			pkt [6+i]=SendungsString.GetAt(i) ;
		}
	}

	if (folgetelegramm)
	{
		pktlen=48 ;
	}

	return true ;
}

void CGeneratorFMS::SetText(CString text)
{
	m_sText=text ;
}

unsigned char CGeneratorFMS::ConvertChar(char c)
{
	int paritaet=0 ;
	unsigned char c_out=0 ;

	c_out=c ;

	for (int i=0;i<7;i++)
	{
		paritaet=paritaet ^ (c & 0x1) ;

		/*
		c_out <<= 1 ;
		c_out |= c &0x1;
		*/
		c >>= 1;
	}

	c_out <<=1 ;
	c_out|= (paritaet) & 0x1 ;

	return c_out ;
}

unsigned char CGeneratorFMS::EncodeChar(unsigned char curchr)
{
	if (curchr<=9)
	{
		return curchr+'0' ;
	}
	else
	{
		return curchr+55 ;
	}
	return '0' ;
}
