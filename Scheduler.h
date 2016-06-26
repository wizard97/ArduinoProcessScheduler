#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Arduino.h"
#include "Extra.h"

class Service;

class Scheduler
{

public:
    Scheduler();
    SchedulerAction add(Service &service);
    SchedulerAction disable(Service &service);
    SchedulerAction enable(Service &service);
    SchedulerAction destroy(Service &service);

    uint8_t getID(Service &service);
    bool isRunningService(Service &service);
    bool isNotDestroyed(Service &service);
    bool isEnabled(Service &service);

    Service *getCurrService();
    uint8_t countServices(bool enabledOnly = true);
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

    Service *volatile _head;
    Service *_active;
    uint8_t _lastID;
    volatile bool _locked; // Prevent modifications of the underlying linked list
private:


#ifdef _SERVICE_STATISTICS
public:
    void updateStats();
private:
    void handleHistOverFlow(uint8_t div);

#endif

};

#endif
