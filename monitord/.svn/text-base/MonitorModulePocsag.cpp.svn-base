// MyMonModulePocsag.cpp: Implementierung der Klasse MonitorModulePocsag.
//
//////////////////////////////////////////////////////////////////////


#include <iostream>
using namespace std;
#ifdef WIN32
	#include <vector>
	#include <algorithm>
	#include <functional>
#endif

#include "time.h"
#include "MonitorModulePocsag.h"
#include "convert.h"
#include "MonitorLogging.h"

#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif

#define LONG unsigned long

/*
 * the code used by POCSAG is a (n=31,k=21) BCH Code with dmin=5,
 * thus it could correct two bit errors in a 31-Bit codeword.
 * It is a systematic code.
 * The generator polynomial is:
 *   g(x) = x^10+x^9+x^8+x^6+x^5+x^3+1
 * The parity check polynomial is:
 *   h(x) = x^21+x^20+x^18+x^16+x^14+x^13+x^12+x^11+x^8+x^5+x^3+1
 * g(x) * h(x) = x^n+1
 */
#define BCH_POLY 03551 /* octal */
#define BCH_N    31
#define BCH_K    21

/*
 * some codewords with special POCSAG meaning
 */
#define POCSAG_SYNC     0x7CD215D8
#define POCSAG_SYNCINFO 0x7CF21436
#define POCSAG_IDLE     0x7a89c197

#define POCSAG_SYNC_WORDS ((2000000 >> 3) << 13)



//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MonitorModulePocsag::MonitorModulePocsag()
{
	maxVal=0.0 ;
	dcd_shreg=0 ;
	sphase=0 ;
	subsamp=0 ;
	lastbit=0 ;
	m_fTrigger=0.2 ;
	global_rx_data=0;
	// ctrl=0;
	m_bPreambel_detected=false ;
	m_bErrorCorrection=true ;
	m_iPreambelLen=0 ;
	m_bRXmode=false ;
	m_iRXErrorCount=0 ;
	MAX_RX_ERRORS=1500 ;
	PREAMBEL_MINLEN=300 ;
	m_iAlgorithmus=0 ;

	SetFilter("^1(.*)",1) ;
	for (int i=0; i<2;i++)
	{
		rx_buff[i].rx_sync=0 ;
		rx_buff[i].rx_word=0 ;
		rx_buff[i].rx_bit=0 ;
		rx_buff[i].numnibbles=0;
		rx_buff[i].func=0;
		rx_buff[i].adr=0;
		memset(rx_buff[i].buffer,0,sizeof(rx_buff[i].buffer));
		rx_buff[i].receiving=0;
	}
}

MonitorModulePocsag::~MonitorModulePocsag()
{

}

void MonitorModulePocsag::demod(float *buffer, int length)
{
	switch (m_iAlgorithmus)
	{
	case 1:
		demod_se(buffer, length) ; // war demod_se ...
		break ;
	case 0:
	default:
		demod_mg(buffer, length) ;
	} ;
}

void MonitorModulePocsag::demod_se(float *buffer, int length)
{

}

void MonitorModulePocsag::demod_mg(float *buffer, int length)
{

}

void MonitorModulePocsag::rxbit(int bit)
{
	global_rx_data <<= 1;
	global_rx_data |= ( bit==0 ? 0 : 1) ;
	//
	// Wozu dient das nachfolgende ? Alle Bits invertieren ?
	// Skyper Modus ???????
	do_one_bit(rx_buff, ~(global_rx_data));

	// Mit dieser Zeile alleine geht's zumindest bei mir ;-)
	//
	do_one_bit(rx_buff + 1, global_rx_data);
}

