#include "SocketServer.h"
#include "SocketThreadMonitord.h"
#include "SocketThreadFMS32.h"
#include "SocketThreadCrusader.h"

#ifndef WIN32
	#include <sys/param.h>
	#include <arpa/inet.h> // fuer inet_ntoa
#else
	#define usleep Sleep
	#define socklen_t int
#endif

#include "memlock.h"
#include "MonitorModulesResults.h"
#include "base64.h"
#include "convert.h"
#include "Monitor.h"
#include "MonitorLogging.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using namespace std;

// Vielleicht sollte man das hier spaeter mal auslagern ...
// TODO Auslagern in externe Dateien (.h, .cpp)-Paar
//

const int SERVER_PORT = 9333 ;

typedef struct {long tv_sec; long tv_usec;} timval;




SocketServer::SocketServer(MonitorConfiguration *config, int iLockStartwert)
{
	m_MonitorConfiguration=config ;
	m_ServerModus=SocketThread::monitord ;
	m_iLockStartwert=iLockStartwert ;
	m_bWantStop=false ;
}


SocketServer::SocketServer(MonitorConfiguration *config, std::string FilterFileName, int iLockStartwert)
{
	m_MonitorConfiguration=config ;
	m_ServerModus=SocketThread::monitord ;
	m_iLockStartwert=iLockStartwert ;
	m_bWantStop=false ;
        #ifdef LUA
	m_bUseLUAScript=false ;
        #endif

	if (! FilterFileName.empty()) {

		#ifdef LUA
			// LUA TEST
			try
			{
				L = lua_open() ;
				luaL_openlibs(L) ;


				if(luaL_loadfile(L, FilterFileName.c_str()))
				{
						throw std::string(std::string(lua_tostring(L, -1)));
				}

				if (lua_pcall(L, 0, 0, 0))
				{
					FILE_LOG(logERROR) << "LUA test fehlgeschlagen" << endl ;
				}

				m_bUseLUAScript=true ;
				FILE_LOG(logINFO) << "Successfully loaded LUA filter: " << FilterFileName ;
			}
			catch (const std::string &e)
			{
				FILE_LOG(logERROR) << "Error loading lua script: "  << e;
			}

		#endif
	}

}


SocketServer::~SocketServer()
{
	// Beim SocketManager abmelden
	GetSocketsManager()->removeModule(this);

	#ifdef LUA
		if (L!=NULL)
		{
			lua_close(L) ;
		}
	#endif
}


bool SocketServer::createListeningSocket()
{
		bool result=false;
#ifdef _WIN32
    /* Initialisiere TCP fuer Windows ("winsock") */
    short wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD (1, 1);
    if (WSAStartup (wVersionRequested, &wsaData) != 0) {
    	ThrowMonitorException("Failed to init windows sockets (WSAStartup)") ;
        //fprintf( stderr, "Failed to init windows sockets\n");
        //exit(1);
    }
#endif

    /* Erzeuge das Socket */
    m_sock = socket( PF_INET, SOCK_STREAM, 0);
    if (m_sock < 0) {
    	ThrowMonitorException("Failed to create listening socket") ;
        //perror( "failed to create socket");
        //exit(1);
    }


	switch (m_ServerModus)
	{
	case SocketThread::fms32pro:
		m_iPort=m_MonitorConfiguration->m_PortFMS32Pro ;
		break ;
	case SocketThread::crusader:
		m_iPort=m_MonitorConfiguration->m_PortCrusader ;
		break ;
	case SocketThread::monitord:
	default:
		m_iPort=m_MonitorConfiguration->m_Port ;
		break ;
	}

	if ( (m_iPort<=0) && (m_iPort >65535))
	{
		if (m_ServerModus==SocketThread::monitord)
		{
			m_iPort=SERVER_PORT ;
		} else {
			m_iPort=0 ;
		}
	}
    /* Erzeuge die Socketadresse des Servers
     * Sie besteht aus Typ und Portnummer */
    memset( &m_server, 0, sizeof (m_server));
    m_server.sin_family = AF_INET;

    if (m_MonitorConfiguration->m_BindIP=="")
    {
    	m_server.sin_addr.s_addr = htonl( INADDR_ANY);
    } else {
    	m_server.sin_addr.s_addr = (inet_addr(m_MonitorConfiguration->m_BindIP.c_str()));
    }
    m_server.sin_port = htons( m_iPort);

	int on = 1 ;
#ifndef WIN32
    setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) ;
