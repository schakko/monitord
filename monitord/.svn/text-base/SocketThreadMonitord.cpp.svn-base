#include "SocketThreadMonitord.h"
#include "config.h"
#include "Monitor.h"
#include <algorithm>
#include <iostream>

#include "MonitorLogging.h"

const std::string welcomeString="100;" +std::string(PACKAGE_STRING)+ " READY\r\n";
const std::string authenticateString="101:001\r\n" ;
const std::string illegalFormatString="101:004\r\n" ;

const std::string CAPABILITY_MONITORD_VERSION="0020" ;
const std::string CAPABILITY_MONITORD_PROTOCOLVERSION="0004" ;

using namespace std;

SocketThreadMonitord::SocketThreadMonitord(MonitorConfiguration *config, int LOCKNUM, int PortNum)
	: SocketThread(config, LOCKNUM, PortNum, monitord)
{
}

SocketThreadMonitord::~SocketThreadMonitord()
{
}

void SocketThreadMonitord::sayWelcome()
{
	say(welcomeString) ;
}

bool SocketThreadMonitord::parseCommand()
{
	m_paramCount=0 ;
	std::string parserTemp =m_CommandBuffer ;
	std::string item ; // Aktueller Teil zwischen den Doppelpunkten
	size_t len = 0 ;      // Laenge bis zum naechsten Doppelpunkt
	int itemCount=0;   // Wievielter Parameter ?
	bool lastItem=false ; // letzter Eintrag ? (fuer CR/LF interessant)
	bool illegalCommand=false ;
	int i ;

	for (i=0;i<MAX_PARAMS;i++)
	{
		m_cmdParam[i].clear() ;
	}

	// übergebene Zeichenkette in die einzelnen Parameter zerlegen. Kommentare am Ende ( Durch ";" abgetrennt ) entfernen

	len=parserTemp.find_last_of(";", 0) ;

	if (len!=std::string::npos) // Kein ";" gefunden ? - sonst abschneiden
	{
		parserTemp=parserTemp.substr(0,len) ;
	}
	do
	{
		len=parserTemp.find(":", 0) ;

		if (len==std::string::npos) // Kein ":" gefunden
		{
			if (!parserTemp.empty())
			{
				// CR/LF entfernen
				// TODO: Hier eigentlich noch auf CRLF prüfen !
				lastItem=true ;
				len=parserTemp.length(); // war: len=parserTemp.length()-2;
			}
		}

		if (len!= std::string::npos)
		{
			item=parserTemp.substr(0,len) ;

			if (lastItem)
			{
				parserTemp.erase(0,parserTemp.length()) ;
			} else {
				parserTemp.erase(0,len+1) ;
			}
			switch (itemCount)
			{
				case 0: // 3stelliges Kommando zwingend zu beginn !
						if (len!=3) {
							illegalCommand=true ;
						}
						m_cmdString=item ;
						break ;
				default:
						if (itemCount < MAX_PARAMS)
						{
							m_cmdParam[itemCount-1]=item ;
						}
						break;
			}
			itemCount++ ;
		}

		if (illegalCommand)
		{
			len=std::string::npos ;
			m_paramCount=0 ;
		} else
		{
			m_paramCount=itemCount ;
		}

	} while ((len!= std::string::npos) && (itemCount<=MAX_PARAMS));

	return true ;
}

void SocketThreadMonitord::processInput()
{
	bool isProcessed=false ;
	if (parseCommand())	{
		int cmd ;

		try {
			FILE_LOG(logDEBUG) << "command from client: " << m_cmdString  ;
			cmd = convertToInt(m_cmdString) ;
		}
		catch (std::runtime_error err)
		{
			FILE_LOG(logERROR) << "Fehler bei der Cmd Konvertierung: " << m_cmdString  ;
			cmd=-1 ;
			say(illegalFormatString) ;
		}

		// Bevor wir pruefen, ob eine Anmeldung vorliegt gibt es zwei Kommandos,
		// die trotzdem ausgefuehrt werden: 299 (bye) und 220 (Login)
		switch (cmd)
		{
			case 299:
					isProcessed=true ;
					say ("199\r\n") ; // Logout
					doLogout() ;
					break ;
			case 220:
					isProcessed=true ;
					checkLogin() ;
					break ;
			case 210: // Inquiery
					isProcessed=true ;
					tellCapabilites() ;
					break ;
			/*
			case 999: // Stop Server
					say ("going down...\r\n") ;
					m_monitor.m_bWantStop=true ;
			*/
			default:
					// Nichts tun ...
					break ;
		}

		if (isProcessed==false)
		{
			if (!m_authenticated)
			{
				say (authenticateString) ;
			} else
			{
				// Sonst alle anderen Kommandos jetzt pruefen ...

				switch (cmd)
				{
					case 202: // Keepalive
						say ("100\r\n") ; // OK
						break ;
					case 203: // ChannelInfo
						tellChannels() ;
						break ;
					case 204: // Record
						say ("101:005\r\n") ; // 005:Not implemented
						// auskommentiert, weil die Recording-Funktionalität nicht stabil/sicher ist.
						// Rückmeldung an den Client: not implemented
						// startRecording() ;
						break ;
					default:
						say(illegalFormatString);
					break;
				}
			}
		}
	} else {
		say(illegalFormatString) ;
	}
}

