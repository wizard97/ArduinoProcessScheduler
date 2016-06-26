#include "Scheduler.h"
#include "Service.h"


Scheduler::Scheduler()
{
    _head = NULL;
    _lastID = 0;
}

void Scheduler::add(Service &service)
{
    // Make sure it is not added twice!
    if (!findNode(service))
    {
        service.setup();
        service.setID(++_lastID);
        appendNode(service);
    }
}


uint32_t Scheduler::getCurrTS()
{
    return millis();
}

Service *Scheduler::getCurrService()
{
    return _active;
}

bool Scheduler::isRunningService(Service &service)
{
    return _active && service.getID() == _active->getID();
}


bool Scheduler::isNotDestroyed(Service &service)
{
    return findNode(service);
}

bool Scheduler::isEnabled(Service &service)
{
    return service.isEnabled();
}

void Scheduler::disable(Service &service)
{
    // If this task is not currently running
    if (getCurrService() != &service) {
        setDisable(service);
    } else { // Otherwise queue it
        uint8_t flag = Service::FLAG_DISABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}


void Scheduler::enable(Service &service)
{
    if (getCurrService() != &service) {
        setEnable(service);
    } else {
        uint8_t flag = Service::FLAG_ENABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}


void Scheduler::destroy(Service &service)
{
    if (getCurrService() != &service) {
        setDestroy(service);
    } else {
        uint8_t flag = Service::FLAG_DESTROY;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}

uint8_t Scheduler::getID(Service &service)
{
    return service.getID();
}

int Scheduler::run()
{
    // Nothing to run or already running in another call frame
    if (!_head || _active) return 0;
    int count = 0;
    for (_active = _head; _active != NULL ; _active = _active->getNext(), count++)
    {
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




// This method should only be called if not inside service!
void Scheduler::processFlags(Service &node)
{
    // Process flags
    RingBuf *queue = _active->getFlagQueue();

    uint8_t flag;
    while(queue->pull(queue, &flag)) // Empty Queue
    {
        switch (flag)
        {
            case Service::FLAG_ENABLE:
                if (!_active->isEnabled() && isRunningService(*_active))
                    setEnable(*_active);
                break;

            case Service::FLAG_DISABLE:
                if (_active->isEnabled() && isRunningService(*_active))
                    setDisable(*_active);
                break;

            case Service::FLAG_DESTROY:
                if (isRunningService(*_active))
                    setDestroy(*_active);
                while(queue->pull(queue, &flag)); // Empty Queue
                break;

            default:
                break;
        }
    }

}

#ifdef _SERVICE_STATISTICS

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
