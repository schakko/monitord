#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <iostream>

#include "Monitor.h"
#include "SocketServer.h"
#include "MonitorConfiguration.h"
#include "SocketThreadMonitord.h"

#include "MonitorLogging.h"

#ifdef PLUGINS
#include "plugins/mplugin.h"
#include "PluginThread.h"
#endif
#ifdef WIN32
#include <monitord/win32/MonitorService.h>
#define usleep Sleep
#endif

#include "lua.hpp"

using namespace std ;

void Monitor::CreateSocketServer(MonitorConfiguration *config)
{
	static SocketServer socketServer(config,0) ;

	static SocketServer fms32ProServer(config,1000) ;
	fms32ProServer.m_ServerModus = SocketThread::fms32pro ;

	static SocketServer crusaderServer(config,2000) ;
	crusaderServer.m_ServerModus = SocketThread::crusader ;

	socketServer.Start() ;
	fms32ProServer.Start() ;
	crusaderServer.Start() ;
}


void Monitor::Initialize (int argc, char *argv[])
{
   m_bWantStop=false;
   if ( memLockCreate( 12345, & s) < 0) {
   		ThrowMonitorException("memLockCreate failed") ;
    }

	m_MonitorConfig.ParseCommandline(argc,argv) ;
 	m_MonitorConfig.ReadConfiguration(m_MonitorConfig.m_ConfigFile) ;
	m_MonitorConfig.ParseCommandline(argc,argv) ;
}

void initFileLogging(MonitorConfiguration *config)
{

	if (!(config->m_sLogfile=="screen"))
	{
		FILE* pFile = fopen(config->m_sLogfile.c_str(), "a");
		Output2FILE::Stream() = pFile;
		FILE_LOG(logINFO) << "monitord restarted - logging with loglevel " << config->m_sLoglevel;
	} else {
		FILE_LOG(logINFO) << "Logging with loglevel " << config->m_sLoglevel;
	}

	FILELog::ReportingLevel() = FILELog::FromString(config->m_sLoglevel);
}

int main(int argc, char** argv)
{
	Monitor m_monitor;
	try {
		m_monitor.Initialize (argc, argv);
		initFileLogging(&m_monitor.m_MonitorConfig) ;
		m_monitor.m_SignalStopped = new MonitorBlockingSignal();
		GlobalDispatcher = new MonitorResultsDispatcher();
	#ifdef WIN32
		/**
		 * Soll als Dienst ausgef�hrt werden ?
		 */

		try
		{
			if (m_monitor.m_MonitorConfig.m_service_uninstall == true) {
				FILE_LOG(logINFO) << PACKAGE_NAME << " wird als Dienst entfernt.";
				/* uninstall service from service control daemon */
				MonitorService *m_MonitorService = new MonitorService(&m_monitor);
				m_MonitorService->UnInstallService();
				delete m_MonitorService;
			} else if (m_monitor.m_MonitorConfig.m_service_install == true) {
				FILE_LOG(logINFO) << PACKAGE_NAME << " wird als Dienst eingerichtet.";
				/* install service in service control daemon */
				MonitorService *m_MonitorService = new MonitorService(&m_monitor);
				m_MonitorService->InstallService();
				delete m_MonitorService ;
			} else {
				if (m_monitor.m_MonitorConfig.m_service_run == true) {
					FILE_LOG(logINFO) << PACKAGE_NAME << " startet als Dienst.";
					/* running monitor as windows service application */
					MonitorService *m_MonitorService = new MonitorService(&m_monitor);
					m_MonitorService->Run ();
					delete m_MonitorService ;
				} else {
	#endif
				/* running monitor as command line application */
				FILE_LOG(logINFO) << PACKAGE_STRING << " READY" ;
				cout << PACKAGE_STRING << " running...\r\n";
				if (!(m_monitor.m_MonitorConfig.m_sLogfile == "screen")) {
					cout << "Logging in Logfiles, keine weiteren Ausgaben hier.";
				}
				m_monitor.MainLoop ();
	#ifdef WIN32
				}
			}
		} catch (MonitorServiceException(err))
		{
			// FIXME: Dienste koennen nicht auf die Console schreiben, da siehe
			// unsichtbar im Hintergrund laufen
			FILE_LOG(logERROR) << err.what() ;
		}
	#endif

	} catch (MonitorException(err))
	{
		FILE_LOG(logERROR) << err.what() ;
	}
}


