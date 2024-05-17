#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <cmsis_os2.h>
#include "types.h"

struct mutex {
    osMutexId_t id; 
};

void mutex_create(struct mutex *lock);
void mutex_lock(struct mutex *lock);
void mutex_lock_timeout(struct mutex *lock, u32 timeout);
void mutex_unlock(struct mutex *lock);
void mutex_delete(struct mutex *lock);

#endif
