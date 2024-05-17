/*
 * vim: ai ts=4 sts=4 sw=4 cinoptions=>4 expandtab
 */
#define _GNU_SOURCE

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "timer.h"



/* how many runs to average by default */
#define DEFAULT_NR_LOOPS 10

/* we have 3 tests at the moment */
#define MAX_TESTS 3

/* default block size for test 2, in bytes */
#define DEFAULT_BLOCK_SIZE 262144

/*
 * MBW memory bandwidth benchmark
 *
 * 2006, 2012 Andras.Horvath@gmail.com
 *
 * http://github.com/raas/mbw
 *
 * compile with:
 *			gcc -O -o mbw mbw.c
 *
 * run with eg.:
 *
 *			./mbw 300
 *
 * or './mbw -h' for help
 *
 * watch out for swap usage (or turn off swap)
 */

void usage()
{
    printf("Usage: mbw [options] array_size_in_MiB\n");
    printf("Options:\n");
    printf("	-n: number of runs per test\n");
    printf("	-a: Don't display average\n");
    printf("	-t0: memcpy test\n");
    printf("	-t1: dumb (b[i]=a[i] style) test\n");
    printf("	-t2 : memcpy test with fixed block size\n");
    printf("	-b <size>: block size in bytes for -t2 (default: %d)\n", DEFAULT_BLOCK_SIZE);
    printf("	-q: quiet (print statistics only)\n");
    printf("(will then use two arrays, watch out for swapping)\n");
    printf("'Bandwidth' is amount of data copied over the time this operation took.\n");
    printf("\nThe default is to run all tests available.\n");
}

/* ------------------------------------------------------ */

/* allocate a test array and fill it with data
 * so as to force Linux to _really_ allocate it */
long *make_array(unsigned long long asize)
{
    unsigned int long_size=sizeof(long);
    long *a;

    a=malloc(asize * long_size);

    if(NULL==a) {
        perror("Error allocating memory");
        return NULL;
    }

    ///* make sure both arrays are allocated, fill with pattern */
    memset(a, 0xaa, asize * long_size);
    return a;
}

/* actual benchmark */
/* asize: number of type 'long' elements in test arrays
 * long_size: sizeof(long) cached
 * type: 0=use memcpy, 1=use dumb copy loop (whatever GCC thinks best)
 *
 * return value: elapsed time in seconds
 */
double worker(unsigned long long asize, long *a, long *b, int type, unsigned long long block_size)
{
    unsigned long long t;
    unsigned long long starttime, endtime;
    double te;
    unsigned int long_size=sizeof(long);
    /* array size in bytes */
    unsigned long long array_bytes=asize*long_size;

    if(type==0) { /* memcpy test */
        /* timer starts */
        clk_get_pts(&starttime);
        memcpy(b, a, array_bytes);
        /* timer stops */
        clk_get_pts(&endtime);
    } else if(type==2) { /* memcpy block test */
        clk_get_pts(&starttime);
        for(t=0; t<array_bytes; t+=block_size) {
            //b=mempcpy(b, a, block_size);
            b=memmove(b, a, block_size);
        }
        if(t>array_bytes) {
            //b=mempcpy(b, a, t-array_bytes);
            b=memmove(b, a, t-array_bytes);
        }
        clk_get_pts(&endtime);
    } else { /* dumb test */
        clk_get_pts(&starttime);
        for(t=0; t<asize; t++) {
            b[t]=a[t];
        }
        clk_get_pts(&endtime);
    }

    te=(double)(endtime - starttime) / 1000000000;

    return te;
}

/* ------------------------------------------------------ */

/* pretty print worker's output in human-readable terms */
/* te: elapsed time in seconds
 * mt: amount of transferred data in MiB
 * type: see 'worker' above
 *
 * return value: -
 */
void printout(double te, double mt, int type)
{
    switch(type) {
        case 0:
            printf("Method: MEMCPY\t");
            break;
        case 1:
            printf("Method: DUMB\t");
            break;
        case 2:
            printf("Method: MCBLOCK\t");
            break;
    }
    printf("Elapsed: %.5f\t", te);
    printf("MiB: %.5f\t", mt);
    printf("Copy: %.3f MiB/s\n", mt/te);
    return;
}

/* ------------------------------------------------------ */

static int mbw(int argc, char **argv)
{
    unsigned int long_size=0;
    double te, te_sum; /* time elapsed */
    unsigned long long asize=0; /* array size (elements in array) */
    int i;
    long *a, *b; /* the two arrays to be copied from/to */
    unsigned long testno;

    /* options */

    /* how many runs to average? */
    int nr_loops=DEFAULT_NR_LOOPS;
    /* fixed memcpy block size for -t2 */
    unsigned long long block_size=DEFAULT_BLOCK_SIZE;
    /* show average, -a */
    int showavg=1;
    /* what tests to run (-t x) */
    int tests[MAX_TESTS];
    double mt=0; /* MiBytes transferred == array size in MiB */
    int quiet=0; /* suppress extra messages */

    tests[0]=0;
    tests[1]=0;
    tests[2]=0;

    /* default is to run all tests if no specific tests were requested */
    if( (tests[0]+tests[1]+tests[2]) == 0) {
        tests[0]=1;
        tests[1]=1;
        tests[2]=1;
    }

    if(argc == 2) {
        mt=strtoul(argv[1], (char **)NULL, 10);
    } else {
        printf("Error: no array size given!\n");
        return(1);
    }

    if(0>=mt) {
        printf("Error: array size wrong!\n");
        return(1);
    }

    /* ------------------------------------------------------ */

    long_size=sizeof(long); /* the size of long on this platform */
    asize=1024*1024/long_size*mt; /* how many longs then in one array? */

    if(asize*long_size < block_size) {
        printf("Error: array size larger than block size (%llu bytes)!\n", block_size);
        return(1);
    }

    if(!quiet) {
        printf("Long uses %d bytes. ", long_size);
        printf("Allocating 2*%lld elements = %lld bytes of memory.\n", asize, 2*asize*long_size);
        if(tests[2]) {
            printf("Using %lld bytes as blocks for memcpy block copy test.\n", block_size);
        }
    }

    a=make_array(asize);
    b=make_array(asize);

    /* ------------------------------------------------------ */
    if(!quiet) {
        printf("Getting down to business... Doing %d runs per test.\n", nr_loops);
    }

    /* run all tests requested, the proper number of times */
    for(testno=0; testno<MAX_TESTS; testno++) {
        te_sum=0;
        if(tests[testno]) {
            for (i=0; i<nr_loops; i++) {
                te=worker(asize, a, b, testno, block_size);
                te_sum+=te;
                printf("%d\t", i);
                printout(te, mt, testno);
            }
            if(showavg) {
                printf("AVG\t");
                printout(te_sum/nr_loops, mt, testno);
            }
        }
    }

    free(a);
    free(b);
    return 0;
}

MSH_CMD_EXPORT(mbw, mem test)
