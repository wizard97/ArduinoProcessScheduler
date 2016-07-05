#ifndef PS_CONFIG_H
#define PS_CONFIG_H

#include <Arduino.h>

/* Uncomment this to allow Exception Handling functionality */
//#define _PROCESS_EXCEPTION_HANDLING

/* Uncomment this to allow Process timing statistics functionality */
//#define _PROCESS_STATISTICS

/* Uncomment this to use microseconds instead of milliseconds for timestamp unit (more precise) */
//#define _MICROS_PRECISION


#ifdef _PROCESS_STATISTICS
/*** The larger the following two types are, the more accurate the statistics will be ****/
    // Type used to track process iterations run count
    typedef uint32_t hIterCount_t;
    // Type used to track process total runtime
    typedef uint32_t hTimeCount_t;
/**************************************/

    // What to divide the two vars above when overflow is about to happen
    #define HISTORY_DIV_FACTOR 2
#endif

#endif
