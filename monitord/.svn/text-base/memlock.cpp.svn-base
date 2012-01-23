#include <stdio.h>

#include "memlock.h"

#ifndef _WIN32
//#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#else
#include <stdarg.h>
#include <winbase.h>
#endif

int memLockCreate( int key, MEMLOCK *s)
{
#ifdef _WIN32
    char semname[30];
    sprintf( semname, "sem%08x", key);
    *s = CreateSemaphore( NULL, 1, 1, semname);
    return (*s != NULL)? 0 : -1;
#else
    *s = semget( key, 1, IPC_CREAT|0600);
    if (*s > 0) memUnlock( *s);
    return (*s >= 0)? 0 : -1;
#endif
}

int memLockOpen( int key, MEMLOCK *s)
{
#ifdef _WIN32
    char semname[30];
    sprintf( semname, "sem%08x", key);
    *s = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, semname);
    return (*s != NULL)? 0 : -1;
#else
    *s = semget( key, 1, 0);
    return (*s >= 0)? 0 : -1;
#endif
}

void memLockDestroy( MEMLOCK s)
{
#ifdef _WIN32
    CloseHandle( s);
#else
    semctl( s, 0, IPC_RMID);
#endif
}

int memLock( MEMLOCK s)
{
#ifdef _WIN32
    return (WAIT_FAILED == WaitForSingleObject( s, INFINITE));
#else
    struct sembuf sop;

    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;

    return semop( s, &sop, 1);
#endif
}

int memUnlock( MEMLOCK s)
{
#ifdef _WIN32
    return !ReleaseSemaphore ( s, 1, NULL);
#else
    struct sembuf sop;

    if (semctl( s, 0, GETVAL) > 0)
        return -1 ; /* already unlocked */

    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;

    return semop( s, &sop, 1);
#endif
}
