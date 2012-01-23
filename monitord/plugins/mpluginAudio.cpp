#include "mpluginAudio.h"

using namespace std ;

MonitorAudioPlugIn::MonitorAudioPlugIn()
{
	m_bLockSocketMessages=false;
	for (int i=0; i<MAXAUDIOCLIENTS;i++)
	{
		clearClient(i) ;
	}
}

void MonitorAudioPlugIn::addThreadMessage(SocketThread* pClient, std::string message)
{
	SocketMessage* pMsg=new SocketMessage ;
	pMsg->pThread=pClient ;
	pMsg->message=message ;

	while (m_bLockSocketMessages==true) Sleep(3) ;

	m_bLockSocketMessages=true ;
	m_SocketMessages.push_back(pMsg) ;
	m_bLockSocketMessages=false ;
}

void MonitorAudioPlugIn::addThreadMessage(int jobID, std::string message)
{
	if ((jobID<0) || (jobID>=MAXAUDIOCLIENTS))
	{
		ThrowMonitorException ("jobID nicht im erlaubten Bereich: " + convertIntToString(jobID)) ;
	} ;

	addThreadMessage (m_pClient[jobID].pThread,message) ;
}

bool MonitorAudioPlugIn::getThreadMessage(SocketMessage & msg)
{
	bool retVal=false ;

	while (m_bLockSocketMessages) Sleep(1) ;

	m_bLockSocketMessages=true ;

	SocketMessage* pMsg ;
	if (m_SocketMessages.size()>0)
	{
		pMsg=m_SocketMessages.back() ;
		m_SocketMessages.pop_back() ;

		msg.pThread=pMsg->pThread ;
		msg.message=pMsg->message ;

		delete pMsg ;
		retVal=true ;
	}
	m_bLockSocketMessages=false ;

	return retVal ;
}

void MonitorAudioPlugIn::broadcastMessage(std::string message)
{
	for (int i=0; i<MAXAUDIOCLIENTS;i++)
	{
		if (m_pClient[i].pThread)
		{
			cout << "sende an : " << i << endl;
			addThreadMessage(m_pClient[i].pThread,message) ;
		}
	}
}

int MonitorAudioPlugIn::addClient(SocketThread *pClient, unsigned long customValue,unsigned long info, std::string infoText)
{
	int i=0 ;
	bool bDuplicate=false;

	while (( m_pClient[i].pThread!=NULL) && (i<MAXAUDIOCLIENTS))
	{
		if (m_pClient[i].pThread==pClient)
		{
			bDuplicate=true ;
			break;
		}
		i++ ;
	}

	if ( (i<MAXAUDIOCLIENTS) && (!bDuplicate) )
	{
		if (pClient)
		{
			m_pClient[i].pThread=pClient ;
			updateClient(i,customValue,0,info,infoText) ;

			cout << "Threadliste: " << i << " hinzugef�gt" << endl ;
		}
	}

	if (i>=MAXAUDIOCLIENTS) i=-1 ;

	return i ;
}

void MonitorAudioPlugIn::updateClient(int jobID, unsigned long customValue,unsigned long customValue2,unsigned long info, std::string infoText, time_t time1,time_t time2)
{
	m_pClient[jobID].customValue=customValue ;
	m_pClient[jobID].customValue2=customValue2 ;
	m_pClient[jobID].info=info ;
	m_pClient[jobID].infoText=infoText;
	m_pClient[jobID].time1=time1 ;
	m_pClient[jobID].time2=time2 ;
}

unsigned long MonitorAudioPlugIn::getCustomValue(int jobID)
{
	return m_pClient[jobID].customValue ;
}

unsigned long MonitorAudioPlugIn::getCustomValue2(int jobID)
{
	return m_pClient[jobID].customValue2 ;
}
SocketThread* MonitorAudioPlugIn::getClient(int jobID)
{
	return m_pClient[jobID].pThread ;
}

unsigned long MonitorAudioPlugIn::getInfo(int jobID)
{
	return m_pClient[jobID].info ;
}

std::string MonitorAudioPlugIn::getInfoText(int jobID)
{
	return m_pClient[jobID].infoText ;
}

time_t MonitorAudioPlugIn::getTime1(int jobID)
{
	return m_pClient[jobID].time1;
}


time_t MonitorAudioPlugIn::getTime2(int jobID)
{
	return m_pClient[jobID].time2;
}

void MonitorAudioPlugIn::clearClient(int jobID)
{
	m_pClient[jobID].pThread=NULL ;
	m_pClient[jobID].info=0 ;
	m_pClient[jobID].infoText="" ;
	m_pClient[jobID].customValue=0 ;
}
int MonitorAudioPlugIn::parseCommand(std::string command)
{
	m_paramCount=0 ;

	for (int i=0;i<9;i++) m_param[i].clear() ;

	int pos ;
	unsigned int start=0 ;
	m_paramCount=0;

	pos=command.find(":",start) ;
	while ((pos>0) && (m_paramCount<9))
	{
		m_param[m_paramCount++]=command.substr(start,pos-start) ;
		cout << "ParamScan:" << m_paramCount-1 << ":" <<  m_param[m_paramCount-1] << endl ;
		start=pos+1 ;
		pos=command.find(":",start) ;
	}

	if (command.size()>start)
	{
		m_param[m_paramCount++]=command.substr(start,command.size()-start) ;
	}

return m_paramCount ;
}





