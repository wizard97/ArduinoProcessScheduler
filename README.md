# ArduinoProcessScheduler
A cooperative Arduino object oriented, cooperative process scheduler to replace them all.

This fork is adds a wrapper for STM32 (Arduino_STM32) to implement the same basic Process interface, on top of object-oriented [FreeRTOS TaskCpp](https://github.com/GitMoDu/FreeRTOScpp).

Depends on https://github.com/GitMoDu/FreeRTOScpp .

## What is this?
As your Arduino projects get more complicated, you will begin to see the need for multitasking, or at least appear to multitask. Perhaps you want to check if a button was pressed as often as you can, but you only want to update a display once every second. Trying to do this on your own can quickly turn into overwhelming spagetti code involving `millis()`. `ArduinoProcessScheduler` seeks to simplify this. Simply create your custom Process that needs to be serviced at certain times, and let the scheduler handle the rest.

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
- Control over how often a process runs (periodically, iterations, or as often as possible)
- Process priority levels (easily make custom levels as well)
- Dynamically add/remove and enable/disable processes
- Interrupt safe (add, disable, destroy, etc.. processes from interrupt routines)
- Process concurrency protection (Process will always be in a valid state)

### Advanced
- Spawn new processes from within running processes
- Automatic process monitoring statistics (calculates % CPU time for process)
- Truly object oriented (a Process is its own object)
- Exception handling (wait what?!)
- Scheduler can automatically interrupt stuck processes

## Supported Platfroms
- AVR
- ESP8266 (No exception handling or process timeouts)


## Install & Usage 
See [Wiki](https://github.com/wizard97/ArduinoProcessScheduler/wiki)

NOTE: Don't forget to install this [RingBuf](https://github.com/wizard97/ArduinoRingBuffer) library dependency!

## Contributing
I welcome any contributions! Here are some ideas:
- Built in logging
- Built in timers
- Built in process ownership (Library tracks who owns a Process)
- More advanced Process statistics monitoring
- Adding support for additional platforms
- Any Examples!


## License
MIT.
