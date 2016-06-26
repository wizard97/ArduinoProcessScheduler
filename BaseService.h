#ifndef BASE_SERVICE_H
#define BASE_SERVICE_H

#include "Arduino.h"

// Service period
#define SERVICE_CONSTANTLY 0
#define SERVICE_SECONDLY 1000
#define SERVICE_MINUTELY 60000
#define SERVICE_HOURLY 3600000

// Number of services
#define RUNTIME_FOREVER -1
#define RUNTIME_ONCE 1

class ServiceManager;

class BaseService
{
    friend class ServiceManager;
public:
    BaseService(ServiceManager &manager, unsigned int period,
            int iterations=RUNTIME_FOREVER, bool enabled=true);
    int getID();
    ServiceManager &getManager();
    void disable();
    void enable();
    void destroy();

    int getIterations();
    unsigned int getPeriod();

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
    inline bool hasNext();
    // GETTERS
    inline BaseService *getNext();
    // SETTERS
    inline void setNext(BaseService *next);
    inline void setID(uint8_t sid);

    ServiceManager &_manager;
    bool _enabled;
    int _iterations;
    unsigned int _period;
    uint8_t _sid;

    // Linked List
    BaseService *_next;
};

#endif
