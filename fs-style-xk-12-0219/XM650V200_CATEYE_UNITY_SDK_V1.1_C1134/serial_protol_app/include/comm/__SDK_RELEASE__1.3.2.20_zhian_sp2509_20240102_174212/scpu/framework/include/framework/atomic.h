#ifndef __ATOMIC_H__
#define __ATOMIC_H__


static asm volatile void kdp_atomic_inc32(unsigned int *mem) {
#if 0    
  mov   r2,r0
1
  ldrex r0,[r2]
  adds  r1,r0,#1
  strex r3,r1,[r2]
  cbz   r3,%F2
  b     %B1
2
  bx     lr
#endif    
}

static asm volatile void kdp_atomic_dec32(unsigned int *mem) {
#if 0    
  mov   r2,r0
1
  ldrex r0,[r2]
  cbnz  r0,%F2
  clrex
  bx    lr
2
  subs  r1,r0,#1
  strex r3,r1,[r2]
  cbz   r3,%F3
  b     %B1
3
  bx     lr
#endif
}

#endif
