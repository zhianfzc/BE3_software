#ifndef __UTILS_H__
#define __UTILS_H__


#define UNUSED_VARIABLE(x) ((void)(x))

#define GET_MIN(x, y) ({            \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define ROUND_DOWN(x, y) ((x) & ~__round_mask(x, y))
#define ROUND_UP(x, y) ((((x) + (y - 1)) / y) * y)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

#endif
