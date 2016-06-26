#ifndef BASE_SERVICE_H
#define BASE_SERVICE_H

#include "Arduino.h"
#include "RingBuf.h"
#include "ServiceManager.h"

// Service period
#define SERVICE_CONSTANTLY 0
#define SERVICE_SECONDLY 1000
#define SERVICE_MINUTELY 60000
#define SERVICE_HOURLY 3600000

// Number of services
#define RUNTIME_FOREVER -1
#define RUNTIME_ONCE 1

#define MAX_QUEUED_FLAGS 5

class ServiceManager;

typedef enum ServiceFlags
{
    FLAG_ENABLE = 0,
    FLAG_DISABLE,
    FLAG_DESTROY
} ServiceFlags;

class BaseService
{
    friend class ServiceManager;
public:
    BaseService(ServiceManager &manager, unsigned int period,
            int iterations=RUNTIME_FOREVER, bool enabled=true);
    ~BaseService();
    int getID();
    inline ServiceManager &getManager() { return _manager; }
    void disable();
    void enable();
    void destroy();

    inline bool isEnabled() { return _enabled; }
    inline bool isNotDestroyed() { return _manager.isNotDestroyed(*this); }
    inline uint32_t getLastRunTS() { return _lastRunTS; }
    inline int getIterations() { return _iterations; } // might return RUNTIME_FOREVER
    inline unsigned int getPeriod() { return _period; }

protected:
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
    inline BaseService *getNext() { return _next; }
    inline RingBuf *getFlagQueue() { return _flags; }
    // SETTERS
    inline void setNext(BaseService *next) { this->_next = next; }
    inline void setID(uint8_t sid) { this->_sid = sid; }
    inline void decIterations() { _iterations--; }
    inline void updateRunTS(uint32_t ts) { _lastRunTS = ts; }

    ServiceManager &_manager;
    bool _enabled;
    int _iterations;
    unsigned int _period;
    uint8_t _sid;
    uint32_t _lastRunTS;

    // Flag queue
    RingBuf* _flags;
    // Linked List
    BaseService *_next;
};

#endif