void MonitorModulePocsag::do_one_bit(struct rx *rx, unsigned long rx_data)
{
	unsigned char rxword;

	unsigned char dumpString[5] ;
	unsigned long data;
	unsigned char *bp;

	dumpString[0]=0x00 ;
	dumpString[1]=0x00 ;
	dumpString[2]=0x00 ;
	dumpString[3]=0x00 ;
	dumpString[4]=0x00 ;

	/* TODO: Hack fuer 64Bit Systeme. Da ist ein unsigned long 8 Byte gross. Die
	 * SYNC-Worte etc. sind aber nur 4 Byte gross. Da uns immer nur die letzten 4 Bytes
	 * interessieren werden vorderen 4 Bytes per Und-Verknüpfung auf Null gesetzt.
	 * Dann passen auch wieder die Vergleiche bei den Sync-Worten
	 *
	 * Ggf. muesste man das hier noch netter gestalten. Aber so geht's auf jeden Fall :)
	 * SE, 15.08.09
	 */
	rx_data &= 0xffffffff ;

	if ( ( (!rx->rx_sync)) ) // || (m_iRXErrorCount>MAX_RX_ERRORS) )
	{
		rx->rx_sync=0 ;
	} ;


	if (!rx->rx_sync) {
		if (isSync(rx_data)) {
			rx->rx_sync = 2 ; // war: 10 -- vielleicht auch 4/5/6 ?
			rx->rx_bit = rx->rx_word = 0;
			rx->func = -1;
			FILE_LOG(logDEBUG)<< "Sync gefunden\n" ;
			//TRACE0("Sync gefunden\n") ;
			m_iPreambelLen =0 ;
			m_bRXmode=true ;
			m_iRXErrorCount=0 ;
			return;
		}
		return;
	}

	if ((++(rx->rx_bit)) < 32) return;

	/*	one complete word received	*/
	rx->rx_bit = 0;

	unsigned long compareData =rx_data ;

	/*	check codeword	*/
	if (error_correction(rx_data)==false) {
		//TRACE2("CODEWORD ung?ltig (DATA=0x%4x = \"%s\")\n",rx_data,dumpString) ;
		/* codeword not valid	*/
		rx->rx_sync--;
		rx->rx_word++;	/*	Zeile als Programmkorrektur eingef?gt	*/
		m_iRXErrorCount++ ;
		/*Bad codeword	*/
		if (!(rx->func & (~3))) {
		/* message garbled	*/
			printmessage(rx) ;
			rx->buffer[0]= 0x00 ;
			rx->buffer[1]= 0x00 ;
		}
		rx->func = -1; /* invalidate message */
		return;
	}

	// Hier kommen wir nur hin, wenn das Codewort g?ltig ist
	//

	if (rx_data!=compareData)
	{
			//TRACE0("Error correction applied\n") ;
	};

	/* do something with the data */
	// printf("%s%s: Codeword: %08lx\n", s->dem_par->name, add_name, rx_data);
	rxword = rx->rx_word++;

	if (rxword >= 16) {
		/*	received word shoud be a frame synch	*/
		rx->rx_word = 0;
		if ((rx_data == POCSAG_SYNC) || (rx_data == POCSAG_SYNCINFO))
		{
			rx->rx_sync = 10;
			//TRACE0("erneutes Sync gefunden\n") ;
			// m_iPreambelLen =0 ;
			m_bRXmode=true ;
			m_iRXErrorCount=0 ;
		}
		else
			rx->rx_sync -= 2;
		return;
	}

	if (rx_data == POCSAG_IDLE) {
		// RotateString(dumpString, rx_data) ;

		//TRACE1("CODEWORD g?ltig (IDLE-WORD = %s) \n",dumpString) ;
		/*	it seems that we can output the message right here	*/
		if (!(rx->func & (~3)))
		{
			printmessage(rx);
			rx->buffer[0]= 0x00 ;
			rx->buffer[1]= 0x00 ;
		}
		rx->func = -1; /* invalidate message */
		return;
	}


	if ((rx_data & 0x80000000) ==0)
	{
		LONG testadr= ((rx_data >> 10) & 0x1ffff8) | ((rxword >> 1) & 7);

		if ((unsigned char)(testadr & 0x7) != (rx->rx_word>>1))
		{
			//TRACE0("Adresswort im falschen Frame -> Als Daten auswerten \n") ;
			//	rx_data |= 0x80000000 ;
		} ;
	} ;

	if (rx_data & 0x80000000 || rx_data == 0x7cd215d8) {
		//RotateString(dumpString, rx_data) ;
		// TRACE2("CODEWORD g?ltig (DATA=0x%4x = \"%s\") \n",rx_data,dumpString) ;
		/*	this is a data word	*/

		if (rx_data & 0x80000000) rx->receiving = 1;
		else rx->receiving = 0;

		if (rx->func & (~3)) {
			/* no message being received
			 * Lonesome data codeword	*/
			return;
		}
		if (rx->numnibbles > sizeof(rx->buffer) * 2 - 5) {
			/*	Warning: Message too long	*/
			printmessage(rx);
			rx->func = -1;
			rx->buffer[0]= 0x00 ;
			rx->buffer[1]= 0x00 ;
			return;
		}

		bp = rx->buffer + (rx->numnibbles >> 1);
		data = rx_data >> 11;
		if (rx->numnibbles & 1) {
			bp[0] = (bp[0] & 0xf0) | ((data >> 16) & 0xf);
			bp[1] = data >> 8;
			bp[2] = data;
		}
		else {
			bp[0] = data >> 12;
			bp[1] = data >> 4;
			bp[2] = data << 4;
		}
		rx->numnibbles += 5;
		return;
	}

	/*	process address codeword	*/
/*	if (rx_data >= POCSAG_SYNC_WORDS) {
		unsigned char func = (rx_data >> 11) & 3;
		unsigned long adr = ((rx_data >> 10) & 0x1ffff8) |
			((rxword >> 1) & 7);

		verbprintf(0, "%s%s: Nonstandard address codeword: %08lx "
			   "func %1u adr %08lx\n", s->dem_par->name, add_name, rx_data,
			   func, adr);
		return;
	}
*/

	if (!(rx->func & (~3)))
	{
		printmessage(rx);
		rx->buffer[0]= 0x00 ;
		rx->buffer[1]= 0x00 ;
	}

	/*rx->buffer[0]=0x00 ;
		rx->buffer[1]=0x00 ;
		*/


	//long tempAdr ; - Kann man da noch etwas retten ??? (Test,SE)
	rx->func = (rx_data >> 11) & 3;
	rx->adr = ((rx_data >> 10) & 0x1ffff8) | ((rxword >> 1) & 7);
	//tempAdr = ((rx_data >> 10) & 0x1ffff8) | ((rxword >> 1) & 7);
	rx->numnibbles = 0;
	/*
	if (tempAdr>8)
	{
		rx->adr=tempAdr ;

		bp=(unsigned char*) dumpString ;
		data=rx_data>> 11 ;

		if (rx->numnibbles & 1) {
			bp[0] = (bp[0] & 0xf0) | ((data >> 16) & 0xf);
			bp[1] = data >> 8;
			bp[2] = data;
		}
		else {
			bp[0] = data >> 12;
			bp[1] = data >> 4;
			bp[2] = data << 4;
		}

		//TRACE3("CODEWORD g?ltig (ADDRESS=%d ,Frame=%d, rxCount=%d) \n",rx->adr,rx->adr & 0x7,rx->rx_word>>1) ;
		if ((rx->adr & 0x7) != (rx->rx_word>>1))
			int doof=0 ;
			//TRACE0("Vermutlich ein Fehlerhaftes Adresswort !-------------------------------\n") ;
	} ;
	*/

}

