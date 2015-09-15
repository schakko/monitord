#ifndef WIN32

#include <dlfcn.h>
#include "dll.h"
#include "../MonitorLogging.h"


DLLManager::DLLManager( const char *fname )
{
	// Try to open the library now and get any error message.
	LOG_INFO("Opening \"" << fname << "\"")

	h = dlopen( fname, RTLD_NOW );

	if (h == NULL)
	{
		err = dlerror();
		LOG_ERROR("error loading library file \"" << fname << "\"" )
		LOG_ERROR("root cause: \"" << err << "\"")
	}
	else {
		LOG_INFO("dlopen for \"" << fname << "\" succeeded")
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
	LOG_INFO("GetSymbol \"" << sym_name << "\"")

	if( h!=0 )
	{
		LOG_INFO("Calling dlsym \"" << sym_name << "\"")

		*v = dlsym( h, sym_name );
    		err=dlerror();
		
		if( err==0 ) {
			LOG_INFO("Symbol found, initializing plug-in")
			return true;
		}
		else {
			LOG_ERROR("error getting symbol \"" << sym_name << "\"")
			LOG_ERROR("root cause: \"" << err << "\"")

			*v=0 ;
			return false;
    		}
	}
	else
	{
		LOG_ERROR("No handle for dl found")

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
	LOG_INFO("DLLFactoryBase: init")
	// try get the factory function if there is no error yet

	factory_func=0;

	if( LastError() != NULL )
	{
		LOG_INFO("DllFactoryBase: trying to get symbol")
		GetSymbol( (void **)&factory_func, factory ? factory : "factory0" );
	}
	else {
		LOG_ERROR("DLLFactoryBase: error occured")
	}
}


DLLFactoryBase::~DLLFactoryBase()
{
}

#endif /* ndef WIN32 */

