
#include "MonitorConfiguration.h"
#include "convert.h"
#include <iostream>
#include "MonitorLogging.h"

#ifdef WIN32
	#include <winsock2.h> // gethostbyname, inet_ntoa
#else
	#include <netdb.h> // fuer gethostbyname
	#include <arpa/inet.h> // fuer inet_ntoa
#endif

// Startparameter (Kommandozeile) auslesen
#include <simpleopt/SimpleOpt.h>
#include <simpleopt/SimpleGlob.h>


using namespace std ;

// ----------

MonitorConfiguration::MonitorConfiguration()
{
	ResetConfiguration() ;
}

MonitorConfiguration::~MonitorConfiguration()
{
}

// ----------

bool MonitorConfiguration::ResetConfiguration()
{
	int i ;

	m_sMonitordName="default" ;
	m_Daemonize=true ;
#ifdef WIN32
	m_service_install=false;
	m_service_run=false;
	m_service_uninstall=false;
#endif
	m_ConfigFile="monitord.xml" ;
	m_crusaderUsername="crusader" ;

	m_IPBlacklist.clear() ;
	m_IPWhitelist.clear() ;
	m_IPLoginlist.clear() ;

	m_Port=9333 ;
	m_PortFMS32Pro=0 ;
	m_PortCrusader=0 ;
	m_BindIP="" ;

	m_sndConfig[0].sDevice="/dev/dsp" ;
	m_sndConfig[1].sDevice="/dev/dsp1" ;
	m_sndConfig[2].sDevice="/dev/dsp2" ;
	m_sndConfig[3].sDevice="/dev/dsp3" ;

	for (i=0; i<4;i++)
	{
		m_sndConfig[i].iAktiv=0 ;
		m_sndConfig[i].iChannel=2 ;
		m_sndConfig[i].iFMS[0]=0 ;
		m_sndConfig[i].iFMS[1]=0 ;
		m_sndConfig[i].iZVEI[0]=0 ;
		m_sndConfig[i].iZVEI[1]=0 ;
		m_sndConfig[i].iPOC512[0]=0 ;
		m_sndConfig[i].iPOC512[1]=0 ;
		m_sndConfig[i].iPOC1200[0]=0 ;
		m_sndConfig[i].iPOC1200[1]=0 ;

		strcpy(m_sndConfig[i].sChannelName0,"default") ;
		strcpy(m_sndConfig[i].sChannelName1,"default") ;
	}

	return true ;
}

std::string MonitorConfiguration::ReadChannel(XMLNode channelNode, int sndCard, int channelNum)
{
	std::string name;
	int nModules=channelNode.nChildNode("module");
	m_sndConfig[sndCard].iFMS[channelNum]= 0 ;
	m_sndConfig[sndCard].iZVEI[channelNum]= 0 ;
	m_sndConfig[sndCard].iPOC512[channelNum]= 0 ;
	m_sndConfig[sndCard].iPOC1200[channelNum]= 0 ;
	name = getNodeText(channelNode,"name","default") ;

	for (int module=0; module<nModules ;++module)
	{
		XMLNode moduleNode ;
		if (!((moduleNode=channelNode.getChildNode("module",module))).isEmpty())
		{
			std::string moduleType=moduleNode.getAttribute("type") ;
			if (moduleType=="fms")
			{
				m_sndConfig[sndCard].iFMS[channelNum]= 1 ;
				m_sndConfig[sndCard].configFMS[channelNum]=moduleNode ;
				std::string test=moduleNode.createXMLString() ;

			} else if (moduleType=="zvei")
			{
				m_sndConfig[sndCard].iZVEI[channelNum]= 1 ;
				m_sndConfig[sndCard].configZVEI[channelNum]=moduleNode ;
			}

			m_sndConfig[sndCard].iPOC512[channelNum]= (moduleType=="poc512" ? 1:m_sndConfig[sndCard].iPOC512[channelNum]);
			m_sndConfig[sndCard].iPOC1200[channelNum]= (moduleType=="poc1200" ? 1:m_sndConfig[sndCard].iPOC1200[channelNum]);
		}
	}

	return name;
}

void MonitorConfiguration::ReadSoundCard(XMLNode sndNode, int sndCard)
{
	// Daten pro Soundkarte
	std::string channelName ;
	m_sndConfig[sndCard].configSoundcard=sndNode ;
	m_sndConfig[sndCard].iAktiv=getNodeInt(sndNode,"status",0) ;
	m_sndConfig[sndCard].sDevice=getNodeText(sndNode,"device",m_sndConfig[sndCard].sDevice) ;

	// Kanaele lesen (part=left/right)
	int nChannels=sndNode.nChildNode("channel");
	for (int channel=0; channel<nChannels ; ++channel)
	{
		XMLNode channelNode ;
		if (!((channelNode=sndNode.getChildNode("channel",channel))).isEmpty())
		{
			std::string channelPart=channelNode.getAttribute("part");
			if (channelPart=="left") // Linker Kanal
			{
				m_sndConfig[sndCard].configChannel[0]=channelNode ;
				channelName=ReadChannel(channelNode,sndCard,0) ;
				strcpy(m_sndConfig[sndCard].sChannelName0,channelName.c_str()) ;
			}  else if (channelPart=="right") // Rechter Kanal
			{
				m_sndConfig[sndCard].configChannel[1]=channelNode ;
				channelName=ReadChannel(channelNode,sndCard,1) ;
				strcpy(m_sndConfig[sndCard].sChannelName1,channelName.c_str()) ;
			}
		}
	}
}

