/*MonitorModules.cpp
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

/* ---------------------------------------------------------------------- */
#include <iostream>

#include "MonitorModules.h"
#include "convert.h"
#include "base64.h"


#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MonitorModule::MonitorModule()
{
	memset(rxbuf,0,512) ;
	rxstate=0 ;
	rxbitbuf=0 ;
	rxbitstream=0L ;
	FREQ_SAMP=22050 ;
	m_bTranslate=true ;
	m_iFilterModus=2 ;
	m_lpszFilter="" ;
	//m_pCriticalSection=NULL ;
	m_iFunkkanal=0 ;
	m_iChannelNum=0 ;

	if (memLockOpen( 12345, & m_queueLock) < 0) {
		// Erstmal tun wir hier nix ...
	}

    this->setChannelName("channel01" ) ;
    this->setServerName("monitord") ;
}

MonitorModule::~MonitorModule()
{

}


void MonitorModule::demod(float *buffer, int length)
{

}

void MonitorModule::SetName(std::string Name)
{
	m_lpszName=Name ;
}

void MonitorModule::SetSampleRate(int rate)
{

}

void MonitorModule::getCurrentTime()
{
	time(&m_time);
}

void MonitorModule::setChannelName(std::string name)
{
	convertStringToHex(name,m_channelNameHex) ;
}


void MonitorModule::setChannelNum(int num)
{
	m_iChannelNum=num ;
}

std::string MonitorModule::getChannelNameHex()
{
	return (m_channelNameHex) ;
}

int MonitorModule::getChannelNum()
{
	return (m_iChannelNum) ;
}

void MonitorModule::setServerName(std::string name)
{
	convertStringToHex(name,m_serverNameHex) ;
}


bool MonitorModule::currentTime(std::string & timeString)
{
	getCurrentTime() ;
	try
	{
		timeString = convertIntToString(m_time) ;
	} catch (BadConversion e)
	{
		return false ;
	}
	return true ;

}

const char* MonitorModule::translate_alpha(unsigned char chr)
{

	static const struct trtab {
		const unsigned char code;
		const char *str;
	}

	trtab[] = {
		{  0, "<NUL>" },
		{  1, "<SOH>" },
		{  2, "<STX>" },
		{  3, "<ETX>" },
		{  4, "<EOT>" },
		{  5, "<ENQ>" },
		{  6, "<ACK>" },
		{  7, "<BEL>" },
		{  8, "<BS>" },
		{  9, "<HT>" },
		{ 10, "<LF>" },
		{ 11, "<VT>" },
		{ 12, "<FF>" },
		{ 13, "<CR>" },
		{ 14, "<SO>" },
		{ 15, "<SI>" },
		{ 16, "<DLE>" },
		{ 17, "<DC1>" },
		{ 18, "<DC2>" },
		{ 19, "<DC3>" },
		{ 20, "<DC4>" },
		{ 21, "<NAK>" },
		{ 22, "<SYN>" },
		{ 23, "<ETB>" },
		{ 24, "<CAN>" },
		{ 25, "<EM>" },
		{ 26, "<SUB>" },
		{ 27, "<ESC>" },
		{ 28, "<FS>" },
		{ 29, "<GS>" },
		{ 30, "<RS>" },
		{ 31, "<US>" },
#ifdef CHARSET_LATIN1
		{ 0x5b, "\304" }, /* upper case A dieresis */
		{ 0x5c, "\326" }, /* upper case O dieresis */
		{ 0x5d, "\334" }, /* upper case U dieresis */
		{ 0x7b, "\344" }, /* lower case a dieresis */
		{ 0x7c, "\366" }, /* lower case o dieresis */
		{ 0x7d, "\374" }, /* lower case u dieresis */
		{ 0x7e, "\337" }}; /* sharp s */
#else /* CHARSET_LATIN1 */
		{ 0x5b, "AE" }, /* upper case A dieresis */
		{ 0x5c, "OE" }, /* upper case O dieresis */
		{ 0x5d, "UE" }, /* upper case U dieresis */
		{ 0x7b, "ae" }, /* lower case a dieresis */
		{ 0x7c, "oe" }, /* lower case o dieresis */
		{ 0x7d, "ue" }, /* lower case u dieresis */
		{ 0x7e, "ss" }}; /* sharp s */
#endif /* CHARSET_LATIN1 */

	int min = 0, max = (sizeof(trtab) / sizeof(trtab[0])) - 1;

	/*	binary search, list must be ordered!	*/
	for (;;) {
		int mid = (min + max) >> 1;
		const struct trtab *tb = trtab + mid;
		int cmp = ((int) tb->code) - ((int) chr);

		if (!cmp) return tb->str;
		if (cmp < 0) {
			min = mid + 1;
			if (min > max) return NULL;
		}
		if (cmp > 0) {
			max = mid - 1;
			if (max < min) return NULL;
		}
	}
}

void MonitorModule::SetFilter(std::string filter,int modus)
{
	m_lpszFilter=filter ;
	m_iFilterModus=modus ;
}

void MonitorModule::setConfigData(XMLNode *pConfigNode)
{
	m_pXMLConfig=pConfigNode ;
	parseConfigData() ;
}

bool MonitorModule::MatchFilter(std::string filterstring)
{
	/*
	int nPos = 0;
	int nReplaced = 0;
	CRegExp r;
	//LPTSTR str = filterstring.c_str() ;

	if (m_iFilterModus==2)
	{
		return true ;
	} ;

	r.RegComp( m_lpszFilter.c_str() );
	if ( (nPos = r.RegFind( m_lpszFilter.c_str())) != -1 )
	{
		return m_iFilterModus==0 ;
	} else {
		return !(m_iFilterModus==0) ;
	}*/
	return true ;
}

void MonitorModule::DebugMessage(std::string message)
{
	//GetBAMon()->LogMessage(message) ;

}

void MonitorModule::SetFunkkanal(int kanal)
{
	m_iFunkkanal=kanal ;
}


