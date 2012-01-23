#include <typeinfo>
#include <iostream>
#include <time.h>
#include "mpluginAudio.h"
#include "../../config.h"
#include "../convert.h"
#include "../MonitorExceptions.h"
#include "../MonitorLogging.h"

#ifdef HAVE_LIBMP3LAME
	#include <lame/lame.h>
#endif

#ifndef WIN32
	typedef short SHORT ;
	#include <dlfcn.h>
#else
	#include <windows.h>
#endif

using namespace std ;

class MonitorAudioPlugInRecorder : public MonitorAudioPlugIn
{
 public:
 	#ifdef HAVE_LIBMP3LAME
 	/**
 	 *
 	 */
 	 lame_global_flags *gfp ;

	#endif

	
	MonitorAudioPlugInRecorder()
	{
		useLame=false ;
		m_channelNum=0 ;
	}

	virtual ~MonitorAudioPlugInRecorder()
	{
		#ifdef HAVE_LIBMP3LAME
			lame_close(gfp) ;
			gfp=NULL ;
		#endif
	}

	virtual bool InitAudioProcessing(class MonitorConfiguration* configPtr, XMLNode config, int channelNum)
	{
		std::string logFile="screen" ;
		std::string logLevel="DEBUG" ;
		XMLNode parameters ;
		m_channelNum=channelNum;

		if (!((parameters=config.getChildNode("parameters"))).isEmpty())
		{
			m_filePath=getNodeText(parameters,"path","./") ;
			logFile=getNodeText(parameters,"logfile","screen") ;
			logLevel=getNodeText(parameters,"loglevel","INFO") ;
		}

		#ifdef WIN32
		if (!(logFile=="screen"))
		{
			FILE* pFile = fopen(logFile.c_str(), "a");
			Output2FILE::Stream() = pFile;
		}
		FILELog::ReportingLevel() = FILELog::FromString(logLevel);
		FILE_LOG(logINFO) << "logging started";
		#endif

		#ifdef HAVE_LIBMP3LAME

		try {
			FILE_LOG(logINFO) << "enabling lame mp3 support"  ;
			FILE_LOG(logINFO) << "Using MP3Lame Lib Version: " << get_lame_version() ;

			gfp=lame_init() ;
			if (gfp==NULL)
			{
				ThrowMonitorException("Error initializing lame library!") ;
			}

			lame_set_bWriteVbrTag(gfp,0) ; // Kein ID3 Tag
			lame_mp3_tags_fid(gfp,NULL) ;
			lame_set_num_channels(gfp,1) ;
			lame_set_in_samplerate(gfp,22050) ;
			lame_set_brate(gfp,32) ;
			lame_set_mode(gfp,MONO) ;
			lame_set_quality(gfp,7) ;

			int retCode = lame_init_params(gfp) ;
			if (retCode<0)
			{
				ThrowMonitorException("LIBMP3Lame init failed") ;
			}else{
				useLame=1 ;
			}
		} catch (MonitorException e)
		{
			FILE_LOG(logERROR) << e.what() ;
		}
		#endif

		return true ;
	}

	virtual void Show()
	{
		FILE_LOG(logINFO) << "MonitorAudioPluginRecorder successfully loaded"  ;
	}

	virtual void WriteRAWToFile (FILE *pFile,SHORT buffer[],unsigned int bufferSize)
	{
		size_t len=fwrite(buffer, sizeof(SHORT), bufferSize,pFile);
		if (len!=bufferSize)
		{
			FILE_LOG(logERROR) << "Fehler beim Schreiben: " << len << " statt " << bufferSize  ;
			// Fehler beim Schreiben ?
		}
	}

	virtual void WriteMP3ToFile (FILE *pFile,unsigned char buffer[],unsigned int bufferSize)
	{
		size_t len=fwrite(buffer, sizeof(unsigned char), bufferSize,pFile);
		if (len!=bufferSize)
		{
			FILE_LOG(logERROR) << "Fehler beim Schreiben: " << len << " statt " << bufferSize  ;
			// Fehler beim Schreiben ?
		}
	}

