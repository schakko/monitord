#include "SocketThreadCrusader.h"
#include <iostream>
#include "MonitorLogging.h"
#include "convert.h"

using namespace std ;

SocketThreadCrusader::SocketThreadCrusader(MonitorConfiguration *config, int LOCKNUM, int PortNum)
	: SocketThread(config, LOCKNUM, PortNum, monitord)
{
	telegrammCounter=0 ;
}

SocketThreadCrusader::~SocketThreadCrusader()
{
}

void SocketThreadCrusader::sayWelcome()
{
}

std::string SocketThreadCrusader::createFMSOutputString(ModuleResultBase Result)
{
	std::string baustufeCString, richtungCString ;
	std::string counterCString ;
	std::string socketText ;

	baustufeCString = convertStringBoolText (Result["baustufe"]);
	richtungCString = convertStringBoolText (Result["richtung"]);

	counterCString=convertIntToString(telegrammCounter++) ;


	socketText = Result["datum"] // dateStr
				 + "#![]!#" + Result["uhrzeit"] //timeStr
				 + "#![]!#" + Result["fmskennung"] // fahrzeugKennung
				 + "#![]!#" + Result["status"] // statusString
				 + "#![]!#" + baustufeCString
				 + "#![]!#" + richtungCString
				 + "#![]!#" + Result["tki"]
				 + "#![]!#-1#![]!#false#![]!#false#![]!#" + counterCString  ;

				 /*
				 if ( (Result["textuebertragung"].size()>0) && (convertToInt(Result["status"])==10))
				 {
				 	socketText+=   "#![]!#" + Result["textuebertragung"]
				 				 + "#![]!#" ;
				 }
				 */

	return socketText ;

}

std::string SocketThreadCrusader::createZVEIOutputString(ModuleResultBase Result)
{
	std::string socketText ;
	std::string hexText ;
	std::string counterCString ;

	convertStringToHex(Result["text"],hexText) ;
	counterCString=convertIntToString(telegrammCounter++) ;

	socketText = Result["datum"]
				 + "#![]!#" + Result["uhrzeit"]
				 + "#![]!#" + Result["zvei"]
				 + "#![]!#16#![]!#false#![]!#true#![]!#-1#![]!#-1#![]!#false#![]!#false#![]!#" + counterCString
				 + "#![]!#Melderalarm#![]!#"
				 ;

	return socketText ;
}

std::string SocketThreadCrusader::createPOCSAGOutputString(ModuleResultBase Result)
{
	std::string socketText ;
	std::string hexText ;
	std::string counterCString ;

	counterCString=convertIntToString(telegrammCounter++) ;
	convertStringToHex(Result["text"],hexText) ;

	socketText = Result["datum"]
				 + "#![]!#" + Result["uhrzeit"]
				 + "#![]!#" + Result["ric"] + Result["subhex"]
				 + "#![]!#17#![]!#false#![]!#true#![]!#0#![]!#-1#![]!#false#![]!#false#![]!#" + counterCString
				 + "#![]!#" + Result["text"]
				 + "#![]!#"
				 ;
	return socketText ;
}

void SocketThreadCrusader::processInput()
{
	// Anmeldung ?
	std::string Kommando=m_CommandBuffer ;

		if (Kommando.substr(0,5)=="PASS:")
		{
			std::string password=Kommando.substr(5) ;
			checkLogin(password) ;
		}
		else if (Kommando=="ClientCiao")
		{
			doLogout() ;
		} else if (Kommando.substr(8, 6) == "#![]!#") {
			ModuleResultBase *pfms = new ModuleResultBase();
			size_t position;
			std::string datum, zeit, fmskennung, status, a, b, c, d, e, f, g, text;
			/* Status im Crusader manuell gesetzt:
			 * 23.07.08#![]!#22:19:36#![]!#93185811#![]!#7#![]!#true#![]!#false#![]!#3#![]!#1#![]!#false#![]!#true#![]!#-1#![]!#7 TelegrammText#![]!#
			 */
			 FILE_LOG (logDEBUG) << "manueller Status von Crusader Client empfangen.";

			/* es wird nur FMS betrachtet */
			pfms->set ("typ", "fms");
			/* Feld  1: Datum */
			pfms->set ("datum", Kommando.substr(0, 8));
			/* Feld  2: Uhrzeit */
			position = Kommando.find_first_of(CRUSADER_DELIMITER) + 6;
			pfms->set ("uhrzeit", Kommando.substr(position, 8));
			/* Feld  3: FMS Kennung */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			fmskennung = Kommando.substr(position, 8);
			pfms->set ("fmskennung", fmskennung);
			pfms->set ("bosdezimal", convertIntToString (convertNibbleToInt (fmskennung[0])));
			pfms->set ("landdezimal", convertIntToString (convertNibbleToInt (fmskennung[1])));
			pfms->set ("bos", fmskennung.substr(0, 1));
			pfms->set ("land", fmskennung.substr(1, 1));
			pfms->set ("ort", fmskennung.substr(2, 2));
			pfms->set ("kfz", fmskennung.substr(4, 4));
			/* Feld  4: Fahrzeugstatus */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			status = Kommando.substr(position, 1);
			pfms->set ("status", status);
			pfms->set ("statusdezimal", convertIntToString (convertNibbleToInt (status[0])));
			// TODO Status 16 bedeutet ZVEI und 17 POCSAG
			/* Feld  5: Baustufe */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			pfms->set ("baustufe", convertStringTextBool(Kommando.substr(position, Kommando.find_first_of(CRUSADER_DELIMITER, position) - position)));
			/* Feld  6: Richtung */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			pfms->set ("richtung", convertStringTextBool(Kommando.substr(position, Kommando.find_first_of(CRUSADER_DELIMITER, position) - position)));
			/* Feld  7: TKI */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			pfms->set ("tki", Kommando.substr(position, 1));
			/* Feld  8: ? */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			/* Feld  9: ? */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			/* Feld 10: ? */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			/* Feld 11: ? */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			/* Feld 12: Text */
			position = Kommando.find_first_of(CRUSADER_DELIMITER, position) + 6;
			pfms->set ("textuebertragung", Kommando.substr(position, Kommando.find_first_of(CRUSADER_DELIMITER, position) - position));

			FILE_LOG(logDEBUG) << "Debug(FMS vom Crusader):" << endl << (*pfms);

			GlobalDispatcher->addResult (pfms);
		} else {
			FILE_LOG (logDEBUG) << "received from Crusader: " <<  m_CommandBuffer ;
		}
}

void SocketThreadCrusader::checkLogin(std::string password)
{
	if (m_MonitorConfiguration->IsValidLogin(m_MonitorConfiguration->m_crusaderUsername,password,m_sClientIP))
	{
		m_authenticated=true ;
		FILE_LOG(logINFO) << "login accepted (crusader pw allowed): from ip " << m_sClientIP ;
	} else {
		FILE_LOG(logINFO) << "login denied (crusader pw not allowed): " << password << " from ip " << m_sClientIP ;
	}
}

