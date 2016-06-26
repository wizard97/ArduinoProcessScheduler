#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Arduino.h"

class Service;

class Scheduler
{

public:
    Scheduler();
    void add(Service &service);
    void disable(Service &service);
    void enable(Service &service);
    void destroy(Service &service);

    inline uint8_t getID(Service &service);
    inline bool isRunningService(Service &service);
    inline bool isNotDestroyed(Service &service);
    inline bool isEnabled(Service &service);

    inline Service *getCurrService();
    uint32_t getCurrTS();


    int run();
protected:
    // Methods that can be called while inside a service
    void setDisable(Service &service);
    void setEnable(Service &service);
    void setDestroy(Service &service);

    void processFlags(Service &node);
    // Linked list methods
    bool appendNode(Service &node); // true on success
    bool removeNode(Service &node); // true on success
    bool findNode(Service &node); // True if node exists in list

    Service *_head;
    Service *_active;
    uint8_t _lastID;
private:

};

#endif
