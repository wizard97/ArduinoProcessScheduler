#ifndef SCHEDULER_SERVICE_EXTRA_H
#define SCHEDULER_SERVICE_EXTRA_H

#define _SERVICE_STATISTICS

#define _SERVICE_EXCEPTION_HANDLING

typedef enum SchedulerAction
{
    ACTION_NONE,
    ACTION_SUCCESS,
    ACTION_QUEUED,
} SchedulerAction;

#ifdef _MICROS_PRECISION
    #define TIMESTAMP() micros()
#else
    #define TIMESTAMP() millis()
#endif

#ifdef _SERVICE_STATISTICS
    #define HISTORY_COUNT_TYPE uint32_t
    #define HISTORY_TIME_TYPE uint32_t

    #define HISTORY_DIV_FACTOR 2
#endif

#endif
