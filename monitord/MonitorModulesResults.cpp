#include <iostream>

#include "MonitorModulesResults.h"
#include "MonitorExceptions.h"
#include "SocketServer.h"
#include "MonitorLogging.h"

#ifdef PLUGINS
#include "PluginThread.h"
#endif

using namespace std;

#ifdef WIN32
	#define sleep Sleep
#endif

MODULERESULTSET ModuleResultSet ;
MonitorResultsDispatcher *GlobalDispatcher;


ModuleResultBase::ModuleResultBase()
{
}

ModuleResultBase::~ModuleResultBase()
{
	m_Items.clear() ;
}

void ModuleResultBase::copyTo(ModuleResultBase & target)
{
	target.m_Items = m_Items ;
}


ostream& operator<<(ostream & os, ModuleResultBase & m)
{
 for (ResultItemsMap::iterator iter=m.m_Items.begin(); iter!=m.m_Items.end(); ++iter)
	 {
        os << iter->first << " = \"" << iter->second << "\"" << endl ;
	 }
	 return os ;
}

//----------------------------------------------------------------
// MonitorResultsDispatcher
//
MonitorResultsDispatcher::MonitorResultsDispatcher()
{
	FILE_LOG(logDEBUG) << "Dispatcher startet"  ;

	if ( memLockCreate( 12346, & m_MemLock) < 0) {
   		ThrowMonitorException("Dispatcher: memLockCreate failed") ;
    }

     Start() ;
    m_bStop=false ;
}

MonitorResultsDispatcher::~MonitorResultsDispatcher()
{
	 memLockDestroy(m_MemLock) ;
}

bool MonitorResultsDispatcher::addResult(ModuleResultBase* pResult)
{
	// Neues Element in die Liste einfuegen
	// ACHTUNG: dieses Kommando wird im Kontext des Dekoders ausgeführt
	//          Somit könnte der Thread des Dispatchers gerade die Liste
	//			Löschen/bearbeiten wollen. Deshalb erst das Lock setzen
	//

	memLock(m_MemLock) ;
	m_Results.push_back(pResult) ;
	m_Signal.SetSignal() ;
	memUnlock(m_MemLock) ;
	return true ;
}

void* MonitorResultsDispatcher::Thread()
{
	ModuleResultBase *pRes ;

	JThread::ThreadStarted();

    /*
     * All Lua contexts are held in this structure. We work with it almost
     * all the time.
     */

	do
	{
		FILE_LOG(logDEBUG1) << "Dispatcher waiting" ;
		m_Signal.WaitForSignal() ; // Auf neue Daten warten ...
		FILE_LOG(logDEBUG1) << "Dispatcher running" ;

		while (m_Results.size()>0)
		{
			m_bSkipDispatching=false ;
			memLock(m_MemLock) ;
			for (MODULERESULTSET::iterator i=m_Results.begin(); i<m_Results.end();i++)
			{
				FILE_LOG(logDEBUG1) << "bearbeite ResultSet aus GlobalDispatcher Queue" ;
				// Daten holen, verteilen, aus Queue loeschen
				pRes=*i ;


				// Ergebnis an alle SocketServer & Plugins verteilen
				if (m_bSkipDispatching==false)
				{
					GetSocketsManager()->dispatchResult(pRes) ;
					#ifdef PLUGINS
					GetPluginsManager().dispatchResult(pRes) ;
					#endif
				}
				// Eintrag aus der Warteschlange loeschen
				FILE_LOG(logDEBUG4) << "loesche ResultSet aus GlobalDispatcher Queue" ;
				m_Results.erase(i) ;
				FILE_LOG(logDEBUG4) << "delete pRes" ;
				delete pRes ;
				FILE_LOG(logDEBUG4) << "delete pRes:done" ;
			}
			memUnlock(m_MemLock) ;
		}
		m_Signal.ResetSignal() ;

	} while (m_bStop==false) ;

	return NULL ;
}

