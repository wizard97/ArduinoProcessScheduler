#ifndef ROUND_ROBIN_h
#define ROUND_ROBIN_h

#include "Includes.h"

namespace SchedulingAlgorithms
{
    class RoundRobin
    {
    public:
        RoundRobin(Scheduler &sched, SchedulerPriorityLevel *pLevels)
        : _sched(sched), _pLevels(pLevels)
        {
            _cPrior = 0;
        }

        Process *getNext()
        {
            // Make sure cPrior is valid
            for (; !_pLevels[_cPrior] && _cPrior < NUM_PRIORITY_LEVELS; _cPrior++);

            if (!_pLevels[_cPrior])
            {
                _cPrior = 0;
                return NULL;
            }

            uint32_t start = _sched.getCurrTS();

            Process *p = _pLevels[_cPrior].next;

            for (; _cPrior < NUM_PRIORITY_LEVELS; _cPrior++)
            {
                if (_pLevels[_cPrior].head) { //Found process
                    while(_pLevels)
                }

            }


                _active = _pLevels[pLevel].next;
                if (!_active)
                    continue;

                /////////// Run the correct process /////////
                uint32_t start = getCurrTS();

                if (_active->needsServicing(start))
                {
                    bool force = _active->forceSet(); // Store whether it was a forced iteraiton
                    _active->willService(start);

        #ifdef _PROCESS_EXCEPTION_HANDLING
                    int ret = setjmp(_env);

        #ifdef _PROCESS_TIMEOUT_INTERRUPTS
                    ENABLE_SCHEDULER_ISR();
        #endif
                    if (!ret) {
                        _active->service();
                    } else {
                        jmpHandler(ret);
                    }
        #else
                    _active->service();
        #endif

        #ifdef _PROCESS_TIMEOUT_INTERRUPTS
                    DISABLE_SCHEDULER_ISR();
        #endif

        #ifdef _PROCESS_STATISTICS
                    uint32_t runTime = getCurrTS() - start;
                    // Make sure no overflow happens
                    if (_active->statsWillOverflow(1, runTime))
                        handleHistOverFlow(HISTORY_DIV_FACTOR);

                    _active->setHistIterations(_active->getHistIterations()+1);
                    _active->setHistRuntime(_active->getHistRunTime()+runTime);

        #endif
                    // Is it time to disable?
                    if (_active->wasServiced(force)) {
                        disable(*_active);
                    }

                    count++; // incr counter
                }
                //////////////////////END PROCESS SERVICING//////////////////////
                delay(0); // For esp8266

                // Determine what to do next ///
                if (!_active->hasNext()) {
        #ifdef _PROCESS_REORDERING
                    if(++_pLevels[pLevel].passes >= _PROCESS_REORDERING_AGGRESSIVENESS) {
                        reOrderProcs((ProcPriority)pLevel);
                        ++_pLevels[pLevel].passes = 0;
                    }
        #endif
                    _pLevels[pLevel].next = _pLevels[pLevel].head; // Set next to first
                } else {
                    _pLevels[pLevel].next = _active->getNext(); // Set next and break
                    _active = NULL;
                    break;
                }

                _active = NULL;
            }
            processQueue();
            return count;
        }

    protected:
        Process *findPStart()
        {
            Process *start = _sched.getPLevels();

            for (uint8_t i = 0; !start && i < NUM_PRIORITY_LEVELS; i++, start++);

            return start;

        }

        uint8_t _cPrior;
        Scheduler &const _sched;
        SchedulerPriorityLevel *const _pLevels;
    }
}

#endif
