#ifndef MPLUGINAUDIO_H_
#define MPLUGINAUDIO_H_

#ifdef PLUGINS

#include "dll.h"
#include "../MonitorModulesResults.h"
#include "../MonitorConfiguration.h"
#include "../SocketServer.h"
#include "../xmltools.h"
#include <time.h>

#ifndef WIN32
	#define Sleep sleep
#endif

#define MAXAUDIOCLIENTS 20


struct strSocketMessage
{
		SocketThread* pThread ;
		std::string	message ;
}  ;

struct strClientInfo {
		SocketThread* pThread ;
		time_t time1 ;
		time_t time2 ;
		unsigned long customValue ;
		unsigned long customValue2 ;
		unsigned long info ;
		std::string infoText ;
};

typedef strSocketMessage SocketMessage ;
typedef strClientInfo ClientInfo ;

class MonitorAudioPlugIn
{
 public:
	MonitorAudioPlugIn() ;
	virtual ~MonitorAudioPlugIn() {} ;

	virtual bool InitAudioProcessing(class MonitorConfiguration* configPtr, XMLNode config, int channelNum) { return true ;} ;
    virtual void ProcessAudio(float *buffer, int length)=0 ;
	virtual bool QuitAudioProcessing() {return true;} ;
	virtual std::string DoCommand(std::string command, SocketThread* pClient=NULL)=0;
	virtual void Show() = 0;

	void addThreadMessage(SocketThread* pClient, std::string message) ;
	void addThreadMessage(int jobID, std::string message) ;
	bool getThreadMessage(SocketMessage & msg) ;

protected:
	ClientInfo m_pClient[MAXAUDIOCLIENTS] ;
	std::vector<SocketMessage*> m_SocketMessages ;
	std::string m_param[10] ;
	int m_paramCount ;
	bool m_bLockSocketMessages ;

	void broadcastMessage(std::string message);
	int addClient(SocketThread *pClient, unsigned long customValue=0,unsigned long info=0, std::string infoText="") ;
	void updateClient(int jobID, unsigned long customValue=0,unsigned long customValue2=0,unsigned long info=0, std::string infoText="",time_t time1=0, time_t time2=0) ;
	SocketThread* getClient(int jobID) ;
	unsigned long getCustomValue(int jobID) ;
	unsigned long getCustomValue2(int jobID) ;
	unsigned long getInfo(int jobID) ;
	time_t getTime1(int jobID) ;
	time_t getTime2(int jobID) ;
	std::string getInfoText (int jobID) ;
	int parseCommand(std::string command);
	void clearClient(int jobID) ;
};

class MonitorAudioPlugInFactory
{
 public:
	MonitorAudioPlugInFactory()
	{
	}

	virtual ~MonitorAudioPlugInFactory()
	{
	}

	virtual MonitorAudioPlugIn * CreatePlugIn() = 0;

};
#endif

#endif /*MPLUGINAUDIO_H_*/
