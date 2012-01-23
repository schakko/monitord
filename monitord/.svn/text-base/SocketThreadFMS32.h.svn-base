#ifndef SOCKETTHREADFMS32_H_
#define SOCKETTHREADFMS32_H_

#include "SocketServer.h"

/**
 * FMS32-Pro Protokoll nach der Beschreibung in http://www.heirue-soft.de/neues321.pdf
 * erstellt
 */
class SocketThreadFMS32 : public SocketThread
{
public:
	SocketThreadFMS32(MonitorConfiguration *config, int LOCKNUM, int PortNum);
	virtual ~SocketThreadFMS32();

protected:
	virtual void processInput() ;
	virtual void sayWelcome() ;
	virtual std::string createFMSOutputString(ModuleResultBase Result) ;
	virtual std::string createZVEIOutputString(ModuleResultBase Result) ;
	virtual std::string createPOCSAGOutputString(ModuleResultBase Result) ;
};

#endif /*SOCKETTHREADFMS32_H_*/
