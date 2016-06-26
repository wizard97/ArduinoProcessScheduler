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

    inline uint8_t getID(BaseService &service);
    inline bool isRunningService(BaseService &service);
    inline bool isNotDestroyed(BaseService &service);
    inline bool isEnabled(BaseService &service);

    inline BaseService *getCurrService();


    int run();
protected:
    // Methods that can be called while inside a service
    void setDisable(BaseService &service);
    void setEnable(BaseService &service);
    void setDestroy(BaseService &service);

    void processFlags(BaseService &node);
    // Linked list methods
    bool appendNode(BaseService &node); // true on success
    bool removeNode(BaseService &node); // true on success
    bool findNode(BaseService &node); // True if node exists in list

    BaseService *_head;
    BaseService *_active;
    uint8_t _lastID;
private:

};

#endif
