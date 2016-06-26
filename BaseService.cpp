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
        this->_lastRunTS = 0;
        _flags = RingBuf_new(sizeof(uint8_t), MAX_QUEUED_FLAGS);
    }

    BaseService::~BaseService()
    {
        RingBuf_delete(_flags);
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




    /*********** PROTECTED *************/

    // Fired on creation/destroy
    void BaseService::setup() { return; }
    void BaseService::cleanup() { return; }
    //called on enable/disable
    void BaseService::onEnable() { return; }
    void BaseService::onDisable() { return; }


    /*********** PRIVATE *************/
