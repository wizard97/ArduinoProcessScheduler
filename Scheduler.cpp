#include "Scheduler.h"
#include "Process.h"


Scheduler::Scheduler()
{
    _head = NULL;
    _lastID = 0;
    // Create queue
    _queue = RingBuf_new(sizeof(QueableOperation), SCHEDULER_JOB_QUEUE_SIZE);
}

Scheduler::~Scheduler()
{
    processQueue();
    RingBuf_delete(_queue);
}

uint32_t Scheduler::getCurrTS()
{
    return TIMESTAMP();
}

Process *Scheduler::getCurrProcess()
{
    return _active;
}

bool Scheduler::isRunningProcess(Process &process)
{
    return _active && &process == _active;
}

Process *Scheduler::findById(uint8_t id)
{
    for (Process *serv = _head; serv != NULL; serv = serv->getNext())
    {
        if (serv->getID() == id)
            return serv;
    }
    return NULL;
}

bool Scheduler::isNotDestroyed(Process &process)
{
    return findNode(process);
}

bool Scheduler::isEnabled(Process &process)
{
    return process.isEnabled();
}

bool Scheduler::disable(Process &process)
{
    QueableOperation op(&process, QueableOperation::DISABLE_SERVICE);
    return op.queue(_queue);
}


bool Scheduler::enable(Process &process)
{
    QueableOperation op(&process, QueableOperation::ENABLE_SERVICE);
    return op.queue(_queue);
}

bool Scheduler::add(Process &process)
{
    QueableOperation op(&process, QueableOperation::ADD_SERVICE);
    return op.queue(_queue);
}

bool Scheduler::destroy(Process &process)
{
    QueableOperation op(&process, QueableOperation::DESTROY_SERVICE);
    return op.queue(_queue);
}

bool Scheduler::halt()
{
    QueableOperation op(QueableOperation::HALT);
    return op.queue(_queue);
}

uint8_t Scheduler::getID(Process &process)
{
    return process.getID();
}

uint8_t Scheduler::countProcesses(bool enabledOnly)
{

    uint8_t count=0;
    for (Process *curr = _head; curr != NULL; curr = curr->getNext())
    {
        count += enabledOnly ? curr->isEnabled() : 1;
    }

    return count;
}

int Scheduler::run()
{
    // Nothing to run or already running in another call frame
    if (!_head || _active) return 0;

    processQueue();

    int count = 0;
    for (_active = _head; _active != NULL ; _active = _active->getNext(), count++)
    {
        uint32_t start = getCurrTS();

        if (_active->needsServicing(start))
        {
            bool force = _active->forceSet(); // Store whether it was a forced iteraiton
            _active->willService(start);

            // Handle scheduler warning
            if (_active->getOverSchedThresh() != OVERSCHEDULED_NO_WARNING && _active->isPBehind(start)) {
                _active->incrPBehind();
                if (_active->getCurrPBehind() >= _active->getOverSchedThresh())
                    _active->overScheduledHandler(start - _active->getScheduledTS());
            } else {
                _active->resetSchedulerWarning();
            }

#ifdef _PROCESS_EXCEPTION_HANDLING
            int ret = setjmp(_env);
            if (!ret) {
                _active->service();
            } else {
                eDispatcher(ret);
            }
#else
            _active->service();
#endif


#ifdef _PROCESS_STATISTICS
            uint32_t runTime = getCurrTS() - start;
            // Make sure no overflow happens
            if (_active->statsWillOverflow(1, runTime))
                handleHistOverFlow(HISTORY_DIV_FACTOR);

            _active->setHistIterations(_active->getHistIterations()+1);
            _active->setHistRuntime(_active->getHistRunTime()+runTime);

#endif
            // Is it time to disable?
            if (_active->wasServiced(force)) {
                disable(*_active);
            }
        }
        processQueue();

    }

    _active = NULL;
    delay(0); // For ESP8266 support
    return count;
}

/************ PROTECTED ***************/
void Scheduler::procDisable(Process &process)
{
    if (process.isEnabled() && isNotDestroyed(process)) {
        process.onDisable();
        process.setDisabled();
    }
}


void Scheduler::procEnable(Process &process)
{
    if (!process.isEnabled() && isNotDestroyed(process)) {
        process.onEnable();
        process.setEnabled();
    }
}


