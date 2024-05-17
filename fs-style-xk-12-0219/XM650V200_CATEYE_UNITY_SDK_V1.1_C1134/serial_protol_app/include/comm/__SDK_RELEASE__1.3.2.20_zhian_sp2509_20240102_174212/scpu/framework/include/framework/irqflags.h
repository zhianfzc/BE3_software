#ifndef __IRQFLAGS_H__
#define __IRQFLAGS_H__

#define FB_USE_SEM_LOCK         1

#if (FB_USE_SEM_LOCK == 0)

static inline void __attribute__((always_inline))
local_irq_enable(void) { // clearing PRIMASK
    __asm volatile("cpsie i");
}

static inline void __attribute__((always_inline))
local_irq_disable(void) { // setting PRIMASK
    __asm volatile("cpsid i");
}

__asm static inline unsigned long __attribute__((always_inline))
local_irq_restore(unsigned long flags) {
    MSR xPSR, r0
    CPSIE I
}

__asm static inline unsigned long __attribute__((always_inline)) 
local_irq_save() {
    AND R0, R0, #0
    MRS R0, xPSR
    CPSID I    
    ORR R0, R0, #0x100
    ORR R0, R0, #0xFF
    BX lr
}

#endif

#endif
