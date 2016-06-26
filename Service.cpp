#include "Service.h"
#include "Scheduler.h"

    /*********** PUBLIC *************/
    Service::Service(Scheduler &scheduler, unsigned int period,
            int iterations, bool enabled)
    : _scheduler(scheduler)
    {
        this->_period = period;
        this->_iterations = iterations;
        this->_enabled = enabled;
        this->_scheduledTS = _scheduler.getCurrTS();
        this->_actualTS = _scheduler.getCurrTS();
        _flags = RingBuf_new(sizeof(uint8_t), MAX_QUEUED_FLAGS);
    }

    Service::~Service()
    {
        RingBuf_delete(_flags);
    }



    void Service::disable()
    {
        _scheduler.disable(*this);
    }


    void Service::enable()
    {
        _scheduler.enable(*this);
    }


    void Service::destroy()
    {
        _scheduler.destroy(*this);
    }

    /* GETTERS */
    int Service::getID()
    {
        return _sid;
    }




    /*********** PROTECTED *************/

    // Fired on creation/destroy
    void Service::setup() { return; }
    void Service::cleanup() { return; }
    //called on enable/disable
    void Service::onEnable() { return; }
    void Service::onDisable() { return; }


    /*********** PRIVATE *************/
