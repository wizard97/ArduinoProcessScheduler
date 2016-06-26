#include "ServiceManager.h"
#include "BaseService.h"


ServiceManager::ServiceManager()
{
    _head = NULL;
    _lastID = 0;
}

void ServiceManager::add(BaseService &service)
{
    // Make sure it is not added twice!
    if (!findNode(service))
    {
        service.setup();
        service.setID(++_lastID);
        appendNode(service);
    }
}


BaseService *ServiceManager::getCurrService()
{
    return _active;
}

bool ServiceManager::isRunningService(BaseService &service)
{
    return _active && service.getID() == _active->getID();
}


bool ServiceManager::isNotDestroyed(BaseService &service)
{
    return findNode(service);
}

bool ServiceManager::isEnabled(BaseService &service)
{
    return service.isEnabled();
}

void ServiceManager::disable(BaseService &service)
{
    // If this task is not currently running
    if (getCurrService() != &service) {
        service.onDisable();
        service._enabled = false;
    } else { // Otherwise queue it
        uint8_t flag = FLAG_DISABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}


void ServiceManager::enable(BaseService &service)
{
    if (getCurrService() != &service) {
        service.onEnable();
        service._enabled = true;
    } else {
        uint8_t flag = FLAG_ENABLE;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}


void ServiceManager::destroy(BaseService &service)
{
    if (getCurrService() != &service) {
        disable(service);
        service.cleanup();
        removeNode(service);
    } else {
        uint8_t flag = FLAG_DESTROY;
        service.getFlagQueue()->add(service.getFlagQueue(), &flag);
    }
}

uint8_t ServiceManager::getID(BaseService &service)
{
    return service.getID();
}

int ServiceManager::run()
{
    // Nothing to run or already running in another call frame
    if (!_head || _active) return 0;
    int count = 0;
    for (_active = _head; _active != NULL ; _active = _active->getNext(), count++)
    {
        uint32_t ts = millis();
        if (_active->isEnabled() &&
            (_active->getPeriod() == SERVICE_CONSTANTLY || ts - _active->getLastRunTS() >= _active->getPeriod()) &&
            (_active->getIterations() == RUNTIME_FOREVER || _active->getIterations() > 0))
        {
            _active->service();
            _active->updateRunTS(ts);

            if (_active->getIterations() > 0)
            {
                _active->decIterations();
                if (_active->getIterations() == 0)
                    _active->disable();
            }
        }

        processFlags(*_active);
    }

    _active = NULL;

    return count;
}

/************ PROTECTED ***************/
void ServiceManager::processFlags(BaseService &node)
{
    if (!_active) return;
    // Process flags
    RingBuf *queue = _active->getFlagQueue();

    uint8_t flag;
    while(queue->pull(queue, &flag)) // Empty Queue
    {
        switch (flag)
        {
            case FLAG_ENABLE:
                if (!_active->isEnabled() && isRunningService(*_active))
                    _active->enable();
                break;

            case FLAG_DISABLE:
                Serial.println("Disabling");
                if (_active->isEnabled() && isRunningService(*_active))
                    _active->disable();
                break;

            case FLAG_DESTROY:
                if (isRunningService(*_active))
                    _active->destroy();
                while(queue->pull(queue, &flag)); // Empty Queue
                break;

            default:
                break;
        }
    }

}


bool ServiceManager::appendNode(BaseService &node)
{
    node.setNext(NULL);

    if (!_head) {
        _head = &node;
    } else {
        BaseService *next = _head;
        for(; next->hasNext(); next = next->getNext()); //run through list
        // Update pointers
        next->setNext(&node);
    }
    return true;
}

bool ServiceManager::removeNode(BaseService &node)
{
    if (&node == _head) { // node is head
        _head = node.getNext();
    } else {
        // Find the previous node
        BaseService *prev = _head;
        for (; prev != NULL && prev->getNext() != &node; prev = prev->getNext());

        if (!prev) return false; // previous node does not exist
        prev->setNext(node.getNext());
    }
    return true;
}


bool ServiceManager::findNode(BaseService &node)
{
    BaseService *prev = _head;
    for (; prev != NULL && prev->getNext() != &node; prev = prev->getNext());
    return prev;
}
