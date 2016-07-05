#include "Process.h"
#include "Scheduler.h"

    /*********** PUBLIC *************/
    Process::Process(Scheduler &scheduler, unsigned int period,
            int iterations, int16_t overSchedThresh)
    : _scheduler(scheduler)
    {
        this->_enabled = false;
        this->_period = period;
        this->_iterations = iterations;
        this->_scheduledTS = _scheduler.getCurrTS();
        this->_actualTS = _scheduler.getCurrTS();
        this->_force = false;
        this->_overSchedThresh = overSchedThresh;
        this->_pBehind = 0;
    }



    bool Process::disable()
    {
        return _scheduler.disable(*this);
    }


    bool Process::enable()
    {
        return _scheduler.enable(*this);
    }


    bool Process::destroy()
    {
        return _scheduler.destroy(*this);
    }

    bool Process::add(bool enableIfNot)
    {
        return _scheduler.add(*this, enableIfNot);
    }


    bool Process::needsServicing(uint32_t start)
    {
        return (isEnabled() && (_force ||
            ((getPeriod() == SERVICE_CONSTANTLY || start - getScheduledTS() >= getPeriod()) &&
            (getIterations() == RUNTIME_FOREVER || getIterations() > 0))));
    }

    /* GETTERS */
    int Process::getID()
    {
        return _sid;
    }




    /*********** PROTECTED *************/

    // Fired on creation/destroy
    void Process::setup() { return; }
    void Process::cleanup() { return; }
    //called on enable/disable
    void Process::onEnable() { return; }
    void Process::onDisable() { return; }
    void Process::overScheduledHandler(uint32_t behind) { resetSchedulerWarning(); }

    /*********** PRIVATE *************/
    bool Process::isPBehind(uint32_t curr)
    {
        return (curr - getScheduledTS()) >= getPeriod();
    }


    void Process::willService(uint32_t ts)
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


    // Return true if last if should disable
    bool Process::wasServiced(bool wasForced)
    {
        if (!wasForced && getIterations() > 0) { //Was an iteration
            decIterations();

            if (getIterations() == 0)
                return true;
        }
        return false;
    }


#ifdef _PROCESS_STATISTICS

    uint32_t Process::getAvgRunTime()
    {
        if (!_histIterations)
            return 0;

        return _histRunTime / _histIterations;
    }

    bool Process::statsWillOverflow(HISTORY_COUNT_TYPE iter, HISTORY_TIME_TYPE tm)
    {
        return (_histIterations > _histIterations + iter) || (_histRunTime > _histRunTime + tm);

    }

    void Process::divStats(uint8_t div)
    {
        ++_histIterations /= div;
        ++_histRunTime /= div;
    }

#endif
