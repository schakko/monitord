#ifndef WIN32

#include <dlfcn.h>
#include "dll.h"
#include "../MonitorLogging.h"


DLLManager::DLLManager( const char *fname )
{
    // Try to open the library now and get any error message.

	h=dlopen( fname, RTLD_NOW );
	if (h==NULL)
	{
		err = dlerror();
		LOG_ERROR("error loading library file \"" << fname << "\"" )
		LOG_ERROR("root cause: \"" << err << "\"")
	}
	
}

DLLManager::~DLLManager()
{
	// close the library if it isn't null
	if( h!=0 )
    	dlclose(h);
}


bool DLLManager::GetSymbol(
			   void **v,
			   const char *sym_name
			   )
{
	// try extract a symbol from the library
	// get any error message is there is any

	if( h!=0 )
	{
		*v = dlsym( h, sym_name );
    	err=dlerror();
	    if( err==0 )
       	  return true;
    	else {
    		*v=0 ;
    	  return false;
    	}
	}
	else
	{
		*v=0 ;
        return false;
	}

}


const char * DLLManager::GetDLLExtension()
{
	return ".so";
}

DLLFactoryBase::DLLFactoryBase(
			       const char *fname,
			       const char *factory
			       ) : DLLManager(fname)
{
	// try get the factory function if there is no error yet

	factory_func=0;

	if( LastError()==0 )
	{
    	GetSymbol( (void **)&factory_func, factory ? factory : "factory0" );
	}

}


DLLFactoryBase::~DLLFactoryBase()
{
}

#endif /* ndef WIN32 */

