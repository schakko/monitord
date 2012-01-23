#include "SocketThreadFMS32.h"
#include "MonitorLogging.h"

SocketThreadFMS32::SocketThreadFMS32(MonitorConfiguration *config, int LOCKNUM, int PortNum)
	: SocketThread(config, LOCKNUM, PortNum, fms32pro)
{
}

SocketThreadFMS32::~SocketThreadFMS32()
{
}

void SocketThreadFMS32::sayWelcome()
{
	std::string welcomeStringFMS32Pro="#Verbindung zu FMS32-Server erfolgreich hergestellt!\r\n#Verbunden mit Anschluss: " ;
	std::string Port=convertIntToString(this->m_iPortNum) ;

	say(welcomeStringFMS32Pro + Port + "\r\nAN" + Port+"\r\n") ;
}

void SocketThreadFMS32::processInput()
{
	std::string Kommando=m_CommandBuffer;

	if (Kommando == "AllFMS") {
		FILE_LOG (logDEBUG) << "FMS32 Kommando nicht implementiert (Übertragung aktueller Fahrzeugstatus aller Fahrzeuge).";
	} else if (Kommando == "GetAlarmListe") {
		FILE_LOG (logDEBUG) << "FMS32 Kommando nicht implementiert (GetAlarmListe).";
	} else if (Kommando.substr(0, 12) == "GetFMSListe\t") {
		FILE_LOG (logDEBUG) << "FMS32 Kommando nicht implementiert (Übertragung Statuswechsel der letzten n Stunden).";
	} else if (Kommando.substr(0, 12) == "GetPOCListe\t") {
		FILE_LOG (logDEBUG) << "FMS32 Kommando nicht implementiert (Übertragung POCSAG Rufe der letzten n Stunden).";
	} else {
		FILE_LOG (logDEBUG) << "received from FMS32: " <<  m_CommandBuffer;
	}

	return;
}


std::string SocketThreadFMS32::createFMSOutputString(ModuleResultBase Result)
{
	std::string socketText ;
	std::string Feld10="0";
	std::string Feld11="0" ;
	std::string Feld12="0" ;
	std::string Feld13="" ;
	std::string Feld14="0" ;
	std::string Feld15="0" ;

	socketText = std::string("FMSTlg")
					 + "\t" + Result["fmskennung"] //fahrzeugKennung
					 + "\t" + Result["bosdezimal"] //bosDezimalString // kein HEX
					 + "\t" + Result["landdezimal"] //landDezimalString // kein HEX
					 + "\t" + Result["bos"]+Result["land"]+Result["ort"] //  bosString + landString + ortString
					 + "\t" + Result["kfz"]
					 + "\t" + Result["statusdezimal"] // kein HEX
					 + "\t" + Result["baustufe"] //baustufeString
					 + "\t" + Result["richtung"] //richtungString
					 + "\t" + Result["tki"] //tkiString
					 ;
				if (Result["textuebertragung"].size()>0)
				{
					if (convertToInt(Result["richtung"])==0)
					{
						// Vom Fahrzeug, Ortstaste wird im Moment
						// nicht unterstuetzt
						 Feld10="2";
						 Feld11="0" ;
						 Feld12="0" ;
						 Feld13=Result["textuebertragung"] ;
						 Feld14="0" ;
						 Feld15="0" ;
					} else if (convertToInt(Result["richtung"])==1)
					{
						// Von der Leitstelle
						Feld10="0";
						Feld11="-1" ;
						Feld12="0" ;
						Feld13=Result["textuebertragung"] ;
						Feld14="0" ;
						Feld15="0" ;
					}
				}
				socketText	+="\t"+Feld10	// 10: Folgetelegramm vom Fzg 0=Nein,1=Ort,2=Sonst
							+ "\t"+Feld11 	// 11: Folgetelegramm von der Lst 0=Nein, 1=ja
				 			+ "\t"+Feld12	// 12: Folgenummer dezimal (Feld10=1, Ortstaste)
				 			+ "\t"+Feld13	// 13: Folgetelegramm  (vom Fzg in Hex, von der Lst in ASCII
				 			+ "\t"+Feld14	// 14: Soundkarte(0..3)
				 			+ "\t"+Feld15	// 15: Kanal (0,1)
				 		;
	return socketText ;
}

std::string SocketThreadFMS32::createZVEIOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	socketText = std::string("ZVEI")
							 + "\t" + Result["zvei"]
							 + "\t0\t0"
							 ;

	return socketText ;
}

std::string SocketThreadFMS32::createPOCSAGOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	std::string subString ;
	int sub=0 ;
	try {
		sub = convertToInt( Result["sub"])+1 ;
	}
	catch (BadConversion)
	{
		sub=0 ;
		FILE_LOG (logERROR) << "Error converting subaddress : " <<  Result["sub"] ;
	}

	subString=convertIntToString(sub) ;

	socketText = std::string("POC")
				 + "\t" + Result["ric"]
				 + "\t" + subString //Funktionsbit
				 + "\t" + Result["text"]
				 + "\t0\t0"
				 ;
	return socketText ;
}
