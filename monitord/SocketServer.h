#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include <jthread-1.2.1/src/jthread.h>
#include "MonitorModulesResults.h"
#include "MonitorConfiguration.h"
#include "memlock.h"

#include "MonitorSignals.h"
#ifdef PLUGINS
	#include "plugins/mplugin.h"
#endif

#ifdef _WIN32
	/* Headerfiles für Windows */
	#include <winsock2.h>
	#include <io.h>
#else
	/* Headerfiles für Unix/Linux */
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#define closesocket(s) close(s)
#endif

/* sofern lua aktiviert ist ? -> ifdef noetig ? */
#include "lua.hpp"

#define MAX_CLIENTS 50
#define RECV_BUFFER 512
#define MAX_COMMANDLINE 1024
#define MAX_PARAMS 9


class ThreadBase : public JThread
{
public:
	ThreadBase(int LOCKNUM);
	virtual ~ThreadBase();
	virtual void *Thread() = 0 ;
	virtual void addResult(ModuleResultBase* pRes)=0 ;

	MonitorConfiguration *m_MonitorConfiguration ;
protected:
	virtual void ResetThreadVars(){} ;
	MEMLOCK m_Lock ;
	bool m_exitThread ;
	int m_iLockNum ;
	bool createLock() ;
	void releaseLock() ;
};


/**
 * @brief Repraesentiert immer genau einen verbunden Client.
 *
 * Der SocketThread wird fuer jeden verbundenen Client gestartet. Er prueft die Anmeldung
 * und gibt die Daten weiter, die vom Master-Thread SocketServer an ihn weitergeleitet werden
 */
class SocketThread : public ThreadBase
{
public:
	enum SocketMode {monitord,crusader,fms32pro} ;

	//SocketThread(int LOCKNUM, int PortNum);
	SocketThread(MonitorConfiguration *config, int LOCKNUM, int PortNum, SocketMode ServerMode=monitord);
	virtual ~SocketThread();
	virtual void *Thread() ;
	virtual void addResult(ModuleResultBase* pRes) ;
	void setFD(int fd) ;
	void addOutputText(std::string outText) ;
	void closeSocket() ;

	std::string getClientLogin() ;
	std::string getClientIP() ;
	bool isClientAuthenticated() ;
	struct sockaddr_in m_client; // war sockaddr_in

protected:
	MonitorConfiguration m_MonitorConfig;
	SocketMode m_ServerMode ;
	bool m_authenticated ;
	int m_fd;
	int m_iPortNum ;
	char m_CommandBuffer[MAX_COMMANDLINE] ;
	int m_cmd;
	int m_paramCount ;
	std::string m_cmdString;
	std::string m_cmdParam[MAX_PARAMS] ;
	std::string m_sClientIP ;
	std::vector<std::string> m_outputStrings ;
	std::string m_loginname ;

	virtual std::string createFMSOutputString(ModuleResultBase Result); //< Erstellt die Ausgaben fuer FMS
	virtual std::string createPOCSAGOutputString(ModuleResultBase Result); //< Erstellt die Ausgaben fuer POCSAG
	virtual std::string createZVEIOutputString(ModuleResultBase Result); //< Erstellt die Ausgaben fuer ZVEI
	std::string createOutputString(ModuleResultBase Result) ;

	virtual void ResetThreadVars() ;
	void say(const std::string& something) ;
	void say(const char* something) ;
	virtual void sayWelcome() ;
	virtual void sayGoodbye() ;
	void createSocket() ;
	virtual void processInput() ;

	// Kommandos auf reine HEX Characters pruefen
	//
	bool paramIsHex(int param) ;
	bool paramIsBase64(int param) ;
	bool HexToString(int param, std::string& result) ;
	bool StringToHex(const std::string& input, std::string &result) ;

	void doLogout() ; 	// 299
};

/**
 * @brief Oeffnet den TCP Socket und nimmt Verbindungsanfragen an.
 *
 * Der SocketServer oeffnet den listening port und startet fuer jeden Client, der sich
 * verbindet einen Thread vom Typ SocketThread.
 */

class SocketServer : public JThread
{
public:
	SocketServer(MonitorConfiguration* config, int iLockStartwert=0);
	SocketServer(MonitorConfiguration* config, std::string FilterFileName , int iLockStartwert=0);
	virtual ~SocketServer();
	virtual void *Thread() ;
	unsigned int m_iPort ;
	int m_iLockStartwert ;
	SocketThread::SocketMode m_ServerModus ;
	MEMLOCK m_queueLock ;
	MODULERESULTSET m_queue ;
	void addResult(ModuleResultBase* pRes) ;
	bool m_bWantStop ;
protected:
	int m_sock ;
	struct sockaddr_in m_server ;
	SocketThread* socketThread[MAX_CLIENTS] ;
	MonitorConfiguration *m_MonitorConfiguration ;
	bool createListeningSocket() ;
	void initSocketThreads() ;

	bool m_bSkipDispatching ;
	#ifdef LUA
		lua_State *L;
		bool m_bUseLUAScript ;
	#endif
} ;


/**
 * @brief Verwaltet alle SocketModule
 */

typedef std::vector< SocketServer*> tMonitorSocketServerVector ;

class MonitorSocketsManager
{
public:


	MonitorSocketsManager() ;
	virtual ~MonitorSocketsManager() ;
	bool addModule(SocketServer* pServer);
	bool removeModule(SocketServer *pServer) ;
	bool dispatchResult(ModuleResultBase *pRes) ;
protected:
	MEMLOCK m_MemLock ;
	tMonitorSocketServerVector m_Modules ;
	bool m_bStop ;
};

extern MonitorSocketsManager *GlobalMonitorSocketsManager ;
MonitorSocketsManager* GetSocketsManager()  ;

// in SocketServer.cpp als globale Variable definiert
//extern tMonitorSocketServerVector globalRegisteredSocketServers ;



#endif /*SOCKETSERVER_H_*/
