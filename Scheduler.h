#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Includes.h"
#include "setjmp.h"
#include "RingBuf.h" // For queing events

#define SCHEDULER_JOB_QUEUE_SIZE 20

class Service;

class Scheduler
{

public:
    Scheduler();
    ~Scheduler();
    bool add(Service &service);
    bool disable(Service &service);
    bool enable(Service &service);
    bool destroy(Service &service);
    bool halt();

    uint8_t getID(Service &service);
    bool isRunningService(Service &service);
    bool isNotDestroyed(Service &service);
    bool isEnabled(Service &service);

    Service *getCurrService();
    Service *findById(uint8_t id);
    uint8_t countServices(bool enabledOnly = true);
    uint32_t getCurrTS();

    int run();
protected:

    // Inner queue object class
    class QueableOperation
    {
    public:
        enum OperationType
        {
            NONE = 0,
            ADD_SERVICE,
            DESTROY_SERVICE,
            DISABLE_SERVICE,
            ENABLE_SERVICE,
            HALT,
#ifdef _SERVICE_STATISTICS
            UPDATE_STATS,
#endif
        };

        QueableOperation() : _service(NULL), _operation(static_cast<uint8_t>(NONE)) {}
        QueableOperation(OperationType op) : _service(NULL), _operation(static_cast<uint8_t>(op)) {}
        QueableOperation(Service *serv, OperationType op)
            : _service(serv), _operation(static_cast<uint8_t>(op)) {}

        Service *getService() { return _service; }
        OperationType getOperation() { return static_cast<OperationType>(_operation); }
        bool queue(RingBuf *queue) { return queue->add(queue, this) >= 0; }

    private:
        Service *_service;
        const uint8_t _operation;
    };

    // Methods that process queued operations
    void procDisable(Service &service);
    void procEnable(Service &service);
    void procDestroy(Service &service);
    void procAdd(Service &service);
    void procHalt();

    void processQueue();
    // Linked list methods
    bool appendNode(Service &node); // true on success
    bool removeNode(Service &node); // true on success
    bool findNode(Service &node); // True if node exists in list


    Service *volatile _head;
    Service *_active;
    uint8_t _lastID;
    RingBuf *_queue;

private:

#ifdef _SERVICE_STATISTICS
public:
    bool updateStats();
private:
    void procUpdateStats();
    void handleHistOverFlow(uint8_t div);

#endif


#ifdef _SERVICE_EXCEPTION_HANDLING
public:
    void raiseException(int e);
    virtual void handleException(Service &service, int e) { };
protected:
    bool eDispatcher(int e);
    jmp_buf _env;
#endif

};

#endif
