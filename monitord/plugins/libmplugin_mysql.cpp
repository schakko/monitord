
#include <typeinfo>
#include <iostream>

#ifdef WIN32
#define usleep Sleep
#include <windows.h>
#endif


#include "mysql/mysql.h"
#include "mplugin.h"
#include "../MonitorLogging.h"

using namespace std ;

enum fieldsource {mysql=0,resultset=1} ;

typedef struct
{
	string value ;
	fieldsource source ;
} FieldInfo ;

typedef map<string,FieldInfo*> MappingInfo ;
typedef pair <string,FieldInfo*> PairMapping ;

// Class PlugInFun inherits from PlugIn
// and shows the world when one is created/destroyed
// and supplies a Show() method that announces its name


class MonitorPlugInMySQL : public MonitorPlugIn
{
 public:
 	MYSQL m_mysql ;
 	std::string hostname ;
 	std::string username ;
 	std::string password ;
 	std::string database ;
 	unsigned int port ;
	unsigned int ssl;
	std::string ssl_cacert;
	std::string ssl_cert;
	std::string ssl_key;
 	bool m_bConnected ;

 	MappingInfo fmsMapping ;
 	MappingInfo zveiMapping ;
 	MappingInfo pocsagMapping ;

 	std::string fmsTable;
 	std::string zveiTable;
 	std::string pocsagTable;

	MonitorPlugInMySQL()
	{
 		m_bConnected=false ;
	}

	virtual ~MonitorPlugInMySQL()
	{
	}

	virtual void Show()
	{
		FILE_LOG(logINFO) << "MonitorMySQLPlugin successfully loaded" ;
	}

	std::string escape_string(std::string text)
	{
		const unsigned int MAXLEN=255 ;
		char result[MAXLEN+1] ;

		memset(result,0,MAXLEN+1) ;
		mysql_real_escape_string(&m_mysql,result,text.c_str(),text.size()>MAXLEN ? MAXLEN : text.size()) ;
		return string(result) ;
	}