void MonitorConfiguration::ReadAuthenticationData(XMLNode authNode)
{
	std::string user,pass ;
	std::string hostip ;

	// Loginnamen lesen
	int nLogin=authNode.nChildNode("login");

	for (int num=0; num<nLogin ; ++num)
	{
		XMLNode loginNode ;
		if (!((loginNode=authNode.getChildNode("login",num))).isEmpty())
		{
			user=getNodeText(loginNode,"name","") ;
			pass=getNodeText(loginNode,"password","") ;

			if (user!="") // Leerer Benutzer geht man nicht !
			{
				struct LoginAccount *accountData = new struct LoginAccount ;
				accountData->Loginname=user ;
				accountData->Password=pass ;

				m_MasterLogins.push_back(*accountData) ;
			}
		}
	}

	// Loginnamen lesen
	int nIP=authNode.nChildNode("ip");

	for (int num=0; num<nIP ; ++num)
	{
		XMLNode ipNode ;
		if (!((ipNode=authNode.getChildNode("ip",num))).isEmpty())
		{
			hostip=ipNode.getText() ;
			std::string action=ipNode.getAttribute("action") ;

			if (action=="allow")
			{
				m_IPWhitelist.push_back(hostip) ;
			} else if (action=="deny")
			{
				m_IPBlacklist.push_back(hostip) ;
			} else if (action=="login")
			{
				m_IPLoginlist.push_back(hostip) ;
			}
		}
	}
}

void MonitorConfiguration::ReadTCPConfiguration(XMLNode tcpNode)
{
	m_BindIP = getNodeText(tcpNode,"bind","") ;
	m_Port = getNodeInt(tcpNode,"port",9333) ; // Altes Format, kein Tarnkappenmodus

	m_PortFMS32Pro=0 ;
	m_PortCrusader=0 ;

	// Neue Optionen auch pr�fen:
	//
	int nPort=tcpNode.nChildNode("port");

	for (int num=0; num<nPort ; ++num)
	{
		XMLNode portNode ;
		if (!((portNode=tcpNode.getChildNode("port",num))).isEmpty())
		{
			std::string portText=portNode.getText() ;
			int Port=convertToInt(portText) ;
			std::string mode=portNode.getAttribute("mode") ;

			if (mode=="monitord")
			{
				m_Port=Port ;
			} else if (mode=="fms32pro")
			{
				m_PortFMS32Pro=Port ;
			} else if (mode=="crusader")
			{
				m_PortCrusader=Port ;
			}
		}
	}

	if (!m_BindIP.empty())
	{
		hostent *host = gethostbyname(m_BindIP.c_str()) ;
		if (host)
		{
			in_addr * address=(in_addr * )host->h_addr;
  			m_BindIP=inet_ntoa(* address);

		} else {
			 m_BindIP="" ;
		}
	}
}

bool MonitorConfiguration::ReadConfiguration(std::string filename)
{
	XMLNode xNode;
	XMLNode config=XMLNode::openFileHelper(filename.c_str(),"monitordconfig");

	// Monitord Bezeichnung
	m_sMonitordName = getNodeText(config,"name","default") ;
	m_sLogfile = getNodeText(config,"logfile","screen") ;
	m_sLoglevel = getNodeText(config,"loglevel","INFO") ;


	m_socketFilterFileName=getNodeText(config,"SocketFilterScript","") ;
	m_pluginFilterFileName=getNodeText(config,"PluginFilterScript","") ;
	XMLNode tcpNode = config.getChildNode("tcpsocket",0) ;

	if (!(tcpNode.isEmpty()))
	{
		ReadTCPConfiguration(tcpNode) ;
	}


	// Anmeldedaten
	XMLNode authNode = config.getChildNode("auth",0) ;

	if (!(authNode.isEmpty()))
	{
		ReadAuthenticationData(authNode) ;
	}

	// Soundkarten Elemente durchgehen
	int nSoundkarten=config.nChildNode("soundcard");
	for (int sndCard=0;sndCard<nSoundkarten;++sndCard)
	{
		XMLNode sndNode = config.getChildNode("soundcard",sndCard) ;

		// SoundCard (i) erfolgreich gelesen ?
		if (!((sndNode=config.getChildNode("soundcard",sndCard))).isEmpty())
		{
			std::string stringNum=sndNode.getAttribute("num");
			int iKarte = convertToInt(stringNum) ;

			ReadSoundCard(sndNode,iKarte) ;
		}
	}

	// Datenplugins
	m_configDataPlugins = config.getChildNode("dataplugins") ;
	/*if (m_configDataPlugins.isEmpty()==false)
	{
		FILE_LOG(logINFO) << "dataplugins found !"  ;
	}*/

	return true;
}