void MonitorModulePocsag::printmessage(struct MonitorModulePocsag::rx *rx)
{
	char Funktionsbit [5]= " " ;

	if (rx->adr==0) return ; // Adresse == 0 macht keine Sinn ...

	Funktionsbit[0] = rx->func < 10 ? '0' + rx->func : 'A' + rx->func-10 ;
	//itoa( rx->func , Funktionsbit,16) ;

	char Adresse[10] ;
	sprintf(Adresse, "%07d", (int) rx->adr ) ;

	std::string outString ;
	std::string message="" ;

	// Fuer Crusader und FMS32Pro Modus
	char dateStr[9];
	char timeStr[9];
	//char subString[2];
	std::string subString ;

	std::string counterCString ;

	if (!isnumeric(rx, message))
	{
		RotateString (message,rx) ;
	} ;

	std::string jetzt ;

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

	pRes->set("typ","pocsag");
	pRes->set("subhex",subString) ;
	pRes->set("sub",std::string(Funktionsbit)) ;
	pRes->set("ric",Adresse) ;
	pRes->set("text",message) ;

	FILE_LOG(logDEBUG) << "Debug(POCSAG):" << endl << (*pRes) ;


	GlobalDispatcher->addResult(pRes) ;

	// StoreResult(rx) ;
}

unsigned int MonitorModulePocsag::syndrome(unsigned long data)
{
	unsigned long shreg = data >> 1; /* throw away parity bit */
	unsigned long mask = 1L << (BCH_N-1), coeff = BCH_POLY << (BCH_K-1);
	int n = BCH_K;

	for(; n > 0; mask >>= 1, coeff >>= 1, n--)
		if (shreg & mask)
			shreg ^= coeff;
	if (even_parity(data))
		shreg |= (1 << (BCH_N - BCH_K));
//	printf("BCH syndrome: data: %08lx syn: %08lx\n", data, shreg);
	return shreg;
}

unsigned char MonitorModulePocsag::even_parity(unsigned long data)
{
	unsigned int temp = data ^ (data >> 16);

	temp = temp ^ (temp >> 8);
	temp = temp ^ (temp >> 4);
	temp = temp ^ (temp >> 2);
	temp = temp ^ (temp >> 1);
	return temp & 1;
}

