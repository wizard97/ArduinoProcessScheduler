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

#define MAX_QUEUED_FLAGS 5

class Scheduler;


class Service
{
    friend class Scheduler;
public:
    Service(Scheduler &manager, unsigned int period,
            int iterations=RUNTIME_FOREVER, bool enabled=true);
    ~Service();
    int getID();
    inline Scheduler &scheduler() { return _scheduler; }
    SchedulerAction add();
    SchedulerAction disable();
    SchedulerAction enable();
    SchedulerAction destroy();

    inline int32_t timeToNextRun() { return (_scheduledTS + _period) - _scheduler.getCurrTS(); };
    inline uint32_t getScheduledTS() { return _scheduledTS; }; // The ts the most recent iteration should of started
    inline uint32_t getActualRunTS() { return _actualTS; }

    inline bool isEnabled() { return _enabled; }
    inline bool isNotDestroyed() { return _scheduler.isNotDestroyed(*this); }
    inline int getIterations() { return _iterations; } // might return RUNTIME_FOREVER
    inline unsigned int getPeriod() { return _period; }

    inline void force() { _force = true; }

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

private:
    enum ServiceFlags
    {
        FLAG_ENABLE = 0,
        FLAG_DISABLE,
        FLAG_DESTROY
    };

    void willService(uint32_t ts);
    void wasServiced(bool wasForced);
    bool needsServicing(uint32_t start);
    inline bool hasNext() { return _next; }
    // GETTERS
    inline bool forceSet() { return _force; }
    inline Service *getNext() { return _next; }
    inline RingBuf *getFlagQueue() { return _flags; }
    // SETTERS
    inline void setNext(Service *next) { this->_next = next; }
    inline void setID(uint8_t sid) { this->_sid = sid; }
    inline void decIterations() { _iterations--; }
    inline void setScheduledTS(uint32_t ts) { _scheduledTS = ts; }
    inline void setActualTS(uint32_t ts) { _actualTS = ts; }

    inline void lock() { _locked = true; }
    inline void unlock() { _locked = false; }
    inline bool locked() { return _locked; }

    Scheduler &_scheduler;
    bool _enabled, _force;
    int _iterations;
    unsigned int _period;
    uint8_t _sid;
    uint32_t _scheduledTS, _actualTS;

    // Flag queue
    RingBuf* _flags;
    // Linked List
    Service *_next;
    //Locks changes
    volatile bool _locked;



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