bool MonitorConfiguration::ParseCommandline(int argc, TCHAR * argv[])
{
	// define the ID values to indentify the option
	enum { OPT_HELP, OPT_FLAG, OPT_ARG };

	CSimpleOpt::SOption g_rgOptions[] = {
    	{ OPT_FLAG, _T((char *)"-d"),     SO_NONE    }, // "-d" = daemon
    	{ OPT_ARG,  _T((char *)"-c"),     SO_REQ_SEP }, // "-c ARG" = config
    	{ OPT_ARG, _T((char *)"--dev0"),     SO_REQ_SEP   }, // Geraete
    	{ OPT_ARG, _T((char *)"--dev1"),     SO_REQ_SEP   }, //
    	{ OPT_ARG, _T((char *)"--dev2"),     SO_REQ_SEP   }, //
    	{ OPT_ARG, _T((char *)"--dev3"),     SO_REQ_SEP   }, //
#ifdef WIN32
    	{ OPT_FLAG, _T((char *)"--install"),   SO_NONE    }, // "--install" = install service
    	{ OPT_FLAG, _T((char *)"--uninstall"), SO_NONE    }, // "--uninstall" = uninstall service
    	{ OPT_FLAG, _T((char *)"--service"),   SO_NONE    }, // "--service" = run as service (only for the service control daemon
#endif
    	SO_END_OF_OPTIONS                       // END
	} ;

	// Jetzt die Kommnadozeile parsen ...
	CSimpleOpt args(argc, argv, g_rgOptions);

	// process any files that were passed to us on the command line.
    // send them to the globber so that all wildcards are expanded
    // into valid filenames (e.g. *.cpp -> a.cpp, b.cpp, c.cpp, etc)
    // See the SimpleGlob.h header file for details of the flags

    CSimpleGlob glob(SG_GLOB_NODOT|SG_GLOB_NOCHECK);
    if (SG_SUCCESS != glob.Add(args.FileCount(), args.Files())) {
        return false;
    }


    // while there are arguments left to process
    while (args.Next()) {
        if (args.LastError() == SO_SUCCESS) {
        	if (strcmp (args.OptionText(), "-d") == 0){
    			m_Daemonize=true ;
        	} else if (strcmp (args.OptionText(), "-c") == 0) {
    			m_ConfigFile=args.OptionArg() ;
        	} else if (strcmp (args.OptionText(), "--dev0") == 0) {
    			m_sndConfig[0].sDevice=args.OptionArg() ;
        	} else if (strcmp (args.OptionText(), "--dev1") == 0) {
    			m_sndConfig[1].sDevice=args.OptionArg() ;
        	} else if (strcmp (args.OptionText(), "--dev2") == 0) {
    			m_sndConfig[2].sDevice=args.OptionArg() ;
        	} else if (strcmp (args.OptionText(), "--dev3") == 0) {
    			m_sndConfig[3].sDevice=args.OptionArg() ;
#ifdef WIN32
        	} else if (strcmp (args.OptionText(), "--install") == 0) {
    			m_service_install = true;
        	} else if (strcmp (args.OptionText(), "--service") == 0) {
    			m_service_run = true;
        	} else if (strcmp (args.OptionText(), "--uninstall") == 0) {
    			m_service_uninstall = true;
#endif
        	}
        }
        else {
            return false;
        }
    }

	return true;
}

bool MonitorConfiguration::IsValidLogin(std::string loginname, std::string password, std::string host)
{
	bool skipBlackList=false ;
	std::vector<std::string>::iterator i ;

	// In IP Whitelist ?
	for (i=m_IPWhitelist.begin() ; i != m_IPWhitelist.end() ; ++i)
	{
		if ((host==*i) || (*i=="any")) // Treffer oder Codewort "any" ?
		{
			return true ;
		}
	}

	// In IP Login erforderlich ?
	for (i=m_IPLoginlist.begin() ; i != m_IPLoginlist.end() ; ++i)
	{
		if ((host==*i) || (*i=="any")) // Treffer oder Codewort "any" ?
		{
			skipBlackList=true ;
		}
	}

	if (!skipBlackList)
	{
		// In IP Blacklist ?
		for (i=m_IPBlacklist.begin() ; i != m_IPBlacklist.end() ; i++)
		{
			if ((host==*i) || (*i=="any")) // Treffer oder Codewort "any" ?
			{
				return false ;
			}
		}
	}

	// Name und Passwort pruefen
	for (std::vector<struct LoginAccount>::iterator i=m_MasterLogins.begin() ; i != m_MasterLogins.end() ; i++)
	{
		if ((loginname==i->Loginname) && (password==i->Password))
		{
			return true ;
		}
	}
	return false ;
}

