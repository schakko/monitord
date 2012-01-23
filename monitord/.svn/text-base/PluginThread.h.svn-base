#ifndef PLUGINTHREAD_H_
#define PLUGINTHREAD_H_

#include "SocketServer.h"

/* sofern lua aktiviert ist ? -> ifdef noetig ? */
#include "lua.hpp"

/*
 * PluginThread
 */

class PluginThread : public ThreadBase
{
public:
	PluginThread(int LOCKNUM, std::string dllfile,XMLNode *pConfig=NULL);
	PluginThread();
	virtual ~PluginThread();
	virtual void *Thread() ;
	bool initPlugin(int LOCKNUM, std::string,XMLNode *pConfig=NULL) ;
	std::string getPluginName() ;
	void setPluginName(std::string pluginName) ;
	MonitorBlockingSignal m_signal ;
	virtual void addResult(ModuleResultBase* pRes) ;
	bool m_bStop ;

private:
	MonitorPlugIn* m_plugin ;
	MODULERESULTSET m_queue ;
	DLLFactory<MonitorPlugInFactory>* dll ;
	std::string m_pluginName ;
} ;

typedef std::vector<PluginThread*> tMonitorPluginThreadVector ;
/**
 * @brief Verwaltet alle Nicht-Audio Plugins
 */

class MonitorPluginsManager
{
public:
	MonitorPluginsManager() ;
	bool loadScriptFilter(std::string FilterFileName) ;
	virtual ~MonitorPluginsManager() ;
	bool addModule(PluginThread* pThread) ;
	bool removeModule(PluginThread* pThread);
	bool dispatchResult(ModuleResultBase *pRes) ;
	bool loadPlugin(std::string dllfile, XMLNode *pConfig, std::string pluginName);
	bool loadPluginsFromConfigNode(XMLNode *pConfig);
	
protected:
	MEMLOCK m_MemLock ;
	tMonitorPluginThreadVector m_Modules ;
	bool m_bStop ;
	
	bool m_bSkipDispatching ;
	#ifdef LUA
		lua_State *L;
		bool m_bUseLUAScript ;
	#endif
};

MonitorPluginsManager& GetPluginsManager() ;


#endif /*PLUGINTHREAD_H_*/
