#ifndef __MonitorConfiguration_H
#define __MonitorConfiguration_H 0

#ifndef WIN32
	#define TCHAR char
#endif

#include <string>
#include "MonitorModules.h"
#include "xmltools.h"

enum MonitorChannel {links=0,rechts=1} ;

struct LoginAccount
{
	std::string Loginname ;
	std::string Password ;
} ;

class SNDConfiguration
{
public:
	int iAktiv ;
	std::string sDevice ;
	int iChannel ;
	int iFMS[2] ;
	int iZVEI[2] ;
	int iPOC512[2] ;
	int iPOC1200[2] ;

	// Namen der Kanaele (hier geht komischerweise kein std::string mit dem gcc 4.0.3
	char sChannelName0[255] ;
	char sChannelName1[255] ;

	XMLNode configSoundcard ;
	XMLNode configChannel[2];
	// Parameter der Module
	XMLNode configFMS[2],
			configZVEI[2],
			configPOC512[2],
			configPOC1200[2] ;

} ;

class MonitorConfiguration
{
public:
	MonitorConfiguration() ;
	virtual ~MonitorConfiguration() ;

	SNDConfiguration m_sndConfig[4] ;
	std::string m_sMonitordName ;
	std::string m_sLogfile ;
	std::string m_sLoglevel ;
	bool ReadConfiguration(std::string filename) ;
	bool ResetConfiguration() ;
	bool IsValidLogin(std::string loginname, std::string password, std::string host="") ;

	//MonitorModuleArray m_ModulesLinks[4] ;
	//MonitorModuleArray m_ModulesRechts[4] ;
	unsigned int m_Port ;
	unsigned int m_PortFMS32Pro ;
	unsigned int m_PortCrusader ;
	XMLNode m_configDataPlugins ;

	std::string m_BindIP ;
	bool m_Daemonize ;
#ifdef WIN32
	bool m_service_install;
	bool m_service_run;
	bool m_service_uninstall;
#endif
	std::string m_ConfigFile ;
	std::string m_crusaderUsername ;
	std::string m_socketFilterFileName ;
	std::string m_pluginFilterFileName ;
	
	bool ParseCommandline (int argc, TCHAR * argv[]) ;
/*	int getNodeInt(XMLNode parent,std::string childName,int defaultValue) ;
	std::string getNodeText(XMLNode parent,std::string childName,std::string defaultValue) ;
*/

protected:
	std::string ReadChannel(XMLNode channelNode, int sndCard, int channelNum) ;
	void ReadSoundCard(XMLNode sndNode, int sndCard);
	void ReadTCPConfiguration(XMLNode tcpNode) ;

	void ReadAuthenticationData(XMLNode authNode) ;
	void ReadMySQLConfiguration(XMLNode dbNode, int moduleNum=0);
	std::vector<struct LoginAccount> m_MasterLogins ;
	std::vector<std::string> m_IPBlacklist ;
	std::vector<std::string> m_IPWhitelist ;
	std::vector<std::string> m_IPLoginlist ;

};

#endif
