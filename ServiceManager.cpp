#include "ServiceManager.h"
#include "BaseService.h"


ServiceManager::ServiceManager()
{
    _head = NULL;
    _lastID = 0;
}

void ServiceManager::add(BaseService &service)
{
    service.setup();
    service.setID(++_lastID);
    appendNode(service);
}


void ServiceManager::disable(BaseService &service)
{
    service.onDisable();
    service._enabled = false;
}


void ServiceManager::enable(BaseService &service)
{
    service.onEnable();
    service._enabled = true;
}


void ServiceManager::destroy(BaseService &service)
{
    disable(service);
    service.cleanup();
    removeNode(service);
}

uint8_t ServiceManager::getID(BaseService &service)
{
    return service.getID();
}

int ServiceManager::run()
{
    // Nothing to run
    if (!_head) return 0;

    int count = 0;
    for (BaseService *next = _head; next->hasNext(); next = next->getNext(), count++)
    {
        next->service();
    }

    return count;
}

/************ PROTECTED ***************/
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
