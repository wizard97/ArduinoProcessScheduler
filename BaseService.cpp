#include "BaseService.h"
#include "ServiceManager.h"

    /*********** PUBLIC *************/
    BaseService::BaseService(ServiceManager &manager, unsigned int period,
            int iterations, bool enabled)
    : _manager(manager)
    {
        this->_period = period;
        this->_iterations = iterations;
        this->_enabled = enabled;
    }


    void BaseService::disable()
    {
        _manager.disable(*this);
    }


    void BaseService::enable()
    {
        _manager.enable(*this);
    }


    void BaseService::destroy()
    {
        _manager.destroy(*this);
    }

    /* GETTERS */
    int BaseService::getID()
    {
        return _sid;
    }

    ServiceManager &BaseService::getManager()
    {
        return _manager;
    }


    int BaseService::getIterations()
    {
        return _iterations;
    }

    unsigned int BaseService::getPeriod()
    {
        return _period;
    }

    /*********** PROTECTED *************/

    // Fired on creation/destroy
    void BaseService::setup() { return; }
    void BaseService::cleanup() { return; }
    //called on enable/disable
    void BaseService::onEnable() { return; }
    void BaseService::onDisable() { return; }


    /*********** PRIVATE *************/
    bool BaseService::hasNext()
    {
        return _next;
    }


    BaseService *BaseService::getNext()
    {
        return _next;
    }


    void BaseService::setNext(BaseService *next)
    {
        this->_next = next;
    }

    void BaseService::setID(uint8_t sid)
    {
        this->_sid = sid;
    }
