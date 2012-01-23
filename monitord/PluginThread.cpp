#include "PluginThread.h"
/**
 * PluginThread
 */

#include "MonitorLogging.h"

#ifdef WIN32
	#define usleep Sleep
#endif

#define MAX_QUEUESIZE (100)

PluginThread::PluginThread() : ThreadBase(-1)
{
	LOG_DEBUG("Erstellt PT:")
	m_iLockNum=-1 ;
	m_plugin=NULL ;
	m_bStop=false ;
	dll=NULL ;
}

bool PluginThread::initPlugin(int LOCKNUM, std::string dllfile,XMLNode *pConfig)
{
	m_iLockNum=LOCKNUM ;

	char dllname[255] ;
	memset(dllname,0,255) ;
	strncpy(dllname,dllfile.c_str(),255) ;
	LOG_DEBUG("DLL Name:" << dllname)  

	dll= new DLLFactory<MonitorPlugInFactory>(dllname);

	LOG_DEBUG("done") 
	//
	// If it worked we should have dll.factory pointing
	// to a subclass of PlugInFactory
	//
	if (dll)
	{
		if( dll->factory )
		{
			m_plugin=dll->factory->CreatePlugIn() ;
			if (m_plugin==NULL)
			{
				ThrowMonitorException("Error creating Plugin !") ;
			} else
			{
				// Plugin ist erstellt, nun konfigurieren
				m_plugin->initProcessing(NULL,*pConfig) ;
			}
		} else {
			ThrowMonitorException("Error accessing factory from " + dllfile) ;
		}
	} else {
		ThrowMonitorException("Error loading dll:" + dllfile) ;
	}
	return true ;
}


PluginThread::PluginThread(int LOCKNUM, std::string dllfile,XMLNode *pConfig)
:ThreadBase(LOCKNUM)
{
	initPlugin(LOCKNUM,dllfile,pConfig) ;
}

PluginThread::~PluginThread()
{

}


void *PluginThread::Thread()
{
	ModuleResultBase* pRes ;


	LOG_INFO("PluginThread starting")  
	this->ThreadStarted() ; // Erstmal Bescheid geben, dass wir laufen
	createLock() ;

	while (!m_bStop)
	{
		m_signal.WaitForSignal() ;
		// Daten an Plugin uebergeben
		memLock(m_Lock) ;
		while (m_queue.size()>0)
		{
			LOG_DEBUG("plugin processing - size=" << m_queue.size()) 
			pRes=m_queue.back() ;
			m_queue.pop_back() ;
			memUnlock(m_Lock) ;

			/* to avoid a lock caused by a crashed plugin we release our lock*/
			if (m_plugin)
			{
				m_plugin->processResult(pRes) ;
			}
			delete pRes ;
			memLock(m_Lock) ;
		}
		memUnlock(m_Lock) ;

	}
	LOG_INFO("PluginThread was stopped")  

	m_plugin->quitProcessing();
	return NULL;
}

std::string PluginThread::getPluginName()
{
	return m_pluginName ;
}

void PluginThread::setPluginName(std::string name)
{
	m_pluginName=name ;

}
void PluginThread::addResult(ModuleResultBase* pRes)
{
	ModuleResultBase* localResult=new ModuleResultBase;
	pRes->copyTo(*localResult) ;

	memLock(m_Lock) ;
	if (m_queue.size()<MAX_QUEUESIZE)
	{
		m_queue.insert(m_queue.begin(),localResult) ;
	} else {
		LOG_ERROR("max plugin queue size exceeded. moduleresult not queued") 
	}

	memUnlock(m_Lock) ;

	m_signal.SetSignal() ;
}

// ----------------------------------------
// MonitorPluginsManager

MonitorPluginsManager GlobalMonitorPluginsManager ;
MonitorPluginsManager& GetPluginsManager() { return (GlobalMonitorPluginsManager); }


MonitorPluginsManager::MonitorPluginsManager()
{
	if ( memLockCreate( 12348, & m_MemLock) < 0) {
   		ThrowMonitorException("PluginManager: memLockCreate failed") ;
    }
    m_bStop=false ;
}

