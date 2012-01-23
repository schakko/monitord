/**
 * Windows Service Application f�r den Monitor, basierend auf
 * http://weblogs.asp.net/kennykerr/archive/2004/05/18/134342.aspx
 */

#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include "../convert.h"


#include "MonitorService.h"
#include "../MonitorLogging.h"

#ifdef WIN32
	#define sleep Sleep
#endif


using namespace std ;

MonitorService* MonitorService::m_service = 0;

MonitorService::MonitorService(Monitor* monitor)
{
    m_handle = 0;
    m_monitor = monitor;
    m_service = this;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    m_installServiceName="monitord" ;
    m_installServiceDisplayName="monitord" ;
}

MonitorService::~MonitorService()
{
}


void MonitorService::Stop()
{
	m_service->UpdateState(SERVICE_STOP_PENDING);
    m_monitor->m_bWantStop = true;
    m_service->m_debugFile << "Warte auf beenden des monitord" << endl ;
    m_monitor->m_SignalStopped->WaitForSignal() ;
    Sleep(2000) ;
    m_service->UpdateState(SERVICE_STOPPED);
    m_service->m_debugFile << "monitord Signal empfangen" << endl ;
    Sleep(1000) ;
}


/**
 * Provides the main entry point for an executable that
 * contains a single service. Loads the service into
 * memory so it can be started. This method blocks until
 * the service has stopped.
 */
void MonitorService::Run()
{
    assert (0 != m_service);

    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        // Even though the service name is ignored for own process services,
        // it must be a valid string and cannot be 0.
        { "", (LPSERVICE_MAIN_FUNCTION)ServiceMain },

        // Designates the end of table.
        { 0, 0 }
    };

    if (!::StartServiceCtrlDispatcher(serviceTable))
    {
		char msg[1024];
		snprintf (msg, 1024, "Fehler bei Dispatcher-Registrierung, siehe net helpmsg %ld", GetLastError());
    	ThrowMonitorServiceException(msg);
	}
}

/**
 * The starting point for the service.
 *
 * @param argc number of arguments in arguments
 * @param argv array of string pointers containing the arguments
 */
void WINAPI MonitorService::ServiceMain(DWORD argc, PSTR* argv)
{
    //
    // Since there's no way to inform the SCM of failure before a successful
    // call to RegisterServiceCtrlHandler, if an error occurs before we have
    // a service status handle we don't catch any exceptions and let the
    // process terminate. The SCM will diligently log this event.
    //

	FILE_LOG(logINFO) << "ServiceMain Startet"  ;
    assert (0 != m_service);

    if (1 != argc || 0 == argv || 0 == argv[0])
    {
        // FIXME AtlThrow(E_INVALIDARG);
    }

    m_service->m_serviceName = argv[0];
    m_service->m_handle = ::RegisterServiceCtrlHandler(m_service->m_serviceName.c_str(), Handler);

    if (0 == m_service->m_handle)
    {
        // FIXME AtlThrowLastWin32();
        //m_service->m_debugFile << "Handler registiert ?=N�" << endl ;
    }

    m_service->UpdateState(SERVICE_START_PENDING);
    sleep(100) ;
    m_service->UpdateState(SERVICE_RUNNING);

    /**
     * now we can call the m_monitor mainloop und wait until the SCM
     * tells us to stop (via Handler)
     */
    m_service->m_monitor->MainLoop();

    m_service->Stop() ;
}

/**
 * The handler function called by the control dispatcher when an event occurs.
 */
void WINAPI MonitorService::Handler(DWORD control)
{
	#ifdef _DEBUG
	FILE_LOG(logINFO) << "Handler: empfangen:" << control << ".." << m_service  ;
	#endif
    //assert (0 == m_service);
        switch (control)
        {
        	case SERVICE_CONTROL_CONTINUE :
            {
                m_service->UpdateState(SERVICE_CONTINUE_PENDING);
                //m_service->Start(control);
                m_service->UpdateState(SERVICE_RUNNING);
                break;
            }
            case SERVICE_CONTROL_PAUSE :
            {
                m_service->UpdateState(SERVICE_PAUSE_PENDING);
                //m_service->Stop();
                m_service->UpdateState(SERVICE_PAUSED);
                break;
            }
            case SERVICE_CONTROL_SHUTDOWN :
            case SERVICE_CONTROL_STOP :
            {
            	FILE_LOG(logINFO) <<  "ServiceMain stopping ...  " ;
                m_service->Stop();
                break;
            }
        }
}