	virtual void ProcessAudio(float *buffer, int length)
	{
		const int MAXTEMP=20000 ;
		int jobID=0 ;
		int retCode=0 ; // LIB_MP3Lame encoding return code
		static SHORT tempBuf[MAXTEMP] ;
		static unsigned char mp3Buffer[2*MAXTEMP] ;
		unsigned int tempBufCounter=0 ;
		FILE* pFile=NULL ;
		time_t startTime ;

		if (length>MAXTEMP)
		{
			ThrowMonitorException ("Audiobuffer size exceeds temporary buffer size!") ;
		}

		//if (m_bRecording)
		{
			for (int i=0;i<length;i++)
			{
				tempBuf[tempBufCounter++]=(SHORT)((*buffer)*32768.0) ;
				buffer++;
			}

			#ifdef HAVE_LIBMP3LAME

				if (useLame==1)
				{
					retCode=lame_encode_buffer(gfp,tempBuf,NULL,length,mp3Buffer,0) ;
					if (retCode<0)
					{
						ThrowMonitorException("Error while lame (mp3) encoding") ;
					}
				}
			#endif


			for (jobID=0;jobID<MAXAUDIOCLIENTS;jobID++)
			{
				pFile=(FILE*) getCustomValue(jobID);
				unsigned long seconds = getInfo(jobID) ;
				SocketThread* pClient=getClient(jobID) ;
				startTime=getTime1(jobID) ;

				if ((pFile) && (pClient))
				{
					//
					if (useLame==1)
					{
						WriteMP3ToFile(pFile,mp3Buffer,retCode) ;
					} else {
						WriteRAWToFile(pFile,tempBuf,tempBufCounter) ;
					}
					time_t curTime=time(NULL) ;
					if ( (curTime-startTime)>(time_t) seconds)
					{
						StopRecording(jobID) ;
					}
				}
			}
		}
	}

	void StartRecording(int jobID,std::string fname,int Sekunden)
	{
		FILE_LOG(logINFO) << "Starte Aufnahme .."  ;

		// Aufnahmedatei erzeugen
		FILE_LOG(logINFO) << "Starte mit dauer:" << Sekunden  ;
		if (fname.size()>0)
		{
			// Dateiname wurde vorgegeben
		} else
		{
			fname="kein_datename" ;
		}

		FILE * f_File = fopen(fname.c_str(), "wb");

		if(f_File==NULL) {
			FILE_LOG(logERROR) << "File ist NICHT erstellt" ;
		}
		else
		{
			FILE_LOG(logERROR) << "File ist erstellt"<< fname  ;
		}

		// Aufnahme starten, Daten beim Job hinterlegen
		updateClient(jobID,
						(unsigned long) f_File,
						0,
						Sekunden,
						fname,
						time(NULL)) ;

		addThreadMessage(jobID,std::string("104:" +convertIntToString(m_channelNum)+ ":1:") + convertStringToHex(fname)) ;
	}

	void StopRecording(int jobID)
	{
		FILE_LOG(logINFO) << "Aufnahme beendet"  ;

		FILE* pFile=(FILE*) getCustomValue(jobID) ;
		std::string fname=getInfoText(jobID) ;

		fclose(pFile) ;

		addThreadMessage(jobID,std::string("104:" +convertIntToString(m_channelNum)+ ":0:") + convertStringToHex(fname)) ;
		clearClient(jobID) ;
	}