#else
	setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,(char*) &on,sizeof(on)) ;
#endif

    /* Erzeuge die Bindung an die Serveradresse
     * (d.h. an einen bestimmten Port) */
    if (bind( m_sock, (struct sockaddr*)&m_server, sizeof( m_server)) < 0) {
		// Verbindungsfehler ignorieren, damit die Nachrichten auf jeden Fall
		// aus dem ModuleResultSet "gepumpt" werden !
		//
        //perror( "can't bind socket");
        //exit(1);
	} else {
		/* Teile dem Socket mit, dass Verbindungswuensche
		 * von Clients entgegengenommen werden */
		listen( m_sock, 10);
		result=true ;

		/**
		 * In globaler ManagerListe eintragen/anmelden
		 */
		GetSocketsManager()->addModule(this) ;
	}


	return result ;
}

void *SocketServer::Thread()
{
	this->ThreadStarted() ; // Erstmal Bescheid geben, dass wir laufen

	if (createListeningSocket())
	{
		initSocketThreads() ;
	}


	if (memLockOpen( 12345, & m_queueLock) < 0) {
		ThrowMonitorException("memLockOpen failed") ;
    }

	while (!m_bWantStop)
	{
	    /* Bearbeite die Verbindungswünsche von Clients
	     * in einer Endlosschleife
	     * Der Aufruf von accept() blockiert solange,
	     * bis ein Client Verbindung aufnimmt, oder der timeout abgelaufen ist */

	   	fd_set fdset ;
	    timeval tv ;
	    int result=0 ;
	    int maxHandle ;
	    m_bWantStop=false ;
	    while ((result==0) && (!m_bWantStop))
	    {
	    	tv.tv_sec=0; // Wartezeit auf Socketereignis, danach Kontrolle, ob ein "Sendeauftrag" vorliegt
	    	tv.tv_usec=100000 ;
	    	FD_ZERO(&fdset) ;   // Den Socket m_sock ueberwachen ...
	   		FD_SET(m_sock,&fdset);

	   		// Diese Zeile sollte man NICHT mit "1" uebersetzen !
	   		// Das geht voll daneben. Es muss m_sock+1 sein ! Auch bei einem (!) Socket !
	   		maxHandle = m_sock + 1;

	    	result= select(maxHandle, &fdset, &fdset, &fdset, &tv); // wartet Zeitraum wie in tv definiert

			if (FD_ISSET(m_sock,&fdset)>0) // Socket ereignis ?
			{
				result=1 ;
			}

			// Neue Verbindungsanfrage ?
			// Dann annehmen und als eigenen Thread laufen lassen
			//
			if (result>0)
			{
				int useSocket=-1 ;

				// Jetzt freien SocketThread suchen
				for (int i=0;i<MAX_CLIENTS;++i)
				{
					if (!socketThread[i]->IsRunning())
					{
						useSocket=i ;
						i=MAX_CLIENTS ;
					}
				}

				if ((useSocket>=MAX_CLIENTS) || (useSocket<0))
				{
					// TODO zu viele Clients !
					ThrowMonitorException("Too many client connects") ;
					perror("Zu viele Clients !\r\n") ;
					exit (1) ;
				}

				socklen_t sin_size = sizeof (sockaddr_in);
				int fd = accept(m_sock,(sockaddr*) &socketThread[useSocket]->m_client,&sin_size);
				FILE_LOG(logINFO) << "new connection from " << inet_ntoa(socketThread[useSocket]->m_client.sin_addr) ;

				// Thread mit dem oben angenommenen Socket starten
				socketThread[useSocket]->setFD(fd) ;
				socketThread[useSocket]->Start() ;
			}
		}
	}

	// Listening Socket beenden
	closesocket( m_sock);

	// Alle Clients beenden
	FILE_LOG(logINFO) << "Beende alle Clients"  ;
	for (int i=0;i<MAX_CLIENTS;++i)
	{
		FILE_LOG(logDEBUG) << "beende client " << i  ;
		if (socketThread[i]->IsRunning())
		{
			socketThread[i]->closeSocket() ;
			FILE_LOG(logINFO) << i << ": closesocket done" ;
			socketThread[i]->Kill() ;
			FILE_LOG(logINFO) << i << ": kill done" ;
		}
	}

	// uns selbst beenden
	usleep(1000) ;
	//JThread::Kill() ;
	return NULL ;
}


