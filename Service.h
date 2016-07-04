#ifndef SERVICE_H
#define SERVICE_H

#include "RingBuf.h"
#include "Scheduler.h"
#include "Includes.h"

// Service period
#define SERVICE_CONSTANTLY 0
#define SERVICE_SECONDLY 1000
#define SERVICE_MINUTELY 60000
#define SERVICE_HOURLY 3600000

// Number of services
#define RUNTIME_FOREVER -1
#define RUNTIME_ONCE 1

#define MAX_QUEUED_FLAGS 10

#define OVERSCHEDULED_NO_WARNING 0
class Scheduler;


class Service
{
    friend class Scheduler;
public:
    Service(Scheduler &manager, unsigned int period,
            int iterations=RUNTIME_FOREVER, bool enabled=true,
            int16_t overSchedThresh = OVERSCHEDULED_NO_WARNING);

    int getID();
    inline Scheduler &scheduler() { return _scheduler; }
    bool add();
    bool disable();
    bool enable();
    bool destroy();

    inline int32_t timeToNextRun() { return (_scheduledTS + _period) - _scheduler.getCurrTS(); };
    inline uint32_t getScheduledTS() { return _scheduledTS; }; // The ts the most recent iteration should of started
    inline uint32_t getActualRunTS() { return _actualTS; }

    inline bool isEnabled() { return _enabled; }
    inline bool isNotDestroyed() { return _scheduler.isNotDestroyed(*this); }
    inline int getIterations() { return _iterations; } // might return RUNTIME_FOREVER
    inline unsigned int getPeriod() { return _period; }

    inline void force() { _force = true; }
    inline void resetSchedulerWarning() { _pBehind = 0; }
    inline uint16_t getOverSchedThresh() { return _overSchedThresh; }
    inline uint16_t getCurrPBehind() { return _pBehind; }

protected:
    inline uint32_t getStartDelay() { return _actualTS - _scheduledTS; }
    // Fired on creation/destroy
    virtual void setup();
    virtual void cleanup();
    //called on enable/disable
    virtual void onEnable();
    virtual void onDisable();
    // service routine
    virtual void service() = 0;
    // Overscheduled warning
    virtual void overScheduledHandler(uint32_t behind);

private:
    enum ServiceFlags
    {
        FLAG_ENABLE = 0,
        FLAG_DISABLE,
        FLAG_DESTROY
    };

    void willService(uint32_t ts);
    bool wasServiced(bool wasForced);
    bool needsServicing(uint32_t start);
    bool isPBehind(uint32_t curr);
    inline bool hasNext() { return _next; }
    // GETTERS
    inline bool forceSet() { return _force; }
    inline Service *getNext() { return _next; }
    // SETTERS
    inline void setNext(Service *next) { this->_next = next; }
    inline void setID(uint8_t sid) { this->_sid = sid; }
    inline void decIterations() { _iterations--; }
    inline void setScheduledTS(uint32_t ts) { _scheduledTS = ts; }
    inline void setActualTS(uint32_t ts) { _actualTS = ts; }

    inline void incrPBehind() { _pBehind++; }

    inline void setDisabled() { _enabled = false; }
    inline void setEnabled() { _enabled = true; }

    Scheduler &_scheduler;
    bool _enabled, _force;
    int _iterations;
    unsigned int _period;
    uint8_t _sid;
    uint32_t _scheduledTS, _actualTS;
    // Linked List
    Service *volatile _next;

    // Tracks overscheduled
    uint16_t _overSchedThresh, _pBehind;



#ifdef _SERVICE_STATISTICS
public:
    uint32_t getAvgRunTime();
    inline uint8_t getLoadPercent() { return _histLoadPercent; }
private:
    bool statsWillOverflow(HISTORY_COUNT_TYPE iter, HISTORY_TIME_TYPE tm);
    void divStats(uint8_t div);
    inline void setHistIterations(HISTORY_COUNT_TYPE val) { _histIterations = val; }
    inline void setHistRuntime(HISTORY_TIME_TYPE val) { _histRunTime = val; }
    inline void setHistLoadPercent(uint8_t percent) { _histLoadPercent = percent; }
    inline HISTORY_COUNT_TYPE getHistIterations() { return _histIterations; }
    inline HISTORY_TIME_TYPE getHistRunTime() { return _histRunTime; }

    HISTORY_COUNT_TYPE _histIterations;
    HISTORY_TIME_TYPE _histRunTime;
    uint8_t _histLoadPercent;

#endif


#ifdef _SERVICE_EXCEPTION_HANDLING

protected:
    // By default do not handle
    virtual bool handleException(int e) { return false; };
    virtual void raiseException(int e) { _scheduler.raiseException(e); }
#endif
};



#endif
