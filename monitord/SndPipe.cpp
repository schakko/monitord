// SndPipe.cpp: Implementierung der Klasse CSndPipe.
//
//////////////////////////////////////////////////////////////////////

#include "SndPipe.h"
#include "MonitorAudio.h"
#include "MonitorExceptions.h"
#include "convert.h"
#include "Monitor.h"

#include "MonitorModules.h"
#include "MonitorModuleFMS.h"
#include "MonitorModuleZVEI.h"
#include "MonitorModulePocsag512.h"
#include "MonitorModulePocsag1200.h"
#include "MonitorConfiguration.h"

#include "MonitorLogging.h"

#include <iostream>
using namespace std ;

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CSndPipe::CSndPipe()
{
	m_SoundIn.setCallback (&DataFromSoundIn);
	m_SoundIn.setOwner (this);
	m_cardNum=0 ;

	#ifdef PLUGINS
	for (int i=0;i<MAXPLUGINS;i++)
	{
		m_pluginsLeft[i]=NULL ;
		m_pluginsRight[i]=NULL ;
	}
	#endif
}

CSndPipe::~CSndPipe()
{

}

void CSndPipe::DataFromSoundIn(CAudioBuffer* buffer, void* Owner)
{
	((CSndPipe*) Owner)->ProcessBuffer(buffer);
}

void CSndPipe::ProcessBuffer(CAudioBuffer *buffer)
{
	for ( std::vector<MonitorModule*>::iterator currentModule=m_ModulesLinks.begin() ; currentModule != m_ModulesLinks.end() ;currentModule++)
	{
		(*currentModule)->demod(buffer->Left, buffer->Samples) ;
	}

	#ifdef PLUGINS
		SocketMessage msg ;

		if (m_pluginsLeft[0]!=NULL)
		{
			m_pluginsLeft[0]->ProcessAudio(buffer->Left,buffer->Samples) ;

			while (m_pluginsLeft[0]->getThreadMessage(msg))
			{
				if(msg.pThread)
				{
					if (msg.pThread->IsRunning())
					{
						FILE_LOG(logDEBUG) << "L: Nachricht wird versendet ..."  ;
						msg.pThread->addOutputText(msg.message);
					}
				}
			}
		}
	#endif

	for ( std::vector<MonitorModule*>::iterator currentModule=m_ModulesRechts.begin() ; currentModule != m_ModulesRechts.end() ;currentModule++)
	{
		(*currentModule)->demod(buffer->Right, buffer->Samples) ;
	}

	#ifdef PLUGINS
		if (m_pluginsRight[0]!=NULL)
		{
			m_pluginsRight[0]->ProcessAudio(buffer->Right,buffer->Samples) ;

			while (m_pluginsRight[0]->getThreadMessage(msg))
			{
				if(msg.pThread)
				{
					if (msg.pThread->IsRunning())
					{
						FILE_LOG(logDEBUG) << "R: Nachricht wird versendet ..."  ;
						msg.pThread->addOutputText(msg.message);
					}
				}
			}
		}
	#endif
}

std::string CSndPipe::PluginCommand(int channel,std::string command, SocketThread* pThread)
{
	std::string returnString="" ;

	#ifdef PLUGINS
	int i ;
	switch (channel)
	{
		case 0:
			for (i=0;i<MAXPLUGINS;i++)
			{
				if (m_pluginsLeft[i]!=NULL)	returnString=m_pluginsLeft[i]->DoCommand(command,pThread) ;
				if (returnString.size()>0) break ;
			}
		  break ;
		case 1:
			for (i=0;i<MAXPLUGINS;i++)
			{
				if (m_pluginsRight[i]!=NULL)	returnString=m_pluginsRight[i]->DoCommand(command,pThread) ;
				if (returnString.size()>0) break ;
			}

	      break ;
	    default:
	      ThrowMonitorException("Ungueltige Kanalnummer fuer PluginCommand:" + convertIntToString(channel)) ;
	      break ;
	}
	#else
		returnString="not implemented" ;
	#endif

	if (returnString.size()==0) returnString="not implemented" ;
	return returnString ;
}


