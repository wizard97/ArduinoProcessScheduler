#ifndef SERVICE_H
#define SERVICE_H

#include "Arduino.h"
#include "RingBuf.h"
#include "Scheduler.h"

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

typedef enum ServiceFlags
{
    FLAG_ENABLE = 0,
    FLAG_DISABLE,
    FLAG_DESTROY
} ServiceFlags;

class Service
{
    friend class Scheduler;
public:
    Service(Scheduler &manager, unsigned int period,
            int iterations=RUNTIME_FOREVER, bool enabled=true);
    ~Service();
    int getID();
    inline Scheduler &getManager() { return _scheduler; }
    void disable();
    void enable();
    void destroy();

    inline int32_t timeToNextRun() { return (_scheduledTS + _period) - _scheduler.getCurrTS(); };
    inline uint32_t getScheduledTS() { return _scheduledTS; }; // The ts the most recent iteration should of started
    inline uint32_t getActualRunTS() { return _actualTS; }

    inline bool isEnabled() { return _enabled; }
    inline bool isNotDestroyed() { return _scheduler.isNotDestroyed(*this); }
    inline int getIterations() { return _iterations; } // might return RUNTIME_FOREVER
    inline unsigned int getPeriod() { return _period; }

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
    inline bool hasNext() { return _next; }
    // GETTERS
    inline Service *getNext() { return _next; }
    inline RingBuf *getFlagQueue() { return _flags; }
    // SETTERS
    inline void setNext(Service *next) { this->_next = next; }
    inline void setID(uint8_t sid) { this->_sid = sid; }
    inline void decIterations() { _iterations--; }
    inline void setScheduledTS(uint32_t ts) { _scheduledTS = ts; }
    inline void setActualTS(uint32_t ts) { _actualTS = ts; }


    Scheduler &_scheduler;
    bool _enabled;
    int _iterations;
    unsigned int _period;
    uint8_t _sid;
    uint32_t _scheduledTS, _actualTS;

    // Flag queue
    RingBuf* _flags;
    // Linked List
    Service *_next;
};

#endif
