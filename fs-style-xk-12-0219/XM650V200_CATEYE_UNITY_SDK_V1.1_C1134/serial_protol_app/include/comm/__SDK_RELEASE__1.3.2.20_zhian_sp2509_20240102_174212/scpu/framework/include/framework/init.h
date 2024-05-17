#ifndef __INIT_H__
#define __INIT_H__


typedef int  (*pf_entr)(void);
typedef void (*pf_exit)(void);


#include "board_kl520.h"
#ifdef CFG_SENSOR_TYPE
    #define ARRANGE_ENTR_RO_SECTION(fn, id) \
        static pf_entr __entr_##fn __attribute__((used, section("entr_section"#id))) = fn;
    #define ARRANGE_EXIT_RO_SECTION(fn) \
        static pf_exit __exit_##fn __attribute__((used, section("exit_section"))) = fn;
#else
    #define ARRANGE_ENTR_RO_SECTION(fn, id) 
    #define ARRANGE_EXIT_RO_SECTION(fn) 
#endif


#define INITTEXT __attribute__((section("init_text")))
#define FINITEXT __attribute__((section("fini_Text"))) 
#define INITDATA __attribute__((section("init_data")))

#endif
