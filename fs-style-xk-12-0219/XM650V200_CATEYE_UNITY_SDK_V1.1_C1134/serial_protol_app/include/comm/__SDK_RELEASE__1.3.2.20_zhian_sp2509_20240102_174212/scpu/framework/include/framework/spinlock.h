#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


#include <framework/irqflags.h>
#include <framework/atomic.h>

struct spinlock_t {
    /* volatile */ unsigned int slock;
} ;

#if 1

#define preempt_disable() \
do { \
    __schedule_barrier(); \
} while (0)

static inline void spin_lock(struct spinlock_t *lock) {  
    preempt_disable();
}

static inline void spin_unlock(struct spinlock_t *lock) {  
    preempt_disable();
}

static inline void spin_lock_irq(struct spinlock_t *lock) {
    local_irq_disable();
    preempt_disable();
} 

static inline void spin_unlock_irq(struct spinlock_t *lock) {
    local_irq_enable();
    preempt_disable();
} 

static inline unsigned long spin_lock_irqsave(struct spinlock_t *lock)  
{  
    unsigned long flags;
    flags = local_irq_save();
    preempt_disable();
    //kdp_atomic_inc32(&lock->slock);
    return flags;
} 

static inline void spin_unlock_irqrestore(struct spinlock_t *lock, unsigned long flags)
{
    //kdp_atomic_dec32(&lock->slock);
	local_irq_restore(flags);
	preempt_disable();
}

// To Do ...
static inline unsigned long spin_lock_irqsave_xcpu(struct spinlock_t *lock)  
{  
    unsigned long flags;
    return flags;
} 

static inline void spin_unlock_irqrestore_xcpu(struct spinlock_t *lock, unsigned long flags)
{

}

#else

#define spin_lock(lock) 
#define spin_unlock(lock) 
#define spin_lock_irq(lock) 
#define spin_unlock_irq(lock) 
#define spin_lock_irqsave(lock ,flag) 
#define spin_unlock_irqrestore(lock ,flag) 

#endif

#endif
