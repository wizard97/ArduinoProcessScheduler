# ArduinoProcessScheduler
An Cooperative Arduino Object Oriented Cooperative Process Scheduler to Replace Them All

## What is this?
As your arduino projects get more complicated, you will begin to see the need for multitasking, or at least appear to multitask. Perhaps you want to check if a button was pressed as often as you can, but you only want to update a display once every second. Trying to do this on your own can quickly turn into overwhelming spagetti code involving `millis()`. `ArduinoProcessScheduler` seeks to simplify this. Simply create your custom Process that needs to be serviced at certain times, and let the scheduler handle the rest.

## Why this one?

Here are some similar popular libraries that inspired this one:
- [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
- [Scheduler](https://github.com/arduino-libraries/Scheduler)
- [Arduino-Scheduler](https://github.com/mikaelpatel/Arduino-Scheduler)

### What is wrong with them?

1. They all treat processes/tasks as just callback functions.
  1. Forces you to use ugly global and/or static function variables to track process state.
  2. Limits you to one instance of a process, or lots of copy & paste.
  3. Impossible to truly dynamically create new processes, you are really just enabling/disabling a callback function.
2. Preemptive schedulers must split stack between all processes
  1. With 2K or RAM and 8 processes, preemptive scheduler could at most equally give each Process 2k/8 = 256 Bytes of RAM.
3. No concurrency protection (not interrupt safe)
  1. What if an interrupt fires an tries to disable a process while it is running?


## Features
### Basic
- Control Over How Often a Process Runs (Periodically, Iterations, or as Often as Possible)
- Process Priority Levels (Easily make custom levels as well)
- Dynamically Add/Remove and Enable/Disable Processes
- Interrupt safe (add, disable, destroy, etc.. processes from interrupt routines)
- Process concurrency protection (Process will always be in a valid state)

### Advanced
- Spawn new processes from within running processes
- Automatic Process Monitoring Statistics (calculates % CPU time for process)
- Truly object oriented (a Process is its own object)
- Exception Handling (wait what?!)
- Scheduler can automatically interrupt stuck processes

## Supported Platfroms
- AVR
- ESP8266 (No exception handling or process timeouts)


## Install & Usage
See [Wiki](https://github.com/wizard97/ArduinoProcessScheduler/wiki)

NOTE: Don't forget to install this [RingBufCPP](https://github.com/wizard97/Embedded_RingBuf_CPP) library dependency!

## Contributing
I welcome any contributions! Here are some ideas:
- Built in logging
- Built in timers
- Built in process ownership (Library tracks who owns a Process)
- More advanced Process statistics monitoring
- Adding support for additional platforms
- Any Examples!

## **IMPORTANT** Contributing Guidelines
### There are three rules:

1. Follow the coding format
2. Document all methods
3. **MOST IMPORTANTLY**, all scheduling algorithms must have the following naming convention:
    [Adjective][Person's Name], where both the name and adjective start with the same letter. 
    Bonus points if the name somehow describes your algorithm.

#### Examples (no stealing):
- RoundRobin
- PriorityPaul
- SimpleSam

Rules 1 and 2 are subjective, but rule 3 must absolutely be met or your contribution
will not be accepted. Good luck :)

## License
GNU GPLv3.
