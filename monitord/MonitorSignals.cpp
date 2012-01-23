#include "MonitorSignals.h"
#include "MonitorLogging.h"

using namespace std;

MonitorBlockingSignal::MonitorBlockingSignal()
{
#ifdef WIN32
	m_Handle = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (m_Handle == NULL) {
		ThrowMonitorException("Win32 Eventhandle is null") ;
	}
#else
	int res=pthread_cond_init (&m_ptCondition,NULL) ;
	res+=pthread_mutex_init( &m_ptLock, NULL) ;
	res+=pthread_mutex_lock(&m_ptLock) ;
	if (res!=0)
	{
		ThrowMonitorException("error initialising pthreads Condition/Mutex") ;
	}
#endif
	LOG_DEBUG("Signal erstellt...") 
}

MonitorBlockingSignal::~MonitorBlockingSignal()
{
#ifdef WIN32
	if (m_Handle) CloseHandle(m_Handle) ;
#else
	pthread_cond_destroy (&m_ptCondition) ;
#endif
}

void MonitorBlockingSignal::ResetSignal()
{
#ifdef WIN32
	if (m_Handle)
		ResetEvent(m_Handle) ;
	else
		ThrowMonitorException("Win32 Eventhandle is null") ;
#else
	// pthread Gegenstueck ?
#endif
}

void MonitorBlockingSignal::SetSignal()
{
	LOG_DEBUG("Signal wird gesetzt")  
#ifdef WIN32
	if (m_Handle)
		SetEvent(m_Handle);
	else
		ThrowMonitorException("Win32 Eventhandle is null") ;
#else
	 pthread_cond_signal(&m_ptCondition) ;
#endif
}

void MonitorBlockingSignal::WaitForSignal()
{
	 LOG_DEBUG("Waiting for signal")
#ifdef WIN32
	if (m_Handle)
		WaitForSingleObject(m_Handle,INFINITE) ;
	else
		ThrowMonitorException("Win32 Eventhandle is null") ;
#else
	pthread_cond_wait(&m_ptCondition,&m_ptLock) ;
#endif
	LOG_DEBUG("Waiting for signal beendet")  
}
