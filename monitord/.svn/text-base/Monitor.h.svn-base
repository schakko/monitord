#ifndef MONITORD_MONITOR_H
#define MONITORD_MONITOR_H

#include <jthread-1.2.1/src/jthread.h>
#include "memlock.h"
#include "MonitorConfiguration.h"
#include "SndPipe.h"
#include "convert.h"
#include "MonitorExceptions.h"
#include "MonitorSignals.h"
#include "config.h"

/**
 * @brief Repraesentiert die Anwendung (MainApp)
 *
 *
 */

class Monitor {
public:
	void Initialize(int argc, char* argv[]);
	void MainLoop();
	MonitorConfiguration m_MonitorConfig;
	bool m_bWantStop;
	CSndPipe *m_sndIn[4];
	MonitorBlockingSignal *m_SignalStopped ;
private:
	void CreateSocketServer(MonitorConfiguration*);
	void InitSndCard();
	void StopSndCard();
	MEMLOCK s;
};

#endif /* MONITORD_MONITOR_H */