void SocketServer::initSocketThreads()
{
	int i ;
	for (i=0;i<MAX_CLIENTS;i++)
	{
		switch (m_ServerModus)
		{
			case SocketThread::monitord:
				socketThread[i]=new SocketThreadMonitord(m_MonitorConfiguration, m_iLockStartwert+i,i+1) ;
				break ;
			case SocketThread::fms32pro:
				socketThread[i]=new SocketThreadFMS32(m_MonitorConfiguration,m_iLockStartwert+i,i+1) ;
				break ;
			case SocketThread::crusader:
				socketThread[i]=new SocketThreadCrusader(m_MonitorConfiguration,m_iLockStartwert+i,i+1) ;
				break;
			default:
				break ;
		}
	}
}


void SocketServer::addResult(ModuleResultBase* pRes)
{
	// Nun f?r die nachgeordneten Threads die Ausgabe nach den Uebergabewerte zusammenbauen
	//
	#ifdef LUA
		char eins[255],zwei[255] ;
	#endif

	// Wenn ja, Ereignis an alle Threads weitergeben
	for (int i=0;i<MAX_CLIENTS;++i)
	{
		if (socketThread[i]->IsRunning())
		{
			m_bSkipDispatching=false ;
			// erstmal ggf. LUA Filter aufrufen
				#ifdef LUA

				if (m_bUseLUAScript==true)
				{
					int z ;
				      /* push functions and arguments */
				     lua_getglobal(L, "filter");  /* function to be called */
				     // start array structure
					   lua_newtable( L );
					   int numCount=1 ;
						for (ResultItemsMap::iterator iter=pRes->m_Items.begin(); iter!=pRes->m_Items.end(); ++iter)
		 				{
		 				  {
						   memset(eins,0,200) ;
						   memset(zwei,0,200) ;
						   strncpy(eins,iter->first.c_str(),199) ;
						   strncpy(zwei,iter->second.c_str(),199) ;

						   lua_pushstring( L, eins );
						   lua_pushstring( L, zwei );
						   lua_rawset( L, -3 );
						   numCount++ ;
		 				  }
		 				}

		 				// noch clientIP, authenticated und loginname dazupacken

		 				// Authenticated
		 				memset(eins,0,200) ;
						memset(zwei,0,200) ;
						strncpy(eins,"client_authenticated",199) ;
						if (socketThread[i]->isClientAuthenticated())
						{
							strncpy(zwei,"1",199) ;
						} else {
							strncpy(zwei,"0",199) ;
						}
						lua_pushstring( L, eins );
						lua_pushstring( L, zwei );
						lua_rawset( L, -3 );
						numCount++ ;

		 				// ClientIP
		 				memset(eins,0,200) ;
						memset(zwei,0,200) ;
						strncpy(eins,"client_ip",199) ;
						strncpy(zwei,socketThread[i]->getClientIP().c_str(),199) ;
						lua_pushstring( L, eins );
						lua_pushstring( L, zwei );
						lua_rawset( L, -3 );
						numCount++ ;

		 				// Loginname
		 				memset(eins,0,200) ;
						memset(zwei,0,200) ;
						strncpy(eins,"client_loginname",199) ;
						strncpy(zwei,socketThread[i]->getClientLogin().c_str(),199) ;
						lua_pushstring( L, eins );
						lua_pushstring( L, zwei );
						lua_rawset( L, -3 );
						numCount++ ;

		 				// Clienttype
		 				memset(eins,0,200) ;
						memset(zwei,0,200) ;
						strncpy(eins,"client_type",199) ;
						switch (m_ServerModus)
						{
							case SocketThread::monitord:
								strncpy(zwei,"monitord",199) ;
								break ;
							case SocketThread::fms32pro:
								strncpy(zwei,"fms32",199) ;
								break ;
							case SocketThread::crusader:
								strncpy(zwei,"crusader",199) ;
								break ;
							default:
								strncpy(zwei,"unknown",199) ;
								break ;
						}
						lua_pushstring( L, eins );
						lua_pushstring( L, zwei );
						lua_rawset( L, -3 );
						numCount++ ;

					   // set the number of elements (index to the last array element)
					   lua_pushliteral( L, "n" );
					   lua_pushnumber( L, numCount-1 );
					   lua_rawset( L, -3 );

					   // set the name of the array that the script will access
					   lua_setglobal( L, "arg" );

				      /* do the call (2 arguments, 1 result) */
				      if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
				      	FILE_LOG(logERROR) << "Fehler beim Aufruf lua dispatcher script:" << lua_tostring(L, -1);
				        //error(L, "error running function `f': %s",
				        //         lua_tostring(L, -1));
					  }

				      /* retrieve result */
				      if (!lua_isnumber(L, -1)) {
				      	FILE_LOG(logERROR) << "nicht-numerische Antwort vom lua dispatcher script" ;
				        //error(L, "function `f' must return a number");
					  }
				      z = lua_tonumber(L, -1);
				      lua_pop(L, 1);  /* pop returned value */
					  FILE_LOG(logDEBUG1) << "lua Result (global dispatcher)" << z ;

					  if (z==1) m_bSkipDispatching=true ;
				}
				#endif

				if (m_bSkipDispatching==false)
				{
					socketThread[i]->addResult(pRes) ;
				}
		}

	}
}