void SocketThreadMonitord::tellCapabilites()
{
	std::string hexOS ;
	//std::string hexChannelname;
	std::string hexProgramName ;
	std::string hexProgramVersion ;
	std::string hexModules ;

	#ifdef WIN32
		convertStringToHex("WINDOWS",hexOS) ;
	#else
		convertStringToHex("LINUX",hexOS) ;
	#endif

	hexProgramName=convertStringToHex(PACKAGE_NAME) ;
	//hexProgramVersion=convertStringToHex(CAPABILITY_MONITORD_VERSION) ;
	//hexModules=convertStringToHex(std::string("REC")) ;

	say ("111:1:" + hexProgramName + "\r\n") ;
	say ("111:2:" + hexOS+ "\r\n") ;
	say ("111:3:" + CAPABILITY_MONITORD_VERSION+ "\r\n") ;
	say ("111:4:" + CAPABILITY_MONITORD_PROTOCOLVERSION+ "\r\n") ;
	say (std::string("111:5:") + "\r\n") ;
	say (std::string("111:0") + "\r\n") ;
}

void SocketThreadMonitord::tellChannels()
{
#if 1 /* FIXME */
// FIXME done: monitord_config scheinbar wieder "richtig drin", es kommen jedenfalls Antworten gemäß der Konfiguration... mdi 29032011
	std::string hexString ;
	int summe ;

	for (int i=3; i>=0;i--)
	{
		if (m_MonitorConfiguration->m_sndConfig[i].iAktiv==1)
		{
			convertStringToHex(m_MonitorConfiguration->m_sndConfig[i].sChannelName0,hexString) ;
			summe=	 	  1*(m_MonitorConfiguration->m_sndConfig[i].iZVEI[0])
						+ 2*(m_MonitorConfiguration->m_sndConfig[i].iFMS[0])
						+ 4*(m_MonitorConfiguration->m_sndConfig[i].iPOC512[0])
						+ 8*(m_MonitorConfiguration->m_sndConfig[i].iPOC1200[0])
						;
			say ("103:" + convertIntToString(2*i+1) + ":" + hexString + ":" + convertIntToString(summe) +"\r\n") ;

			convertStringToHex(m_MonitorConfiguration->m_sndConfig[i].sChannelName1,hexString) ;
			summe=	 	  1*(m_MonitorConfiguration->m_sndConfig[i].iZVEI[1])
						+ 2*(m_MonitorConfiguration->m_sndConfig[i].iFMS[1])
						+ 4*(m_MonitorConfiguration->m_sndConfig[i].iPOC512[1])
						+ 8*(m_MonitorConfiguration->m_sndConfig[i].iPOC1200[1])
						;
			say ("103:" + convertIntToString(2*i) + ":" + hexString + ":" + convertIntToString(summe) +"\r\n") ;
		}
	}
#endif
}


