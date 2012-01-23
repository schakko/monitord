#ifndef MONITORMODULESRESULTS_H_
#define MONITORMODULESRESULTS_H_

/** @brief Basisklasse fuer Rueckgabewerte aus den Modulen
 *
 * In dieser Struktur werden Typ, Zeit und weitere Daten zur Rueckgabe
 * gespeichert.
 */

#include <string>
#include <vector>
#include <time.h>
#include <map>
#include <jthread-1.2.1/src/jthread.h>
#include "MonitorSignals.h"
#include "memlock.h"

#define MAX_CHAR 512


typedef std::map<std::string,std::string> ResultItemsMap ;

/**
 * @brief Basisklasse f�r Rueckgabewerte der Auswerter
 */


class ModuleResultBase
{
public:
	/** Zum einfacheren Zugriff geht folgendes:
	 *
	 * meinString = Result["feldname"]
	 * Result.set("feldname","test")
	 *
	 * Fuers debugging geht auch " os << Result "
	 */

	std::string& operator[](std::string key) { return get(key);} ;
	std::string& operator[](const char key[]) { return operator[](std::string(key));}  ;
	std::string& operator[](char key[]) { return operator[](std::string(key));}  ;

	friend std::ostream & operator<<(std::ostream & os, ModuleResultBase &m) ;

	ModuleResultBase();
	virtual ~ModuleResultBase();
	void copyTo(ModuleResultBase & target) ;

	std::string& get(std::string key) { return m_Items[key];} ;
	bool add(std::string key,std::string item) { m_Items[key]=item; return true;} ;
	bool set(std::string key,std::string item) { return add(key,item);} ;
	ResultItemsMap m_Items ;

protected:
	//time_t m_tTime ;
} ;

std::ostream & operator<<(std::ostream & os, ModuleResultBase & m) ;

typedef ModuleResultBase* ModuleResultBasePtr ;
typedef std::vector<ModuleResultBase*> MODULERESULTSET ;

extern MODULERESULTSET ModuleResultSet ;

/**
 * @brief Sammelt und verteilt Ergebnisse der Decodermodule
 *
 */

class MonitorResultsDispatcher : public JThread
{
public:
	MonitorResultsDispatcher();
	virtual ~MonitorResultsDispatcher();

	bool addResult(ModuleResultBase* pResult) ;
	virtual void* Thread() ;
	bool m_bStop ;

private:
	MODULERESULTSET m_Results ;
	MEMLOCK m_MemLock ;
	MonitorBlockingSignal m_Signal ;
	bool m_bSkipDispatching ;

};



extern MonitorResultsDispatcher *GlobalDispatcher ;

#endif /*MONITORMODULESRESULTS_H_*/