/*
 * -----------------------------------------------------------
 * ThreadBase
 * -----------------------------------------------------------
 */

ThreadBase::~ThreadBase()
{
}

ThreadBase::ThreadBase(int locknum)
{
	m_iLockNum= locknum ;
	m_exitThread=false ;
}

bool ThreadBase::createLock()
{
	if ( memLockCreate( m_iLockNum, &m_Lock) < 0) {

		ThrowMonitorException("memLockCreate failed for " + convertIntToString(m_iLockNum)) ;
    }
    return true;
}

void ThreadBase::releaseLock()
{
	 memLockDestroy(m_Lock) ;

}


/*
 * -----------------------------------------------------------
 * SocketThread
 * -----------------------------------------------------------
 */

SocketThread::~SocketThread()
{
}

SocketThread::SocketThread(MonitorConfiguration *config, int locknum, int PortNum, SocketMode ServerMode)
: ThreadBase(locknum)
{
	m_MonitorConfiguration = config;
	m_iPortNum = PortNum ;
	m_ServerMode = ServerMode ;
	ResetThreadVars() ;
}

void SocketThread::setFD(int fd)
{
	m_fd=fd ;
}

void SocketThread::ResetThreadVars()
{
	// Zuruecksetzen:
	// Kein Socket zugewiesen
	// Client ist nicht angemeldet
	// Thread soll sich nicht beenden
	//

	m_fd=0 ;
	m_authenticated=false ;
	m_exitThread=false ;

	// Commandbuffer loeschen
	memset(m_CommandBuffer,0,MAX_COMMANDLINE) ;

}

void *SocketThread::Thread()
{
	if (m_fd==0)
	{
		ThrowMonitorException("SocketThread started wird fd=NULL") ;
		return NULL ;
	}

	this->ThreadStarted() ; // Erstmal Bescheid geben, dass wir laufen
	createLock() ;
	createSocket() ;

	FILE_LOG(logINFO) << "SocketThreads exits" ;

	releaseLock() ;
	return NULL;
}


std::string SocketThread::createZVEIOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	return socketText ;
}


std::string SocketThread::createPOCSAGOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;
	return socketText ;
}

std::string SocketThread::createFMSOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;

	return socketText ;
}

