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
        this->_locked = false;
        _flags = RingBuf_new(sizeof(uint8_t), MAX_QUEUED_FLAGS);
    }

    Service::~Service()
    {
        RingBuf_delete(_flags);
    }



    SchedulerAction Service::disable()
    {
        return _scheduler.disable(*this);
    }


    SchedulerAction Service::enable()
    {
        return _scheduler.enable(*this);
    }


    SchedulerAction Service::destroy()
    {
        return _scheduler.destroy(*this);
    }

    SchedulerAction Service::add()
    {
        return _scheduler.add(*this);
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



#ifdef _SERVICE_STATISTICS

    uint32_t Service::getAvgRunTime()
    {
        if (!_histIterations)
            return 0;

        return _histRunTime / _histIterations;
    }

    bool Service::statsWillOverflow(HISTORY_COUNT_TYPE iter, HISTORY_TIME_TYPE tm)
    {
        return (_histIterations > _histIterations + iter) || (_histRunTime > _histRunTime + tm);

    }

    void Service::divStats(uint8_t div)
    {
        ++_histIterations /= div;
        ++_histRunTime /= div;
    }

#endif
