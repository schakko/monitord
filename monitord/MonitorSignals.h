#ifndef MONSIGNALS_H_
#define MONSIGNALS_H_ 1

#include "MonitorExceptions.h"
#include "memlock.h"

#if (defined(WIN32) || defined(_WIN32_WCE))
	#ifndef _WIN32_WCE
		#include <process.h>
	#endif // _WIN32_WCE
	#include <winsock.h>
	#include <windows.h>
	//#define JMUTEX_CRITICALSECTION
#else // using pthread
	#include <pthread.h>
#endif // WIN32


class MonitorBlockingSignal {
public:
	MonitorBlockingSignal() ;
	virtual ~MonitorBlockingSignal() ;
	void WaitForSignal() ;
	void SetSignal() ;
	void ResetSignal() ;

private:
#ifdef WIN32
	HANDLE		m_Handle;
#else
	pthread_cond_t m_ptCondition ;
	pthread_mutex_t m_ptLock ;
#endif
} ;

#endif /*MONSIGNALS_H_*/
