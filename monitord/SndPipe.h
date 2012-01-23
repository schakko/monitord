// SndPipe.h: Schnittstelle f�r die Klasse CSndPipe.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNDPIPE_H__8570FCEC_4C9F_4E14_ACCA_6B1559855881__INCLUDED_)
#define AFX_SNDPIPE_H__8570FCEC_4C9F_4E14_ACCA_6B1559855881__INCLUDED_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#include <monitord/win32/MonitorAudioWin32.h>
#else
#ifdef HAVE_LIBASOUND
#include <monitord/posix/MonitorAudioALSA.h>
#else
#include <monitord/posix/MonitorAudioOSS.h>
#endif
#endif
//#include <Sound/SoundOut.h>
//#include <Sound/SoundFile.h>
#include "MonitorAudio.h"
#include "MonitorModules.h"
#include "plugins/mpluginAudio.h"
#include "SocketServer.h"
#include "xmltools.h"

#define MAXPLUGINS 10


class MonitorAudioPlugIn ;

class CSndPipe
{
public:
	void ProcessBuffer(CAudioBuffer* buffer);
	CSndPipe();
	virtual ~CSndPipe();
	static void DataFromSoundIn(CAudioBuffer* buffer, void* Owner);
	int m_cardNum ;
	bool initDecoderModules(int cardnum,MonitorConfiguration* pConfig) ;
	std::string PluginCommand(int channel,std::string command,SocketThread* pThread=NULL) ;
	#ifdef PLUGINS

	MonitorAudioPlugIn* m_pluginsLeft[MAXPLUGINS] ;
	MonitorAudioPlugIn* m_pluginsRight[MAXPLUGINS] ;

	bool loadPlugins(MonitorConfiguration* pConfig, XMLNode configLeft,XMLNode configRight) ; //< Alle Plugins einer Soundkarte laden
	bool loadPlugins(MonitorConfiguration* pConfig, XMLNode config,MonitorAudioPlugIn* plugins[],int channelNum) ; //< Alle Plugins eines Kanals laden
	bool loadPlugin(MonitorConfiguration* pConfig, std::string dllfile,XMLNode config, MonitorAudioPlugIn* plugins[],int channelNum) ; //< einzelnes Plugin laden
	#endif


#ifdef WIN32 /* bisheriger Code unter Sound */
	MonitorAudioWin32	m_SoundIn;
#else /* WIN32 */
#ifdef HAVE_LIBASOUND
	MonitorAudioALSA	m_SoundIn;
#else /* HAVE_LIBASOUND */
	MonitorAudioOSS	m_SoundIn;
#endif /* HAVE_LIBASOUND */
#endif /* WIN32 */
	MonitorModuleArray m_ModulesLinks ;
	MonitorModuleArray m_ModulesRechts ;

};

#endif // !defined(AFX_SNDPIPE_H__8570FCEC_4C9F_4E14_ACCA_6B1559855881__INCLUDED_)
