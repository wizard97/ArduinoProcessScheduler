#ifndef PROCESS_H
#define PROCESS_H

#include "Includes.h"
#include "Scheduler.h"

// Process period
#define SERVICE_CONSTANTLY 0
#define SERVICE_SECONDLY 1000
#define SERVICE_MINUTELY 60000
#define SERVICE_HOURLY 3600000

// Number of Processs
#define RUNTIME_FOREVER -1
#define RUNTIME_ONCE 1

#define OVERSCHEDULED_NO_WARNING 0

class Scheduler;


class Process
{
    friend class Scheduler;
public:
    Process(Scheduler &manager, ProcPriority priority, unsigned int period,
            int iterations=RUNTIME_FOREVER,
            int16_t overSchedThresh = OVERSCHEDULED_NO_WARNING);

    int getID();
    inline Scheduler &scheduler() { return _scheduler; }
    bool add(bool enableIfNot=false);
    bool disable();
    bool enable();
    bool destroy();

    inline int32_t timeToNextRun() { return (_scheduledTS + _period) - _scheduler.getCurrTS(); }
    inline uint32_t getScheduledTS() { return _scheduledTS; } // The ts the most recent iteration should of started
    inline uint32_t getActualRunTS() { return _actualTS; }

    inline bool isEnabled() { return _enabled; }
    inline bool isNotDestroyed() { return _scheduler.isNotDestroyed(*this); }
    inline int getIterations() { return _iterations; } // might return RUNTIME_FOREVER
    inline unsigned int getPeriod() { return _period; }

    inline void force() { _force = true; }

    inline void resetSchedulerWarning() { _pBehind = 0; }
    inline uint16_t getOverSchedThresh() { return _overSchedThresh; }
    inline uint16_t getCurrPBehind() { return _pBehind; }

    inline ProcPriority getPriority() { return _pLevel; }

protected:
    inline uint32_t getStartDelay() { return _actualTS - _scheduledTS; }
    // Fired on creation/destroy
    virtual void setup();
    virtual void cleanup();
    //called on enable/disable
    virtual void onEnable();
    virtual void onDisable();
    // Process routine
    virtual void service() = 0;
    // Overscheduled warning
    virtual void overScheduledHandler(uint32_t behind);

private:
    enum ProcessFlags
    {
        FLAG_ENABLE = 0,
        FLAG_DISABLE,
        FLAG_DESTROY
    };

    void willService(uint32_t now);
    bool wasServiced(bool wasForced);
    bool needsServicing(uint32_t start);
    bool isPBehind(uint32_t curr);
    inline bool hasNext() { return _next; }
    // GETTERS
    inline bool forceSet() { return _force; }
    inline Process *getNext() { return _next; }
    // SETTERS
    inline void setNext(Process *next) { this->_next = next; }
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
    Process *volatile _next;

    // Tracks overscheduled
    uint16_t _overSchedThresh, _pBehind;

    const ProcPriority _pLevel;



#ifdef _PROCESS_STATISTICS
public:
    uint32_t getAvgRunTime();
    inline uint8_t getLoadPercent() { return _histLoadPercent; }
private:
    bool statsWillOverflow(hIterCount_t iter, hTimeCount_t tm);
    void divStats(uint8_t div);
    inline void setHistIterations(hIterCount_t val) { _histIterations = val; }
    inline void setHistRuntime(hTimeCount_t val) { _histRunTime = val; }
    inline void setHistLoadPercent(uint8_t percent) { _histLoadPercent = percent; }
    inline hIterCount_t getHistIterations() { return _histIterations; }
    inline hTimeCount_t getHistRunTime() { return _histRunTime; }

    hIterCount_t _histIterations;
    hTimeCount_t _histRunTime;
    uint8_t _histLoadPercent;

#endif


#ifdef _PROCESS_EXCEPTION_HANDLING

protected:
    // By default do not handle
    virtual bool handleException(int e) { return false; }
    virtual void raiseException(int e) { _scheduler.raiseException(e); }
#endif
};



#endif