	virtual std::string DoCommand(std::string command, SocketThread* pClient)
	{
		std::string resultString="" ;
		std::string fname ;
		int Sekunden=0 ;

		FILE_LOG(logINFO) << "Kommando: " << command ;
		parseCommand(command) ;

		for (int i=0;i<10;i++)
		{
			FILE_LOG(logINFO) << "Param " << i << ":" << m_param[i] ;
		}

		if (m_param[0]=="RECORD") //RECORD:<zeit>
		{
			if (m_paramCount>1)
			{
				try
				{
					Sekunden=convertToInt(m_param[1]) ;
					FILE_LOG(logINFO) << "Sekunden als param:" << m_param[1] ;
					FILE_LOG(logINFO) << "Sekunden als int:" << Sekunden  ;
				} catch (BadConversion e)
				{
					FILE_LOG(logERROR) << "Fehler bei der Datenkonvertierung " << m_param[1]  ;
					Sekunden=60 ;
				}
			}

			int jobID=addClient(pClient) ; //< Auftraggeber, fuer callback mit Result
			FILE_LOG(logINFO) << "jobID=" << jobID  ;

			// Laeuft mit der jobID schon eine Aufzeichnung ?
			// Wenn, dann keine neue starten, sondern nur verlaengern

			long SekundenAlterJob = getInfo(jobID) ;
			FILE_LOG(logINFO) << "Zeitdauer alte Aufnahme:" << SekundenAlterJob  ;
			if (SekundenAlterJob>0)
			{
				// Nur die Zeit anpassen
				time_t curTime=time(NULL) ;
				time_t startTime=getTime1(jobID) ;

				FILE_LOG(logDEBUG) << "Zeiten curTime+Sekunden:" << curTime+Sekunden  ;
				FILE_LOG(logDEBUG) << "Zeiten startTime+SekundenAlterJob:" << startTime+SekundenAlterJob  ;
				if (curTime+Sekunden>startTime+SekundenAlterJob)
				{
					fname=getInfoText(jobID) ;
					Sekunden=curTime+Sekunden-startTime ;
					FILE_LOG(logINFO) << "Aufnahme wird auf " << Sekunden << "verlaengert " ;
					updateClient(jobID,getCustomValue(jobID),0,Sekunden,getInfoText(jobID),startTime) ;
					addThreadMessage(jobID,std::string("104:" +convertIntToString(m_channelNum)+ ":2:") + convertStringToHex(fname)) ;
				}
			} else {
				time_t m_startTime=time(NULL) ;
				struct tm *strTime = localtime(&m_startTime) ;

				char zeitString[60] ;
				sprintf(zeitString,"%04d%02d%02d%02d%02d%02d",
										1900+strTime->tm_year,
										1+strTime->tm_mon,
										strTime->tm_mday,
										strTime->tm_hour,
										strTime->tm_min,
										strTime->tm_sec) ;
				FILE_LOG(logDEBUG) << zeitString  ;

				if (m_paramCount<2)
				{
					m_param[2]="X" ;
				}

				fname=	m_filePath
						+ std::string(zeitString)
						+ "_"
						+ m_param[2]
						+ "_"+ convertIntToString(jobID) ;

				cout << fname << endl ;
				if (useLame==1)
				{
					fname+=".mp3"; ;
				} else
				{
					fname+=".raw"; ;
				}

				StartRecording(jobID,fname,Sekunden) ;
			}
		} ;
		resultString="OK" ;

		return (resultString) ;
	}

protected:
	std::string m_filePath ;
	int useLame ;
	int m_channelNum ;

};



class MonitorAudioPlugInRecorderFactory : public MonitorAudioPlugInFactory
{
 public:
	MonitorAudioPlugInRecorderFactory()
	{
	}

	~MonitorAudioPlugInRecorderFactory()
	{
	}

	virtual MonitorAudioPlugIn * CreatePlugIn()
	{
		return new MonitorAudioPlugInRecorder;
	}
};


//
// The "C" linkage factory0() function creates the PlugInFunFactory
// class for this library
//

DLL_EXPORT void * factory0( void )
{
	return new MonitorAudioPlugInRecorderFactory;
}
