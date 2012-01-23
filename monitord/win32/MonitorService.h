/**
 * Windows Server für den Monitor, basierend auf http://weblogs.asp.net/kennykerr/archive/2004/05/18/134342.aspx
 */

#include <string>
#include <windef.h>
#include <winsvc.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <jthread-1.2.1/src/jthread.h>

#include "monitord/Monitor.h"

using namespace std ;

class MonitorServiceException : public std::runtime_error {
 public:
   MonitorServiceException(const std::string& s)
     : std::runtime_error(s)
     { }
 };
 #define ThrowMonitorServiceException(err) throw( MonitorServiceException(std::string(__FILE__)+ std::string(" Zeile ") + convertIntToString(__LINE__) + std::string(": ") + std::string(err) ))


class MonitorService
{
public:
    MonitorService(Monitor* monitor);
    virtual ~MonitorService() ;

    static void Run();
    void Stop();
    bool InstallService() ;
    bool UnInstallService() ;

private:
    static void WINAPI Handler(DWORD control);
    static void WINAPI ServiceMain(DWORD argumentCount, PSTR* arguments);
    ofstream m_debugFile ;
    void UpdateState(DWORD state, HRESULT errorCode = S_OK);

    Monitor* m_monitor;

    SERVICE_STATUS_HANDLE m_handle;
    SERVICE_STATUS m_status;

    static MonitorService *m_service;
    std::string m_serviceName; // Executed file (started from SCM)
    std::string  m_installServiceName ; // Shortname in SCM (NOT filename)
    std::string  m_installServiceDisplayName ; // Displayname SCN
};