bool MonitorModulePocsag::error_correction(unsigned long & rx_data)
{
	if (!syndrome(rx_data)) return true ;
	if (!m_bErrorCorrection)  return false ;

	if (rx_data==0) return false ;

	int bit1=0,bit2=0 ;
	unsigned long mask1=1 , mask2 ;

	bit2=0 ;
	mask2=1 ;

	// Im ersten Lauf alle Bit's einmal invertieren
	//
	do
	{
		rx_data ^= mask2 ;
		if (!syndrome(rx_data)) return true ;
		bit2++ ;
		rx_data ^= mask2 ;
		mask2 <<=1 ;
	} while (bit2<31) ;

	// Offensichtlich bringt das invertieren eines Bits nichts, also versuchen 2 Bits
	// zu invertieren
	//
	do
	{	// Das Bit laut Z?hler bit1 invertieren
		//
		rx_data ^= mask1 ;
		if (!syndrome(rx_data)) return true ;

		bit2=bit1 ;
		mask2= 1 ;
		mask2 <<= bit2 ;

		do
		{	// Als zweites das Bit bit2 invertieren
			//
			rx_data ^= mask2 ;
			if (!syndrome(rx_data)) return true ;
			bit2++ ;
			rx_data ^= mask2 ;
			mask2 <<=1 ;
		} while (bit2<32) ;

		bit1 ++ ;
		rx_data ^= mask1 ;
		mask1 <<=1 ;
	} while	 (bit1 < 32) ;

	return false ;
}

void MonitorModulePocsag::RotateString(std::string & buffer, struct rx *rx)
{
	unsigned long data = 0;
	int datalen = 0;
	unsigned char *bp = rx->buffer;
	int len = rx->numnibbles;


	unsigned char curchr;
	const char *tstr;
	bool ctrl=true ;

	while (len > 0) {
		while (datalen < 7 && len > 0) {
			if (len == 1) {
				data = (data << 4) | ((*bp >> 4) & 0xf);
				datalen += 4;
				len = 0;
			} else {
				data = (data << 8) | *bp++;
				datalen += 8;
				len -= 2;
			}
		}
		if (datalen < 7)
			continue;
		datalen -= 7;
		/*	7-bit-Code, ein Shift left und revers:	*/
		curchr = ((data >> datalen) & 0x7f) << 1;
		curchr = ((curchr & 0xf0) >> 4) | ((curchr & 0x0f) << 4);
		curchr = ((curchr & 0xcc) >> 2) | ((curchr & 0x33) << 2);
		curchr = ((curchr & 0xaa) >> 1) | ((curchr & 0x55) << 1);
		tstr = translate_alpha(curchr);
		if (tstr) {
			/*	Steuerzeichen ausgeben?	*/
			if (ctrl
			|| ((curchr & 0x58) == 0x58 && (curchr & 0x7) >= 3 && (curchr & 0x7) <= 5)
			|| curchr == 0x7e) {
				buffer.append(tstr) ;
			}
		}
		else {
			buffer.append (1,curchr) ;
		}
	}
}

void MonitorModulePocsag::StoreResult(struct rx *rx)
{


}

bool MonitorModulePocsag::isnumeric(rx *rx, std::string & message)
{
	// Keine Numeric-Kontrolle mehr ;-)
	return false ;

}


void MonitorModulePocsag::SetTrigger(float trigger)
{
	m_fTrigger=trigger ;
}

bool MonitorModulePocsag::isSync(unsigned long rxdata)
{
	int counter_SYNC=0, counter_SYNCINFO=0 ;
	/* ToDo: unsigned long / signed long sind auf 64Bit System (8 !) Byte gross statt 4 !
	 * Deswegen vermutlich Fehler in der Sync-Detection !
	 */
	unsigned long VergleichSYNC=POCSAG_SYNC ;
	unsigned long VergleichSYNCINFO=POCSAG_SYNCINFO ;


	if (!m_bErrorCorrection)
		return (rxdata == POCSAG_SYNC || rxdata == POCSAG_SYNCINFO ) ;

	// Aha - Fehlerkorrektur gew?nscht
	//

	for (int i=0; i < 32 ; i++)
	{
		if ( (rxdata & 0x1) == (VergleichSYNC & 0x1) )
		{
			counter_SYNC++ ;
		} ;

		if ( (rxdata & 0x1) == (VergleichSYNCINFO & 0x1) )
		{
			counter_SYNCINFO++ ;
		} ;

		VergleichSYNC >>= 1 ;
		VergleichSYNCINFO >>= 1 ;
		rxdata >>=1 ;
	}
	return ((counter_SYNC >30) || (counter_SYNCINFO>30)) ;
}