	virtual bool processResult(class ModuleResultBase *pRes)
	{
		FILE_LOG(logDEBUG) << "mysql: processing Result..."  ;

		if (m_bConnected==false)
		{
			return false ;
		}

		int pingcounter=0 ;
		while (mysql_ping(&m_mysql) && pingcounter<100)
		{
			usleep(100) ;
			pingcounter++ ;
			FILE_LOG(logINFO) << "mysql connection lost ... trying reconnect"  ;
		}

		if (mysql_ping(&m_mysql))
		{
			FILE_LOG(logERROR) << " unable to reconnect to mysql database" ;
			return false ;
		}

		if (pingcounter>0)
		{
			FILE_LOG(logINFO) << "mysql: connection re-established"  ;
		}

		// wenn wir latin1 einfuegen sollten wir das mysql auch mitteilen!
		mysql_query(&m_mysql, "SET NAMES latin1");

		if ((*pRes)["typ"]=="fms")
		{
			std::string insertString ;
			insertString = createInsertString(pRes,fmsMapping,fmsTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		} else if ((*pRes)["typ"]=="pocsag")
		{
			std::string insertString ;
			insertString = createInsertString(pRes,pocsagMapping,pocsagTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		}else if ((*pRes)["typ"]=="zvei")
		{
			std::string insertString;
			insertString = createInsertString(pRes,zveiMapping,zveiTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		}


		return true ;
	}

	virtual bool initProcessing(class MonitorConfiguration* configPtr,XMLNode config)
	{
		mysql_init(&m_mysql) ;

		#if (MYSQL_VERSION_ID>50013)
		/* for mysql V5.0.3+ is auto_reconnect disabled by default*/
		my_bool reconnect = 1;
		mysql_options(&m_mysql, MYSQL_OPT_RECONNECT, &reconnect);
		#endif

		hostname=getNodeText(config,"hostname","localhost") ;
		username=getNodeText(config,"username","root") ;
		password=getNodeText(config,"password","rootpw") ;
		database=getNodeText(config,"database","demo") ;
		port=getNodeInt(config,"port",3306) ;
		ssl=getNodeInt(config,"ssl",0);
		ssl_cacert=getNodeText(config,"ssl-cacert","0");
		ssl_cert=getNodeText(config,"ssl-cert","0");
		ssl_key=getNodeText(config,"ssl-key","0");
		std::string logFile=getNodeText(config,"logfile","screen") ;
		std::string logLevel=getNodeText(config,"loglevel","INFO") ;
		#ifdef WIN32
		if (!(logFile=="screen"))
		{
			FILE* pFile = fopen(logFile.c_str(), "a");
			Output2FILE::Stream() = pFile;

		}
		FILELog::ReportingLevel() = FILELog::FromString(logLevel);
		FILE_LOG(logINFO) << "logging started";
		#endif

		if (ssl == 1 && ssl_cacert != "0" && ssl_cert != "0" && ssl_key != "0")
		{
			mysql_ssl_set(&m_mysql,
				ssl_key.c_str(),
				ssl_cert.c_str(),
				ssl_cacert.c_str(),
				NULL,
				NULL);
			FILE_LOG(logINFO) << "mySQL ssl support configured with key=" << ssl_key << " and cert=" << ssl_cert << " and cacert=" << ssl_cacert;
		}

		// Parameter parsen
		parseParameter(config) ;
		if (mysql_real_connect(	&m_mysql,
								hostname.c_str(),
								username.c_str(),
								password.c_str(),
								database.c_str(),
								port,
								NULL,		// Unix Socket
								0			// Options
								) ==NULL)
		{
			FILE_LOG(logERROR) << "Could not connect to database \"" << database << "\" on host " << hostname << " with username=\"" << username << "\""  ;
			m_bConnected=false ;
		} else
		{
			FILE_LOG(logINFO) << "successfully connected to mysql database " << database << " on host " << hostname << " with username=\"" << username << "\""  ;
			m_bConnected=true ;
		}
		return m_bConnected ;
	} ;

	virtual bool quitProcessing() {return true;} ;

	void parseParameter(XMLNode config)
	{
		XMLNode mappingNode ;

		int nMapping=config.nChildNode("mapping");

		for (int num=0; num<nMapping ; ++num)
		{
			if (!((mappingNode=config.getChildNode("mapping",num))).isEmpty())
			{
				std::string typ=mappingNode.getAttribute("typ") ;

				if (typ=="fms")
				{
					readMappings(mappingNode,fmsMapping,fmsTable) ;
				} else if (typ=="pocsag")
				{
					readMappings(mappingNode,pocsagMapping,pocsagTable) ;
				} else if (typ=="zvei")
				{
					readMappings(mappingNode,zveiMapping,zveiTable) ;
				}
			}
		}
	}

	void readMappings(XMLNode config,MappingInfo &mappingInfo,std::string &dbTable)
	{
		XMLNode mappingNode ;
		std::string table=getNodeText(config,"table","status") ;
		std::string name, source, value ;

		FILE_LOG(logDEBUG) << " reading mapping info " ;
		FILE_LOG(logDEBUG) << "table="<< table  ;

		dbTable=table;
		FieldInfo *pFieldInfo ;

		int nMapping=config.nChildNode("field");

		for (int num=0; num<nMapping ; ++num)
		{
			if (!((mappingNode=config.getChildNode("field",num))).isEmpty())
			{

				name=mappingNode.isAttributeSet("name") ? mappingNode.getAttribute("name") : "" ;
				source=mappingNode.isAttributeSet("source") ? mappingNode.getAttribute("source") : "" ;
				value= mappingNode.getText() ;
				if (name.empty()) ThrowMonitorException("MySQL Konfiguration: Kein Feldname vergeben ! ") ;
				if (value.empty()) ThrowMonitorException("MySQL Konfiguration: Kein Feldwert vergeben ! ") ;

				FILE_LOG(logDEBUG) << "Feld: " << name << " / " << source << ":" << value  ;
				pFieldInfo=new FieldInfo ;
				pFieldInfo->value=value ;
				if (source=="mysql")
				{
					pFieldInfo->source=mysql ;
				} else
				{
					pFieldInfo->source=resultset ;
				}
				mappingInfo.insert( PairMapping(name,pFieldInfo));
			}
		}
	}

	std::string createInsertString(class ModuleResultBase *pRes,MappingInfo mapping,std::string table)
	{
		std::string insertString="";
			std::string valueString="" ;
			MappingInfo::iterator i ;
			FieldInfo* pFieldInfo ;
			std::string fieldName="" ;

			insertString=" insert into " + table +  " (" ;
			for (i=mapping.begin();i!=mapping.end();i++)
			{
				fieldName= i->first ;
				pFieldInfo= i->second ;

				FILE_LOG(logDEBUG) << "field:" << fieldName << " | value=" << pFieldInfo->value  ;
				if (i!=mapping.begin())
				{
					insertString+="," ;
					valueString+="," ;
				}

				insertString+=fieldName ;
				if (pFieldInfo->source==mysql)
				{
					valueString+= pFieldInfo->value ;
				} else
				{
					valueString+="\"" +escape_string((*pRes)[pFieldInfo->value])+"\"" ;
				}
			}
			insertString+=") values (" ;
			insertString+=valueString + ")" ;

			FILE_LOG(logDEBUG) << "Insertstring:" << insertString ;
			return insertString ;
	}

};

//
// The PlugInFunFactory class inherits from PlugInFactory
// and creates a PlugInFun object when requested.
//


class MonitorPlugInMySQLFactory : public MonitorPlugInFactory
{
 public:
	MonitorPlugInMySQLFactory()
	{
	}

	~MonitorPlugInMySQLFactory()
	{
	}

	virtual MonitorPlugIn * CreatePlugIn()
	{
		return new MonitorPlugInMySQL() ;
	}
};


//
// The "C" linkage factory0() function creates the PlugInFunFactory
// class for this library
//

DLL_EXPORT void * factory0( void )
{
	return new MonitorPlugInMySQLFactory;
}