/**
 * Updates the current state and exit code of the service
 * and notifies the service control manager of the change.
 */
void MonitorService::UpdateState(DWORD state, HRESULT errorCode)
{
	#ifdef _DEBUG
	FILE_LOG(logDEBUG) <<  "Updatestatus ..."  ;
	#endif

    //assert (0 != m_service);
    m_status.dwCurrentState = state;


    if (FAILED(errorCode))
    {
        if (FACILITY_WIN32 == HRESULT_FACILITY(errorCode))
        {
            m_status.dwWin32ExitCode = errorCode & ~0x80070000;
        }
        else
        {
            m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            m_status.dwServiceSpecificExitCode = errorCode;
        }
    }

        if (!::SetServiceStatus(m_handle,
                                &m_status))
        {
            // FIXME AtlThrowLastWin32();
        }

        FILE_LOG(logDEBUG) <<  "Updatestatus ... done"  ;

}


bool MonitorService::InstallService()
{
	// Get this exes full pathname
	TCHAR szAppPath[_MAX_PATH];
	GetModuleFileName(NULL, szAppPath, _MAX_PATH);

	std::string sServiceCommandline ;

	sServiceCommandline=szAppPath ;
	sServiceCommandline+=" --service" ;

	// Versuche die aktuell genutzte Configdatei als Parameter zu hinterlegen
	if (m_monitor->m_MonitorConfig.m_ConfigFile == "monitord.xml") {
		// default
		TCHAR szConfigPath[_MAX_PATH];
		strncpy(szConfigPath,szAppPath,_MAX_PATH) ;

		char* pos=strrchr(szConfigPath, '\\') ;
		if (pos)
		{
			*pos='\0' ;
			string configFile=std::string(" -c ") + szConfigPath+"\\monitord.xml" ;
			// << "configFile:" << configFile << endl ;
			sServiceCommandline+=configFile ;
		}
	} else {
		sServiceCommandline += std::string(" -c ");
		sServiceCommandline += m_monitor->m_MonitorConfig.m_ConfigFile;
	}


  //Open up the SCM requesting Creation rights
  SC_HANDLE hSCM = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);
  SC_HANDLE hService ;

  if (hSCM==NULL)
  {
		// FIXME: TRACE(_T("Failed in call to open SCM in Install, GetLastError:%d\n"), ::GetLastError());
		return FALSE;
  }

  //Create the new service entry in the SCM database
  hService = ::CreateService(		hSCM,
  									m_installServiceName.c_str(),
  									m_installServiceDisplayName.c_str(),
  									0,
  									SERVICE_WIN32_OWN_PROCESS,
  									SERVICE_DEMAND_START,
                               		SERVICE_ERROR_IGNORE,
                               		sServiceCommandline.c_str(),
                               	 	NULL,
                               		NULL,
                               		NULL,
                               		NULL,
                               		NULL);



  if (hService == NULL)
  {
  	//FIXME:
    //TRACE(_T("Failed in call to CreateService in Create, GetLastError:%d\n"), ::GetLastError());
  } else
  {
  	::CloseServiceHandle(hService);
  }

   if (hSCM!=NULL)
    {
    	::CloseServiceHandle(hSCM);
    }

  return (hService != NULL);
}

bool MonitorService::UnInstallService()
{
	bool bSuccess=false ;

  	SC_HANDLE hSCM = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);

  	if (hSCM==NULL)
  	{
		// FIXME: TRACE(_T("Failed in call to open SCM in Install, GetLastError:%d\n"), ::GetLastError());
		return FALSE;
	}


	SC_HANDLE hService = ::OpenService(hSCM, m_installServiceName.c_str(), DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS);
  	if (hService != NULL)
  	{
		bSuccess = ::DeleteService(hService);

		::CloseServiceHandle(hService);
  	}
    if (hSCM!=NULL)
    {
    	::CloseServiceHandle(hSCM);
    }

  return (bSuccess);
}
