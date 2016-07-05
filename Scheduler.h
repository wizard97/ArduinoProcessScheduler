#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Includes.h"
#include "setjmp.h"
#include "RingBuf.h" // For queing events

#define SCHEDULER_JOB_QUEUE_SIZE 20

class Process;

class Scheduler
{

public:
    Scheduler();
    ~Scheduler();
    bool add(Process &process, bool enableIfNot = false);
    bool disable(Process &process);
    bool enable(Process &process);
    bool destroy(Process &process);
    bool halt();

    uint8_t getID(Process &process);
    bool isRunningProcess(Process &process);
    bool isNotDestroyed(Process &process);
    bool isEnabled(Process &process);

    Process *getCurrProcess();
    Process *findById(uint8_t id);
    uint8_t countProcesses(bool enabledOnly = true);
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
#ifdef _PROCESS_STATISTICS
            UPDATE_STATS,
#endif
        };

        QueableOperation() : _process(NULL), _operation(static_cast<uint8_t>(NONE)) {}
        QueableOperation(OperationType op) : _process(NULL), _operation(static_cast<uint8_t>(op)) {}
        QueableOperation(Process *serv, OperationType op)
            : _process(serv), _operation(static_cast<uint8_t>(op)) {}

        Process *getProcess() { return _process; }
        OperationType getOperation() { return static_cast<OperationType>(_operation); }
        bool queue(RingBuf *queue) { return queue->add(queue, this) >= 0; }

    private:
        Process *_process;
        const uint8_t _operation;
    };

    // Methods that process queued operations
    void procDisable(Process &process);
    void procEnable(Process &process);
    void procDestroy(Process &process);
    void procAdd(Process &process);
    void procHalt();

    void processQueue();
    // Linked list methods
    bool appendNode(Process &node); // true on success
    bool removeNode(Process &node); // true on success
    bool findNode(Process &node); // True if node exists in list


    Process *volatile _head;
    Process *_active;
    uint8_t _lastID;
    RingBuf *_queue;

private:

#ifdef _PROCESS_STATISTICS
public:
    bool updateStats();
private:
    void procUpdateStats();
    void handleHistOverFlow(uint8_t div);

#endif


#ifdef _PROCESS_EXCEPTION_HANDLING
public:
    void raiseException(int e);
    virtual void handleException(Process &process, int e) { };
protected:
    bool eDispatcher(int e);
    jmp_buf _env;
#endif

};

#endif
