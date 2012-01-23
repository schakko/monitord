#include <typeinfo>
#include <iostream>
#include <time.h>
#include "mpluginAudio.h"
#include "../../config.h"
#include "../convert.h"
#include "../MonitorExceptions.h"
#include "../MonitorLogging.h"

extern "C" {
	#include "sox.h"
}


#ifndef WIN32
	typedef short SHORT ;
	#include <dlfcn.h>
#else
	#include <windows.h>
#endif

using namespace std ;

class MonitorAudioPlugInRecorderSOX : public MonitorAudioPlugIn
{
 public:
	
 	/**
 	 *
 	 */
	
	MonitorAudioPlugInRecorderSOX()
	{
		m_channelNum=0 ;
	}

	virtual ~MonitorAudioPlugInRecorderSOX()
	{
			sox_format_quit() ;
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
		// TODO: log4cxx
		FILELog::ReportingLevel() = FILELog::FromString(logLevel);
		LOG_INFO("logging started";
		#endif

		
		if (sox_init() != SOX_SUCCESS)
		{
			ThrowMonitorException("Error initializing sox library!") ;
		} ;

		StartRecording(1,"test",10) ;
		return true ;
	}

	virtual void Show()
	{
		LOG_INFO("MonitorAudioPluginRecorder successfully loaded") 
	}

	virtual void WriteToFile (FILE *pFile,unsigned char buffer[],unsigned int bufferSize)
	{
		size_t len=fwrite(buffer, sizeof(unsigned char), bufferSize,pFile);
		if (len!=bufferSize)
		{
			LOG_ERROR("Fehler beim Schreiben: " << len << " statt " << bufferSize)
			// Fehler beim Schreiben ?
		}
	}

	virtual void ProcessAudio(float *buffer, int length)
	{
		SOX_SAMPLE_LOCALS ;
		const int MAXTEMP=20000 ;
		int jobID=0 ;
		int myCount=0 ;
		static sox_sample_t tempBuf[MAXTEMP] ;
		//static unsigned char mp3Buffer[2*MAXTEMP] ;
		unsigned int tempBufCounter=0 ;
		time_t startTime ;

		if (length>MAXTEMP)
		{
			ThrowMonitorException ("Audiobuffer size exceeds temporary buffer size!") ;
		}

	
		{
			for (int i=0;i<length;i++)
			{
				tempBuf[tempBufCounter++]=SOX_FLOAT_32BIT_TO_SAMPLE(*buffer,myCount) ;
				buffer++;
			}
		
			
			for (jobID=0;jobID<MAXAUDIOCLIENTS;jobID++)
			{
				sox_format_t * pFormat=(sox_format_t*) getCustomValue(jobID);
				unsigned long seconds = getInfo(jobID) ;
				SocketThread* pClient=getClient(jobID) ;
				startTime=getTime1(jobID) ;
				
				//TODO: 
				if ((pFormat) && (pClient))
				{
					//
					if (sox_write (pFormat,tempBuf,length)==0)
					{
						// error
						ThrowMonitorException("Error while sox encoding") ;
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
		LOG_INFO("Starte Aufnahme ..")

		// Aufnahmedatei erzeugen
		LOG_INFO("Starte mit dauer:" << Sekunden)
		if (fname.size()>0)
		{
			// Dateiname wurde vorgegeben
		} else
		{
			fname="kein_datename" ;
		}

		sox_format_t * pFormat = new sox_format_t ;
		sox_signalinfo_t info ;
		sox_encodinginfo_t encoding ;
		 
	
		info.rate=8000 ;
		info.channels=1 ;
		info.precision= 32 ;
		info.length=SOX_IGNORE_LENGTH ;
		info.mult=0 ;

		sox_init_encodinginfo(&encoding) ;
		
		encoding.encoding = SOX_ENCODING_LPC10 ;
		encoding.bits_per_sample = 0 ; // TODO ?
		encoding.compression = 0 ;
		
		pFormat =sox_open_write ("test.mp3", // war: lpc
								&info,
								&encoding,
								0,
								0,
								0) ;

		if(pFormat==NULL) {
			LOG_ERROR("File ist NICHT erstellt")
		}
		else
		{
			LOG_ERROR("File ist erstellt: "<< fname)
		}

		// Aufnahme starten, Daten beim Job hinterlegen
		updateClient(jobID,
						(unsigned long) pFormat,
						0,
						Sekunden,
						fname,
						time(NULL)) ;		
		
		addThreadMessage(jobID,std::string("104:" +convertIntToString(m_channelNum)+ ":1:") + convertStringToHex(fname)) ;
	}

	void StopRecording(int jobID)
	{
		LOG_INFO("Aufnahme beendet")

		sox_format_t* pFormat=(sox_format_t*) getCustomValue(jobID) ;
		std::string fname=getInfoText(jobID) ;

		sox_close(pFormat) ;
		
		addThreadMessage(jobID,std::string("104:" +convertIntToString(m_channelNum)+ ":0:") + convertStringToHex(fname)) ;
		clearClient(jobID) ;
		delete pFormat ;
	}

	virtual std::string DoCommand(std::string command, SocketThread* pClient)
	{
		std::string resultString="" ;
		std::string fname ;
		int Sekunden=0 ;

		LOG_INFO("Kommando: " << command)
		parseCommand(command) ;

		for (int i=0;i<10;i++)
		{
			LOG_INFO("Param " << i << ":" << m_param[i])
		}

		if (m_param[0]=="RECORD") //RECORD:<zeit>
		{
			if (m_paramCount>1)
			{
				try
				{
					Sekunden=convertToInt(m_param[1]) ;
					LOG_INFO("Sekunden als param:" << m_param[1])
					LOG_INFO("Sekunden als int:" << Sekunden)
				} catch (BadConversion e)
				{
					LOG_ERROR("Fehler bei der Datenkonvertierung " << m_param[1])
					Sekunden=60 ;
				}
			}

			int jobID=addClient(pClient) ; //< Auftraggeber, fuer callback mit Result
			LOG_INFO("jobID=" << jobID)

			// Laeuft mit der jobID schon eine Aufzeichnung ?
			// Wenn, dann keine neue starten, sondern nur verlaengern

			long SekundenAlterJob = getInfo(jobID) ;
			LOG_INFO("Zeitdauer alte Aufnahme:" << SekundenAlterJob) 
			if (SekundenAlterJob>0)
			{
				// Nur die Zeit anpassen
				time_t curTime=time(NULL) ;
				time_t startTime=getTime1(jobID) ;

				LOG_DEBUG( "Zeiten curTime+Sekunden:" << curTime+Sekunden  ;
				LOG_DEBUG( "Zeiten startTime+SekundenAlterJob:" << startTime+SekundenAlterJob  ;
				if (curTime+Sekunden>startTime+SekundenAlterJob)
				{
					fname=getInfoText(jobID) ;
					Sekunden=curTime+Sekunden-startTime ;
					LOG_INFO("Aufnahme wird auf " << Sekunden << "verlaengert ")
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
				LOG_DEBUG( zeitString)

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
		return new MonitorAudioPlugInRecorderSOX;
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