bool CSndPipe::initDecoderModules(int cardnum, MonitorConfiguration* pConfig)
{
	MonitorModule* newModule ;
	m_cardNum=cardnum ;
	unsigned int sampleRate=22050 ;
	std::string fms_Filter="" ;

	try
	{
		if (pConfig->m_sndConfig[cardnum].iAktiv==1)
		{
			FILE_LOG(logINFO) << "creating decoders for soundcard #" << cardnum  ;
			// FMS ?
			if (pConfig->m_sndConfig[cardnum].iFMS[0]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "L:FMS" ;
				newModule= (MonitorModule*) new MonitorModuleFMS(sampleRate, &(pConfig->m_sndConfig[cardnum].configFMS[0])) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName0) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum) ; // 2*n+0=Links
				m_ModulesLinks.insert(m_ModulesLinks.end(),newModule) ;
			}

			if (pConfig->m_sndConfig[cardnum].iFMS[1]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "R:FMS" ;
				newModule= (MonitorModule*) new MonitorModuleFMS(sampleRate,&(pConfig->m_sndConfig[cardnum].configFMS[1])) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName1) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum+1) ; // 2*n+1=Rechts

				m_ModulesRechts.insert(m_ModulesRechts.end(),newModule) ;
			}

			// ZVEI
			if (pConfig->m_sndConfig[cardnum].iZVEI[0]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "L:ZVEI" ;
				newModule= (MonitorModule*) new MonitorModuleZVEI(sampleRate,&(pConfig->m_sndConfig[cardnum].configZVEI[0])) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName0) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum) ; // 2*n+0=Links

				m_ModulesLinks.insert(m_ModulesLinks.end(),newModule) ;
			}

 			if (pConfig->m_sndConfig[cardnum].iZVEI[1]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "R:ZVEI" ;
				newModule= (MonitorModule*) new MonitorModuleZVEI(sampleRate,&(pConfig->m_sndConfig[cardnum].configZVEI[1])) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName1) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum+1) ; // 2*n+1=Rechts

				m_ModulesRechts.insert(m_ModulesRechts.end(),newModule) ;
			}

			// POC512
			if (pConfig->m_sndConfig[cardnum].iPOC512[0]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "L:POC512" ;
				//newModule= (MonitorModule*) new MonitorModulePocsag512(sampleRate,&(pConfig->m_sndConfig[cardnum].configPOC512[0])) ;
				newModule= (MonitorModule*) new MonitorModulePocsag512(sampleRate,1,0) ;
				//(sampleRate,1,0) ;
				
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName0) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum) ; // 2*n+0=Links

				m_ModulesLinks.insert(m_ModulesLinks.end(),newModule) ;
			}

			if (pConfig->m_sndConfig[cardnum].iPOC512[1]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "R:POC512" ;
				newModule= (MonitorModule*) new MonitorModulePocsag512(sampleRate,1,0) ;
				//newModule= (MonitorModule*) new MonitorModulePocsag512(&(pConfig->m_sndConfig[cardnum].configPOC512[1])) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName1) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum+1) ; // 2*n+1=Rechts

				m_ModulesRechts.insert(m_ModulesRechts.end(),newModule) ;
			}

			// POC1200
			if (pConfig->m_sndConfig[cardnum].iPOC1200[0]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "L:POC1200" ;
				newModule= (MonitorModule*) new MonitorModulePocsag1200(sampleRate,1,0) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName0) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum) ; // 2*n+0=Links

				m_ModulesLinks.insert(m_ModulesLinks.end(),newModule) ;
			}

			if (pConfig->m_sndConfig[cardnum].iPOC1200[1]==1)
			{
				FILE_LOG(logINFO) << "creating decoder for soundcard #" << cardnum  << "R:POC1200" ;
				newModule= (MonitorModule*) new MonitorModulePocsag1200(sampleRate,1,0) ;
				newModule->setChannelName(pConfig->m_sndConfig[cardnum].sChannelName1) ;
				newModule->setServerName(pConfig->m_sMonitordName) ;
				newModule->setChannelNum(2*cardnum+1) ; // 2*n+1=Rechts

				m_ModulesRechts.insert(m_ModulesRechts.end(),newModule) ;
			}
		} else {
			FILE_LOG(logINFO) << "skipping decoders for soundcard #" << cardnum  << " : card not active !" ;
		}

	} catch (MonitorException e)
	{
		FILE_LOG(logERROR) << "Error initialising decoder modules for soundcard #" << cardnum << ":" << e.what()  ;
		return false ;
	}
	return true ;
}
#ifdef PLUGINS

bool CSndPipe::loadPlugins(MonitorConfiguration* pConfig, XMLNode configLeft, XMLNode configRight)
{

	FILE_LOG(logINFO) << "loading audioplugins for left channel" ;
	loadPlugins(pConfig, configLeft,m_pluginsLeft,2*m_cardNum+0) ;
	FILE_LOG(logINFO) << "loading audioplugins for right channel"  ;
	loadPlugins(pConfig, configRight,m_pluginsRight,2*m_cardNum+1) ;

	return true ;
}

bool CSndPipe::loadPlugins(MonitorConfiguration* pConfig, XMLNode config,MonitorAudioPlugIn* plugins[],int channelNum)
{
	std::string file ;
	int nPlugins=0 ;

	nPlugins=config.nChildNode("plugin");
	for (int plugin=0; plugin<nPlugins ;++plugin)
	{
		XMLNode pluginNode ;
		if (!((pluginNode=config.getChildNode("plugin",plugin))).isEmpty())
		{
			file=getNodeText(pluginNode,"file","") ;
			FILE_LOG(logINFO) << "plugin file:" << plugin << "=" << file  ;
			if (file.size()>0)
			{
				loadPlugin(pConfig, file,pluginNode,plugins,channelNum) ;
			}
		}
	}
	return true ;
}

bool CSndPipe::loadPlugin(MonitorConfiguration* pConfig, std::string dllfile,XMLNode config, MonitorAudioPlugIn* plugins[], int channelNum)
{

	int i=0 ;
	for (i=0;i<MAXPLUGINS;i++)
	{
		if (plugins[i]==NULL) break ;
	}

	if ((i<=MAXPLUGINS) &&(plugins[i]==NULL))
	{
		FILE_LOG(logINFO) << "Plugin# " << i << ": loading audio plugin using: " << dllfile  ;
		#ifdef WIN32
			static DLLFactory<MonitorAudioPlugInFactory> dll( dllfile.c_str() );
		#else
			 static DLLFactory<MonitorAudioPlugInFactory> dll( dllfile.c_str() ) ; //  "plugins//.libs//libmplugin_audiorecorder.so" );
		#endif

		//
		// If it worked we should have dll.factory pointing
		// to a subclass of PlugInFactory
		//

		if( dll.factory )
		{
			if ((plugins[i]=dll.factory->CreatePlugIn())==NULL)
			{
				ThrowMonitorException("Error Creating Plugin");
			} else {
				plugins[i]->InitAudioProcessing(pConfig, config, channelNum) ;
			}
		} else {
			FILE_LOG(logERROR) << "Error plugin Factory from file " << dllfile  ;
		}
	}
	return true ;
}

#endif