std::string SocketThread::createOutputString(ModuleResultBase Result)
{
	std::string socketText="" ;

	if (Result["typ"]=="fms")
	{
		socketText=createFMSOutputString(Result) ;
	}
	else if (Result["typ"]== "pocsag")
	{
		socketText=createPOCSAGOutputString(Result) ;
	}
	else if (Result["typ"]=="zvei")
	{
		socketText=createZVEIOutputString(Result) ;
	}

	return socketText ;
}
void SocketThread::addResult(ModuleResultBase* pRes)
{
	std::string outText ;

	outText=createOutputString(*pRes);
	addOutputText(outText) ;
}

void SocketThread::addOutputText(std::string outText)
{
	memLock(m_Lock) ;
	if (!(outText==""))
	{
		m_outputStrings.insert(m_outputStrings.begin(),outText) ;
	}
	memUnlock(m_Lock) ;
}

void SocketThread::say(const std::string& something)
{
	unsigned int len=send( m_fd, something.c_str(), something.length(), 0);
	if (len!=something.length())
	{
		FILE_LOG(logERROR) << "error sending date to client. thread exiting"  ;
		doLogout() ;
	}
}

void SocketThread::say(const char *something)
{
	unsigned int len = send( m_fd, something, strlen(something), 0);
	if (len!=strlen(something))
	{
		FILE_LOG(logERROR) << "error sending date to client. thread exiting"  ;
		doLogout() ;
	}
}

void SocketThread::processInput()
{

}


void SocketThread::doLogout()
{
	m_exitThread=true ;
}


void SocketThread::sayWelcome()
{
}

void SocketThread::sayGoodbye()
{
}

void SocketThread::createSocket()
{
	timeval tv ;

    if (m_fd < 0) {
    	ThrowMonitorException("accept failed") ;
    	//perror( "accept failed");
        //exit(1);
    }

	// Hier ist die Sitzung noch nicht "authenticated"
	fd_set fdset ;

    FD_ZERO(&fdset) ;
    FD_SET(m_fd,&fdset);

    m_sClientIP= inet_ntoa(m_client.sin_addr) ;

	m_authenticated=false ;
	// Gültige IP Adresse, die sich nicht anmelden muss ?
	if (m_MonitorConfiguration->IsValidLogin("","",this->m_sClientIP))
	{
		FILE_LOG(logINFO) << "login authentication (ip allowed): " << m_sClientIP ;
		this->m_authenticated=true ;
	}

	// FMS32Pro ist dafuer zu doof ;-)
	//
	if ((m_ServerMode==SocketThread::fms32pro))
	{
		// TODO: IP Liste auch bei FMS32 anwenden ? dann nachfolgendes auskommentiert lassen
		// this->m_authenticated=true ;
	}

	m_CommandBuffer[0]='\0';

	// Wir sagen natuerlich brav "Guten Tag" ...
	sayWelcome() ;

	fd_set fdset_write ;
	fd_set fdset_exceptions ;


    for (;;) {
    	if (m_exitThread)
    	{
    		break ;
    	}

        // Fuer eine Sekunde auf Meldung am Port warten
        tv.tv_sec=0 ;
        tv.tv_usec=10000 ;
        FD_ZERO(&fdset) ;
        FD_SET(m_fd,&fdset);

        // Neu: write + exceptions
        FD_ZERO(&fdset_write) ;
    	FD_ZERO(&fdset_exceptions) ;
		//FD_SET(m_fd,&fdset_write);
    	FD_SET(m_fd,&fdset_exceptions);


        int result = select(1, &fdset, NULL, &fdset_exceptions, &tv);

		if (FD_ISSET(m_fd,&fdset)>0) // Socket ereignis ?
		{
			FILE_LOG(logDEBUG) << "Socket reports read event" ;
			result=1 ;
		}

		if (FD_ISSET(m_fd,&fdset_write)>0) // Problem ?
		{
			FILE_LOG(logDEBUG) << "Socket reports write event" ;
			result=0 ;
			//m_exitThread=true ;
		}

		if (FD_ISSET(m_fd,&fdset_exceptions)>0) // Problem ?
		{
			FILE_LOG(logDEBUG) << "Socket reports exception event" ;
			result=0 ;
			m_exitThread=true ;
		}

        if (result>0)
        {
        	char buffer[RECV_BUFFER] ;
			char * posPtr ;
			int gelesen ;
			gelesen=recv (m_fd, buffer, RECV_BUFFER-1, 0) ;
        	if (gelesen<=0) // Nix am Port, aber doch Port Event ?
			{
				FILE_LOG(logINFO) << "recv()<=0 => socketthread exiting"  ;
				m_exitThread=true ;
			}
			buffer[gelesen]='\0';

			/* gelesenen Puffer an evtl. vorhanden Reste im Kommandopuffer anhängen */
        	strncat(m_CommandBuffer,buffer,MAX_COMMANDLINE-strlen(m_CommandBuffer)-1) ;

			/* Erst wenn mindestens eine Eingabezeile empfangen wurde, werden alle vollständigen Zeilen ausgewertet */
			while ((posPtr=strstr(m_CommandBuffer,"\r\n"))>0)
			{
				/* Puffer nach dem CRLF zwischenspeichern */
				char tempbuffer[MAX_COMMANDLINE] ;
				tempbuffer[0]='\0';
				strncpy(tempbuffer,&posPtr[2],MAX_COMMANDLINE) ;

				/* Kommando mit dem Zeilenende beenden */
				posPtr[0]='\0';

				processInput() ;

				/* restlichen Puffer nach dem CRLF wiederherstellen */
				strncpy(m_CommandBuffer,tempbuffer,MAX_COMMANDLINE) ;
			}

        } else
        {
     		// Nix tun
        }

        // Gibt es etwas zu senden ?
        //
		memLock(m_Lock) ;
		while (!m_outputStrings.empty())
		{
			std::string retString=m_outputStrings.back() ;

			if (m_authenticated)
			{
				retString.push_back('\r') ;
				retString.push_back('\n') ;
				say(retString) 	;
			}
			m_outputStrings.pop_back() ;
		}
		memUnlock(m_Lock) ;
    }

    closesocket(m_fd);
    ResetThreadVars() ;
}

