#ifndef SOCKETTHREADMONITORD_H_
#define SOCKETTHREADMONITORD_H_

#include "SocketServer.h"

class SocketThreadMonitord : public SocketThread
{
public:
	SocketThreadMonitord(MonitorConfiguration *config, int LOCKNUM, int PortNum);
	virtual ~SocketThreadMonitord();

protected:
	virtual void processInput() ;
	void checkLogin() ;
	virtual void sayWelcome() ;
	bool parseCommand() ;
	virtual std::string createFMSOutputString(ModuleResultBase Result) ;
	virtual std::string createZVEIOutputString(ModuleResultBase Result) ;
	virtual std::string createPOCSAGOutputString(ModuleResultBase Result) ;
	void startRecording(int seconds=60, int channel=0) ;
	void tellCapabilites() ;
	void tellChannels() ;
};

#endif /*SOCKETTHREADMONITORD_H_*/