void Monitor::MainLoop()
{
	// Soundkarte initialisieren
	InitSndCard() ;

	/********************************************************/


	static SocketServer socketServer(&m_MonitorConfig,m_MonitorConfig.m_socketFilterFileName ,0) ;
	socketServer.Start() ;
	FILE_LOG(logINFO) << "monitord socketserver started" ;


	static SocketServer fms32ProServer(&m_MonitorConfig,m_MonitorConfig.m_socketFilterFileName ,1000) ;
	fms32ProServer.m_ServerModus=SocketThread::fms32pro ;
	fms32ProServer.Start() ;
	FILE_LOG(logINFO) << "fms32pro socketserver started" ;

	static SocketServer crusaderServer(&m_MonitorConfig,m_MonitorConfig.m_socketFilterFileName ,2000) ;
	crusaderServer.m_ServerModus=SocketThread::crusader ;
	crusaderServer.Start() ;
	FILE_LOG(logINFO) << "crusader socketserver started" ;


	/*******************************************************/

	#ifdef PLUGINS
	//GetPluginsManager().loadPlugin("plugins/.libs/libmplugin_mysql-0.dll",NULL);
	GetPluginsManager().loadScriptFilter(m_MonitorConfig.m_pluginFilterFileName) ;
	GetPluginsManager().loadPluginsFromConfigNode(&m_MonitorConfig.m_configDataPlugins);
	FILE_LOG(logDEBUG) << "PluginManager started" ;
	#endif
	/*********************************************************/

	while (!m_bWantStop)
	{
		/**
		 * Wer sich fragt, wo eigentlich denn die Arbeit gemacht wird:
		 * Die drei SocketServer sind eigenstaendige Threads. Die bedienen
		 * die TCP/IP Verbindungen und laufen unabhaegig.
		 *
		 * Die eigentliche (Ton) Auswertung erfolgt in jeweils einem Thread
		 * pro Soundkarte. Diese Threads werden im InitSndCard gestartet.
		 *
		 * Dann gibt es noch den GlobalDispatcher. Er ist auch ein eigenstaendiger
		 * Thread. Er wird von dem Auswerten mit ResultSets versorgt. Die Auswerter
		 * haben damit Ihren Teil erledigt.
		 * Der Dispatcher verteilt dann die Results an alle Sockets und Plugins (ohne die
		 * Auswerter zu blockieren)
		 *
		 */

		usleep(100);
		// Wie man sieht: hier gibt es im Moment nichts zu tun :-
	}

	FILE_LOG(logINFO) << PACKAGE_NAME << " shutting down..."  ;
	StopSndCard() ;

	FILE_LOG(logINFO) << "stopping socketserver monitord";
	socketServer.m_bWantStop=true ;
	FILE_LOG(logINFO) << "stopping socketserver FMS32";
	fms32ProServer.m_bWantStop=true ;
	FILE_LOG(logINFO) << "stopping socketserver Crusader";
	crusaderServer.m_bWantStop=true ;

	usleep(1000) ;
	m_SignalStopped->SetSignal() ;
	usleep(500) ;
	FILE_LOG(logINFO) << "all done. " << PACKAGE_NAME << " exiting";
}

void Monitor::InitSndCard()
{
	unsigned int cardnum;
	for (cardnum=0;cardnum<4;cardnum++)
	{
		if (m_MonitorConfig.m_sndConfig[cardnum].iAktiv==1)
		{
			m_sndIn[cardnum] = new CSndPipe();
			FILE_LOG(logINFO) << "starting soundcard #" << cardnum  ;
			m_sndIn[cardnum]->initDecoderModules(cardnum,&m_MonitorConfig) ;
			m_sndIn[cardnum]->m_SoundIn.setDevice(m_MonitorConfig.m_sndConfig[cardnum].sDevice, 22050) ;

			#ifdef PLUGINS
			  m_sndIn[cardnum]->loadPlugins(&m_MonitorConfig, m_MonitorConfig.m_sndConfig[cardnum].configChannel[0],m_MonitorConfig.m_sndConfig[cardnum].configChannel[1]) ;
			#endif
			m_sndIn[cardnum]->m_SoundIn.Start() ;
			FILE_LOG(logINFO) << "Soundcard #" << cardnum << " started - complete"  ;
		}
	}
}


void Monitor::StopSndCard()
{
	unsigned int cardnum;
	for (cardnum=0;cardnum<4;cardnum++)
	{
		if (m_MonitorConfig.m_sndConfig[cardnum].iAktiv==1)
		{
			FILE_LOG(logINFO) << "stopping soundcard# " << cardnum  ;
			m_sndIn[cardnum]->m_SoundIn.Stop() ;
			FILE_LOG(logINFO) << "soundcard #" << cardnum<< " halted."  ;
			delete m_sndIn[cardnum];
		}
	}
}
