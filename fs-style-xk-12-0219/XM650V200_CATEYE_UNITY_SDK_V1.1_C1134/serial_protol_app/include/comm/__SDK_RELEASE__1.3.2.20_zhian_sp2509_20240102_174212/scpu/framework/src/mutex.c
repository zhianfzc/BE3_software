#include "framework/mutex.h"


void mutex_create(struct mutex *lock)
{
    lock->id = osMutexNew(NULL);
}

void mutex_lock(struct mutex *lock)
{
	//might_sleep();
	osMutexAcquire(lock->id, 0);
}

void mutex_lock_timeout(struct mutex *lock, u32 timeout)
{
    osMutexAcquire(lock->id, timeout);
}

void mutex_unlock(struct mutex *lock)
{
    osMutexRelease(lock->id);
}

void mutex_delete(struct mutex *lock)
{
    if (lock->id != NULL) 
        osMutexDelete(lock->id);
}
