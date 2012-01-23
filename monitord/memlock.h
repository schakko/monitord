#ifndef MEMLOCK_H_
#define MEMLOCK_H_

#ifdef _WIN32
#include <windef.h>
#include <winnt.h>
typedef HANDLE MEMLOCK;
#else
//#include <unistd.h>
typedef int MEMLOCK;
#endif

/* create a new semaphore
 * Input Parameter:
 *   key_t : system unique key of the semaphore
 * Output Parameter:
 *   s  : the handle of the semaphore
 * Return value:
 *    0 : OK
 *   -1 : error
 */
int memLockCreate( int key, MEMLOCK *s);


/* open an existing semaphore
 * Input Parameter:
 *   key_t : system unique key of the semaphore
 * Output Parameter:
 *   s  : the handle of the semaphore
 * Return value:
 *    0 : OK
 *   -1 : error
 */
int memLockOpen( int key, MEMLOCK *s);


/* destroy a semaphore
 * Input Parameter:
 *   s : the semaphore handle
 */
void memLockDestroy( MEMLOCK s);


/* request a lock
 * Input Parameter:
 *   s : the semaphore handle
 * Return value:
 *   0 : OK
 *   else : error
 */
int memLock( MEMLOCK s);


/* release a lock
 * Input Parameter:
 *   s : the semaphore handle
 * Return value:
 *   0 : OK
 *   else : error
 */
int memUnlock( MEMLOCK s);


#endif /*MEMLOCK_H_*/
