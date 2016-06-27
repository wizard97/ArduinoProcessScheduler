#include "Service.h"
#include "Scheduler.h"

    /*********** PUBLIC *************/
    Service::Service(Scheduler &scheduler, unsigned int period,
            int iterations, bool enabled, int16_t overSchedThresh)
    : _scheduler(scheduler)
    {
        this->_period = period;
        this->_iterations = iterations;
        this->_enabled = enabled;
        this->_scheduledTS = _scheduler.getCurrTS();
        this->_actualTS = _scheduler.getCurrTS();
        this->_locked = false;
        this->_force = false;
        this->_overSchedThresh = overSchedThresh;
        this->_pBehind = overSchedThresh;
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

    bool Service::needsServicing(uint32_t start)
    {
        return (isEnabled() && (_force ||
            ((getPeriod() == SERVICE_CONSTANTLY || start - getScheduledTS() >= getPeriod()) &&
            (getIterations() == RUNTIME_FOREVER || getIterations() > 0))));
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
    void Service::overScheduledHandler(uint32_t behind) { resetSchedulerWarning(); }

    /*********** PRIVATE *************/
    bool Service::isPBehind(uint32_t curr)
    {
        return (curr - getScheduledTS()) >= getPeriod();
    }


    void Service::willService(uint32_t ts)
    {
        if (!_force)
        {
            if (getPeriod() != SERVICE_CONSTANTLY)
                setScheduledTS(getScheduledTS() + getPeriod());
            else
                setScheduledTS(ts);
        } else {
            _force = false;
        }

        setActualTS(ts);
    }


    bool Service::wasServiced(bool wasForced)
    {
        if (!wasForced && getIterations() > 0) { //Was an iteration
            decIterations();

            if (getIterations() == 0)
                return true;
        }
        return false;
    }


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