void SocketThreadMonitord::startRecording(int seconds, int channel)
{
	int tempSeconds=0;
	int tempChannel=0;

	bool bErrConversion=false ;
	try
	{
		tempChannel=convertToInt(m_cmdParam[0]) ;
		tempSeconds=convertToInt(m_cmdParam[1]) ;
	} catch (BadConversion e)
	{
		bErrConversion=true ;
	}

	if (bErrConversion==false)
	{
		seconds=tempSeconds ;
		channel=tempChannel ;
	}

	int sndCardNum =channel / 2 ;
	int sndCardLeftRight=channel % 2 ;

	FILE_LOG(logDEBUG) << "Starte Aufnahme mit" << seconds << " Sekunden" << " auf Karte " << sndCardNum << ", Kanal=" << sndCardLeftRight << endl ;
	std::string command=std::string("RECORD:" + convertIntToString(seconds)+ ":") + convertIntToString(channel) ;
#if 0 /* FIXME muß noch umstrukturiert werden */
	std::string resultString =m_monitor.m_sndIn[sndCardNum].PluginCommand(sndCardLeftRight,command,this);

	if (resultString=="not accepted")
	{
		say ("101:009\r\n") ;
	};

	if (resultString=="not implemented")
	{
		say ("101:005\r\n") ;
	};
#endif
}
void SocketThreadMonitord::checkLogin()
{


	// if (m_authenticated==false)
	// Auskommentiert: Auch bei freigeschalteter IP noch eine
	// Anmeldung zulassen. Auth-Status wird bei falschem Benutzernamen aber
	// nicht zurückgesetzt. Dient eher für Filter u.ä. um ggf. höhere Rechte
	// zu erlangen. Nur: Was antwortet man dem Client dann, wenn er das falsche PW ausgibt ?
	//
	{
		if (m_paramCount>3)
		{
			std::string loginname,password ;
			std::string protocol ;
			try {
				HexToString(0,loginname) ;
				HexToString(1,password) ;
				protocol= m_cmdParam[2] ; //HexToString(2,protocol) ;
			}
			catch (std::runtime_error err)
			{
				return ;
			}

			if (protocol==CAPABILITY_MONITORD_PROTOCOLVERSION)
			{
				if (m_MonitorConfiguration->IsValidLogin(loginname,password,m_sClientIP))
				{
					m_authenticated=true ;
					m_loginname=loginname ;
					FILE_LOG(logINFO) << "login accepted (user allowed): " << m_loginname << " from ip " << m_sClientIP ;
					say ("100\r\n") ; // Login OK
				} else {
					say ("101:003\r\n") ; // Benutzername falsch
					FILE_LOG(logINFO) <<"login denied: " << m_loginname << " from ip " << m_sClientIP ;
				}
			} else {
				say ("101:008\r\n") ; // Falsche Protokollversion
				FILE_LOG(logINFO) <<"login denied, incorrect protocol version: " << protocol << " from ip: " << m_sClientIP << " with username: " << loginname ;
			}
		} else {
			FILE_LOG(logINFO) << "login with too few arguments detected" ;
			say ("101:004;TOO FEW ARGUMENTS\r\n") ; // Fehler: Anfrage nicht verstanden !
		}
	}

	/*
	} else {
		say ("100\r\n") ; // Login OK
	}
	*/

}



std::string SocketThreadMonitord::createFMSOutputString(ModuleResultBase Result)
{
	std::string socketText ;
	std::string text ;

	convertStringToHex(Result["textuebertragung"],text) ;

	socketText = std::string("310")
				 + ":" + Result["timestamp"]
				 //+ ":" + Result["servernamehex"]
				 //+ ":" + Result["channelnamehex"]
				 + ":" + Result["channelnum"]
				 + ":" + Result["fmskennung"]
				 + ":" + Result["status"]
				 + ":" + Result["baustufe"]
				 + ":" + Result["richtung"]
				 + ":" + Result["tki"] ;

				 if (text.size()>0)
				 {
				    socketText+= ":" + text ;
				 } ;

	 std::transform (	socketText.begin(),
	 					socketText.end(),
	 					socketText.begin(),
             			(int(*)(int)) toupper);

	return socketText ;
}

std::string SocketThreadMonitord::createZVEIOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	std::string hexText;

	convertStringToHex(Result["text"],hexText) ;

	socketText = std::string("300")
			 + ":" + Result["timestamp"]
			 //+ ":" + Result["servernamehex"]
			 //+ ":" + Result["channelnamehex"]
			 + ":" + Result["channelnum"]
			 + ":" + Result["zvei"]
			 + ":" + Result["weckton"]
			 + ":" + hexText ;
			 ;
			std::transform (socketText.begin(), socketText.end(), socketText.begin(),
   (int(*)(int)) toupper);

	return socketText ;
}

std::string SocketThreadMonitord::createPOCSAGOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	std::string hexText;

	convertStringToHex(Result["text"],hexText) ;

	socketText = std::string("320")
			 + ":" + Result["timestamp"]
			 //+ ":" + Result["servernamehex"]
			 //+ ":" + Result["channelnamehex"]
			 + ":" + Result["channelnum"]
			 + ":" + Result["ric"]
			 + ":" + Result["sub"]
			 + ":" + hexText ;
			 ;
			 std::transform (socketText.begin(), socketText.end(), socketText.begin(),
              (int(*)(int)) toupper);
    return socketText ;
}
