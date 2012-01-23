#ifndef MPLUGIN_H_
#define MPLUGIN_H_

#ifdef PLUGINS

#include "dll.h"
#include "../MonitorModulesResults.h"
#include "../MonitorConfiguration.h"
#include "../xmltools.h"


//
// PlugIn is an abstract class.
//
// This is an example plug in.  This plug in only has one method, Show(),
// which we will use to show its name.
//
//

class MonitorPlugIn
{
 public:
	MonitorPlugIn();

	virtual ~MonitorPlugIn();

	virtual bool initProcessing(class MonitorConfiguration* configPtr,XMLNode config) {return true ; } ;
	virtual bool processResult(class ModuleResultBase *pRes)=0 ;
	virtual bool quitProcessing() {return true;} ;
	virtual void Show() = 0;
protected:

};


//
// The is an example factory for plug ins.
//
// This example factory only announces when it is created/destroyed and
// has the single abstract method CreatePlugIn() which returns a type
// of plug in.
//
// In the real world, you may have multiple different classes in each
// shared library that are made to work together.  All these classes
// must be created by the Factory class.
//
// You may find it useful to have the objects that you create with
// the factory class be given a pointer to the factory class so
// they can create their own objects that they need, using the same
// factory class.  Compiler support of covariant return types is
// real useful here.
//


class MonitorPlugInFactory
{
 public:
	MonitorPlugInFactory()
	{	}

	virtual ~MonitorPlugInFactory()
	{	}

	virtual MonitorPlugIn * CreatePlugIn() {
		return NULL ;
	}
};

#endif

#endif /*MPLUGIN_H_*/
