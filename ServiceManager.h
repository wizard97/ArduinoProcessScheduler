#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include "Arduino.h"

class BaseService;

class ServiceManager
{

public:
    ServiceManager();
    void add(BaseService &service);
    void disable(BaseService &service);
    void enable(BaseService &service);
    void destroy(BaseService &service);

    uint8_t getID(BaseService &service);

    int run();
protected:
    // Linked list methods
    bool appendNode(BaseService &node); // true on success
    bool removeNode(BaseService &node); // true on success
    bool findNode(BaseService &node); // True if node exists in list

    BaseService *_head;
    uint8_t _lastID;
private:

};

#endif