void Scheduler::procDestroy(Process &process)
{
    if (isNotDestroyed(process)) {
        procDisable(process);
        process.cleanup();
        removeNode(process);
    }
}


void Scheduler::procAdd(Process &process)
{
    if (!isNotDestroyed(process)) {
        for (; findById(process.getID()) != NULL; process.setID(++_lastID)) // Find a free id
        process.setup();
        procEnable(process);
        appendNode(process);

#ifdef _PROCESS_STATISTICS
        process.setHistRuntime(0);
        process.setHistIterations(0);
        process.setHistLoadPercent(0);
#endif
    }
}

void Scheduler::procHalt()
{
    for (Process *serv = _head; serv != NULL; serv = serv->getNext())
        procDestroy(*serv);

    HALT_PROCESSOR();
}


//Only call when there is guarantee this is not running in another call frame
void Scheduler::processQueue()
{
    while(!_queue->isEmpty(_queue)) // Empty Queue
    {
        QueableOperation op;
        _queue->pull(_queue, &op);
        switch (op.getOperation())
        {
            case QueableOperation::ENABLE_SERVICE:
                procEnable(*op.getProcess());
                break;

            case QueableOperation::DISABLE_SERVICE:
                procDisable(*op.getProcess());
                break;

            case QueableOperation::ADD_SERVICE:
                procAdd(*op.getProcess());
                break;

            case QueableOperation::DESTROY_SERVICE:
                procDestroy(*op.getProcess());
                break;

            case QueableOperation::HALT:
                procHalt();
                break;

#ifdef _PROCESS_STATISTICS
            case QueableOperation::UPDATE_STATS:
                procUpdateStats();
                break;
#endif

            default:
                break;
        }
    }
}

#ifdef _PROCESS_STATISTICS

bool Scheduler::updateStats()
{
    QueableOperation op(QueableOperation::UPDATE_STATS);
    return op.queue(_queue);
}


void Scheduler::procUpdateStats()
{

    uint8_t count = countProcesses(false);
    HISTORY_TIME_TYPE sTime[count];

    // Thread safe in case of interrupts
    HISTORY_TIME_TYPE totalTime;
    uint8_t i;
    Process *n;
    for(n = _head, i=0, totalTime=0; n != NULL && i < count; n = n->getNext(), i++)
    {
        // to ensure no overflows
        sTime[i] = n->getHistRunTime() / count;
        totalTime += sTime[i];
    }

    for(i=0, n = _head; n != NULL && i < count; n = n->getNext(), i++)
    {
        // to ensure no overflows have to use double
        if (!totalTime) {
            n->setHistLoadPercent(0);
        } else {
            double tmp = 100*((double)sTime[i]/(double)totalTime);
            n->setHistLoadPercent((uint8_t)tmp);
        }
    }

    return;
}

// Make sure it is locked
void Scheduler::handleHistOverFlow(uint8_t div)
{
    for(Process *n = _head; n != NULL; n = n->getNext())
        n->divStats(div);
}

#endif


#ifdef _SERVICE_EXCEPTION_HANDLING
    void Scheduler::raiseException(int e)
    {
        longjmp(_env, e);
    }


    bool Scheduler::eDispatcher(int e)
    {
        if (e != 0 && _active)
        {
            if (!_active->handleException(e))
                handleException(*_active, e);
            return true;
        }
        return false;

    }
#endif



bool Scheduler::appendNode(Process &node)
{
    node.setNext(NULL);

    if (!_head) {
        _head = &node;
    } else {
        Process *next = _head;
        for(; next->hasNext(); next = next->getNext()); //run through list
        // Update pointers
        next->setNext(&node);
    }
    return true;
}

bool Scheduler::removeNode(Process &node)
{
    if (&node == _head) { // node is head
        _head = node.getNext();
    } else {
        // Find the previous node
        Process *prev = _head;
        for (; prev != NULL && prev->getNext() != &node; prev = prev->getNext());

        if (!prev) return false; // previous node does not exist
        prev->setNext(node.getNext());
    }
    return true;
}


bool Scheduler::findNode(Process &node)
{
    Process *prev = _head;
    for (; prev != NULL && prev != &node; prev = prev->getNext());

    return prev;
}