bool MonitorPluginsManager::loadScriptFilter(std::string pluginFilterFileName)
{
        #ifdef LUA
	m_bUseLUAScript=false ;
        #endif

	if (! pluginFilterFileName.empty()) {
		#ifdef LUA
		// LUA TEST
		try
		{
			L = lua_open() ;
			luaL_openlibs(L) ;

			if(luaL_loadfile(L, pluginFilterFileName.c_str()))
			{
					throw std::string(std::string(lua_tostring(L, -1)));
			}

			if (lua_pcall(L, 0, 0, 0))
			{
				LOG_ERROR("LUA test fehlgeschlagen" << std::endl) 
			}

			m_bUseLUAScript=true ;
			LOG_INFO("Successfully loaded LUA filter: " << pluginFilterFileName) 
		}
		catch (const std::string &e)
		{
			LOG_ERROR("Error loading lua dispatcher script: "  << e)
		}
		return m_bUseLUAScript ;
		#endif
	}
	return false ;
}

MonitorPluginsManager::~MonitorPluginsManager()
{

	#ifdef LUA
		if (L!=NULL)
		{
			lua_close(L) ;
		}
	#endif
}

bool MonitorPluginsManager::addModule(PluginThread* pThread)
{
	m_Modules.push_back(pThread) ;
	return true ;
}

bool MonitorPluginsManager::removeModule(PluginThread* pThread)
{
	return false ;
}


bool MonitorPluginsManager::dispatchResult(ModuleResultBase *pRes)
{
	tMonitorPluginThreadVector::iterator i ;
        #ifdef LUA
	char eins[255],zwei[255] ;
        #endif

	for (i= m_Modules.begin(); i< m_Modules.end(); i++)
	{
			m_bSkipDispatching=false ;
			// erstmal ggf. LUA Filter aufrufen
				#ifdef LUA

				if (m_bUseLUAScript==true)
				{
					int z ;
				      /* push functions and arguments */
				     lua_getglobal(L, "pluginFilter");  /* function to be called */
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

		 				// Plugintype
		 				memset(eins,0,200) ;
						memset(zwei,0,200) ;
						strncpy(eins,"plugin_name",199) ;
						strncpy(zwei,(*i)->getPluginName().c_str(),199) ;
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
				      	LOG_ERROR("Fehler beim Aufruf lua dispatcher script:" << lua_tostring(L, -1))
				        //error(L, "error running function `f': %s",
				        //         lua_tostring(L, -1));
					  }

				      /* retrieve result */
				      if (!lua_isnumber(L, -1)) {
				      	LOG_ERROR("nicht-numerische Antwort vom lua dispatcher script") 
				        //error(L, "function `f' must return a number");
					  }
				      z = lua_tonumber(L, -1);
				      lua_pop(L, 1);  /* pop returned value */
					  LOG_DEBUG("lua Result (global dispatcher)" << z) 

					  if (z==1) m_bSkipDispatching=true ;
				}
				#endif

				if (m_bSkipDispatching==false)
				{
						(*i)->addResult(pRes) ;
				}
	}
	return true ;
}

bool MonitorPluginsManager::loadPlugin(std::string dllfile, XMLNode *pConfig, std::string pluginName)
{
 	PluginThread* pt=new PluginThread() ;
 	if (pt->initPlugin(4000,dllfile,pConfig))
 	{
		LOG_DEBUG("startet plugin " << dllfile) 
		pt->Start() ;
		pt->setPluginName(pluginName) ;
		addModule(pt) ;

		return true ;
	} else	{
		LOG_ERROR("DLL Factory konnte nicht initialisiert werden !") 
		return false ;
	}
}

bool MonitorPluginsManager::loadPluginsFromConfigNode(XMLNode *pConfig)
{
	// Plugins durchgehen

	std::string pluginName ;
	std::string pluginFile ;
	XMLNode parameterNode ;
	XMLNode *pParameters ;
	XMLNode pluginNode; // = pConfig->getChildNode("plugin",sndCard) ;

	LOG_INFO("reading plugin configuration") 
	int nPlugins=pConfig->nChildNode("plugin");
	for (int plugin=0;plugin<nPlugins;++plugin)
	{
		// Plugin auslesen
		if (!((pluginNode=pConfig->getChildNode("plugin",plugin))).isEmpty())
		{
			pluginName=pluginNode.getAttribute("name");
			pluginFile=getNodeText(pluginNode,"file","") ;
			parameterNode=pluginNode.getChildNode("parameters");

			LOG_INFO("Plugin found: " << pluginName)
			LOG_DEBUG("File:" << pluginFile)
			if (parameterNode.isEmpty()==false)
			{
				LOG_DEBUG("found parameters")
				pParameters=&parameterNode ;
			} else
			{
				LOG_DEBUG("no parameters found")  
				pParameters=NULL;
			}

			if (pluginFile.size()>0)
			{
				loadPlugin(pluginFile,pParameters,pluginName) ;
			}
		}
	}
	return true ;
}






