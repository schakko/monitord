#ifdef WIN32

#include "dll.h"

#include "../MonitorLogging.h"


DLLManager::DLLManager( const char *fname )
{
    // Try to open the library now and get any error message.
	FILE_LOG(logDEBUG) << "calling LoadLibrary.."  ;
	h = LoadLibrary(fname);
	FILE_LOG(logDEBUG) << "LoadLib done."  ;
	if (h == NULL)
	{

		DWORD m;
		int errorCode=GetLastError() ;
		m = FormatMessageA(
					  FORMAT_MESSAGE_ALLOCATE_BUFFER|
					  FORMAT_MESSAGE_FROM_SYSTEM |
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  errorCode,   /* Message Number */
					  0,   				/* Language */
					  (char*)&err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */

		FILE_LOG(logERROR) << "Error loading library \"" << fname << "\":" << err  ;
	}
	else
	{
		err=NULL ;
	}
}

DLLManager::~DLLManager()
{
	// Free error string if allocated
	LocalFree(err);

	// close the library if it isn't null
	if (h != NULL)
    	FreeLibrary(h);

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
		*v = (void*) GetProcAddress(h, sym_name);

	    if ((*v) != NULL)
       	  return true;
    	else
    	{
    		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
					  FORMAT_MESSAGE_FROM_SYSTEM |
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  GetLastError(),   /* Message Number */
					  0,   				/* Language */
					  err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */
    		return false;
    	}
	}
	else
	{
        return false;
	}

}

const char * DLLManager::GetDLLExtension()
{
	return ".dll";
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
	} else {
		FILE_LOG(logERROR) << "Lasterror=" << LastError()  ;
	}

}


DLLFactoryBase::~DLLFactoryBase()
{
}


#endif /* WIN32 */
