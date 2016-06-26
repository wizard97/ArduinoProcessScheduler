#include "Scheduler.h"
#include "Service.h"


Scheduler::Scheduler()
{
    _head = NULL;
    _lastID = 0;
}

SchedulerAction Scheduler::add(Service &service)
{
    // Make sure it is not added twice!
    if (!findNode(service) && !_locked && !isRunningService(service))
    {
        _locked = true;
        service.setup();
        service.setID(++_lastID);
        appendNode(service);
        _locked = false;
        uint8_t tmp;
        // Empty out flag buffer
        while (service.getFlagQueue()->pull(service.getFlagQueue(), &tmp));
#ifdef _SERVICE_STATISTICS
        service.setHistRuntime(0);
        service.setHistIterations(0);
        service.setHistLoadPercent(0);
#endif

        return ACTION_SUCCESS;
    }
    return ACTION_NONE;
}


uint32_t Scheduler::getCurrTS()
{
    return TIMESTAMP();
}

Service *Scheduler::getCurrService()
{
    return _active;
}

bool Scheduler::isRunningService(Service &service)
{
    return _active && &service == _active;
}


bool Scheduler::isNotDestroyed(Service &service)
{
    return findNode(service);
}

bool Scheduler::isEnabled(Service &service)
{
    return service.isEnabled();
}

SchedulerAction Scheduler::disable(Service &service)
{
    if (service.locked()) { // Queue it
        uint8_t flag = Service::FLAG_DISABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
        return ACTION_QUEUED;
    } else if (service.isEnabled() && isNotDestroyed(service)) {
        setDisable(service);
        return ACTION_SUCCESS;
    } else {
        return ACTION_NONE;
    }
}


SchedulerAction Scheduler::enable(Service &service)
{
    if (service.locked()) { // Queue it
        uint8_t flag = Service::FLAG_ENABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
        return ACTION_QUEUED;
    } else if (!service.isEnabled() && isNotDestroyed(service)) {
        setEnable(service);
        return ACTION_SUCCESS;
    } else {
        return ACTION_NONE;
    }
}


SchedulerAction Scheduler::destroy(Service &service)
{
    if (!service.locked() && isNotDestroyed(service) && !_locked) {
        _locked = true;
        setDestroy(service);
        _locked = false;
        return ACTION_SUCCESS;
    } else {
        uint8_t flag = Service::FLAG_DESTROY;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
        return ACTION_QUEUED;
    }
}

uint8_t Scheduler::getID(Service &service)
{
    return service.getID();
}

uint8_t Scheduler::countServices(bool enabledOnly)
{
    if (_locked)
        return 0;
    _locked = true;
    uint8_t count=0;
    for (Service *curr = _head; curr != NULL; curr = curr->getNext())
    {
        count += enabledOnly ? curr->isEnabled() : 1;
    }
    _locked = false; // restore
    return count;
}

int Scheduler::run()
{
    // Nothing to run or already running in another call frame
    if (!_head || _active) return 0;
    int count = 0;
    for (_active = _head; _active != NULL ; _active = _active->getNext(), count++)
    {
        _active->lock(); // Lock changes to it!
        uint32_t start = getCurrTS(), runTime;

        if (_active->isEnabled() &&
            (_active->getPeriod() == SERVICE_CONSTANTLY || start - _active->getScheduledTS() >= _active->getPeriod()) &&
            (_active->getIterations() == RUNTIME_FOREVER || _active->getIterations() > 0))
        {
            if (_active->getPeriod() != SERVICE_CONSTANTLY)
                _active->setScheduledTS(_active->getScheduledTS() + _active->getPeriod());
            else
                _active->setScheduledTS(getCurrTS());

            _active->setActualTS(start);
            _active->service();

            runTime = getCurrTS() - start;

            if (_active->getIterations() > 0)
            {
                _active->decIterations();
                if (_active->getIterations() == 0)
                    _active->disable();
            }

#ifdef _SERVICE_STATISTICS
            // Make sure no overflow happens
            if (_active->statsWillOverflow(1, runTime))
                handleHistOverFlow(HISTORY_DIV_FACTOR);

            _active->setHistIterations(_active->getHistIterations()+1);
            _active->setHistRuntime(_active->getHistRunTime()+runTime);

#endif

        }
        processFlags(*_active);
        _active->unlock();
    }

    _active = NULL;

    return count;
}

/************ PROTECTED ***************/

void Scheduler::setDisable(Service &service)
{
    service.onDisable();
    service._enabled = false;
}


void Scheduler::setEnable(Service &service)
{
    service.onEnable();
    service._enabled = true;
}


void Scheduler::setDestroy(Service &service)
{
    setDisable(service);
    service.cleanup();
    removeNode(service);
}



//only call if safe to do so
void Scheduler::processFlags(Service &node)
{
    if (_locked)
        return;
    _locked = true;
    // Process flags
    RingBuf *queue = _active->getFlagQueue();

    uint8_t flag;
    while(queue->pull(queue, &flag)) // Empty Queue
    {
        switch (flag)
        {
            case Service::FLAG_ENABLE:
                if (!_active->isEnabled() && isNotDestroyed(*_active))
                    setEnable(*_active);
                break;

            case Service::FLAG_DISABLE:
                if (_active->isEnabled() && isNotDestroyed(*_active))
                    setDisable(*_active);
                break;

            case Service::FLAG_DESTROY:
                if (isNotDestroyed(*_active))
                {
                    setDestroy(*_active);
                    while(queue->pull(queue, &flag)); // Empty Queue
                }
                break;

            default:
                break;
        }
    }
    _locked = false;

}

#ifdef _SERVICE_STATISTICS

void Scheduler::updateStats()
{
    if (_locked)
        return;
    uint8_t count = countServices(false);
    HISTORY_TIME_TYPE sTime[count];

    // Thread safe in case of interrupts
    HISTORY_TIME_TYPE totalTime;
    uint8_t i;
    Service *n;
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

}

void Scheduler::handleHistOverFlow(uint8_t div)
{
    for(Service *n = _head; n != NULL; n = n->getNext())
        n->divStats(div);
}

#endif

bool Scheduler::appendNode(Service &node)
{
    node.setNext(NULL);

    if (!_head) {
        _head = &node;
    } else {
        Service *next = _head;
        for(; next->hasNext(); next = next->getNext()); //run through list
        // Update pointers
        next->setNext(&node);
    }
    return true;
}

bool Scheduler::removeNode(Service &node)
{
    if (&node == _head) { // node is head
        _head = node.getNext();
    } else {
        // Find the previous node
        Service *prev = _head;
        for (; prev != NULL && prev->getNext() != &node; prev = prev->getNext());

        if (!prev) return false; // previous node does not exist
        prev->setNext(node.getNext());
    }
    return true;
}


bool Scheduler::findNode(Service &node)
{
    Service *prev = _head;
    for (; prev != NULL && prev->getNext() != &node; prev = prev->getNext());
    return prev;
}