std::string SocketThread::getClientIP()
{
	return (m_sClientIP) ;
}

std::string SocketThread::getClientLogin()
{
	return (m_loginname) ;
}

bool SocketThread::isClientAuthenticated()
{
	return (m_authenticated) ;
}

void SocketThread::closeSocket()
{
	closesocket( m_fd);
}


bool SocketThread::paramIsHex(int param)
{
	// TODO
	return true ;
}

bool SocketThread::paramIsBase64(int param)
{
	// TODO
	return true ;
}


bool SocketThread::HexToString(int param, std::string &result)
{
	return convertHexToString(m_cmdParam[param],result);
}

// ----------------------------------------
//MonitorSocketsManager

MonitorSocketsManager *GlobalMonitorSocketsManager=NULL;
MonitorSocketsManager* GetSocketsManager() {
	if (GlobalMonitorSocketsManager==NULL) {
		GlobalMonitorSocketsManager = new MonitorSocketsManager();
	}
	return (GlobalMonitorSocketsManager);
}

MonitorSocketsManager::MonitorSocketsManager()
{
	FILE_LOG(logDEBUG) << "SocketManager erstellt"  ;

	if ( memLockCreate( 12347, & m_MemLock) < 0) {
   		ThrowMonitorException("SocketsManager: memLockCreate failed") ;
    }

    m_bStop=false ;
}


MonitorSocketsManager::~MonitorSocketsManager()
{
}


bool MonitorSocketsManager::addModule(SocketServer* pServer)
{
	m_Modules.push_back(pServer) ;

	return true ;
}


bool MonitorSocketsManager::dispatchResult(ModuleResultBase *pRes)
{
	tMonitorSocketServerVector::iterator i ;

	for (i= m_Modules.begin(); i< m_Modules.end(); i++)
	{
			(*i)->addResult(pRes) ;
	}
	return true ;
}


bool MonitorSocketsManager::removeModule(SocketServer* pServer)
{
	return true ;
}
