#ifndef SOCKETTHREADCRUSADER_H_
#define SOCKETTHREADCRUSADER_H_

#include "SocketServer.h"

#define CRUSADER_DELIMITER "#![]!#"

class SocketThreadCrusader : public SocketThread
{
public:
	SocketThreadCrusader(MonitorConfiguration *config, int LOCKNUM, int PortNum);
	virtual ~SocketThreadCrusader();

protected:
	virtual void processInput() ;
	void checkLogin(std:: string) ;
	virtual void sayWelcome() ;
	bool parseCommand() ;
	virtual std::string createFMSOutputString(ModuleResultBase Result) ;
	virtual std::string createZVEIOutputString(ModuleResultBase Result) ;
	virtual std::string createPOCSAGOutputString(ModuleResultBase Result) ;
	unsigned long telegrammCounter ;

};

#endif /*SOCKETTHREADCRUSADER_H_*/
