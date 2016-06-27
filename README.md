# ArduinoServiceScheduler
An Arduino Object Oriented Cooperative Service Scheduler to Replace Them All

## Features
- Fine Grained Control Over How Often a Service Runs (Periodically, Iterations, or as Often as Possible)
- Exception Handling (wait what?!)
- Atomicity Support
- Dynamically Add/Remove and Enable/Disable Services
- Automatic Service Monitoring Statsitics
- Truly object orinted (a Service is no longer just a callback function like other libraries)

## Basic Usage
There are two classes to worry about: `Scheduler` and `Service`.

### Class `Service`:
A service can be thought of as job or a task that needs to be run at certain times. Each Service had be scheduled to run Periodically
Constantly, or Periodically/Constantly with a set number of run iterations.

Each project will have multiple services, here are some examples of `Services`:
  - A Service to handle user input
  - A Service to run a basic webserver
  - A Service to update a display
  
`Service` is a base class that should extended to handle your particular Service (such as the examples mentioned above). The scheduler's will call the following virtual function as an entry point when it is time for you service to run:
```
    virtual void service();
```
From here your `service()` routine is expected to be non-blocking, and return from `service()` as soon as possible (that means no long `delay()`'s). Fortunately, since your service routine is a method inside a class, you can store the state of your service in custom class attributes before returning, so you know where you left off when the scheduler calls your Service again. As soon you return from `service()`, control is transferred back to the Scheduler (it will also be transferred back if you raise an Exception--advanced).

Additionally, each Service inherits the following virtual functions which are called by the `Scheduler` at the appropriate times. Note you are guaranteed that only one of these methods will exist in the call stack at once, even if an interrupt fires trying to modify this Service at the same time (such as with `add()`, `destroy()`, `enable()`, or `disable()`). This also includes the `service()` method mentioned above. You do not have to worry about concurrency!

Thanks to the magic of virtual functions, you don't have to impliment them unless your particular Service needs them:
```
    virtual void setup()
    virtual void cleanup()
    
    virtual void onEnable()
    virtual void onDisable()
    
    virtual void overScheduledHandler(uint32_t behind)
```

#### `setup()`
This method is called by the scheduler when the Service is being added to the scheduling chain with `add()`, it is guaranteed to only be called once when a Service not part of the scheduling chain is being added (`add()`). Keep in mind it will still be called if the Service is removed from the scheduling chain with `destroy()`, then added again with `add()`. 

#### `cleanup()`
This is only called once when a `Service` part of the scheduling chain is being removed from the scheduling chain with `destroy()`. This method should undo whatever was done in `setup()`. You are guaranteed it will not be called unless `setup()` was called previously.

#### `onEnable()`
This method is called only once when a disabled task is being enabled with `enable()`. 

#### `onDisable()`
This method is called only once when a enabled task is being disabled with `disable()`. You are guaranteed it will not be called unless `onEnable()` was called previously. It should undo whatever `onEnable()` did.

#### `overScheduledHandler(uint32_t behind)`
This method is called if the scheduler can not meet the current set period for the Service and is falling behind. The scheduler will pass in variable `behind` containing how many milliseconds (or microseconds) behind the scheduler is with this task. Inside this method might be a good time increase the period between when this task is run, then call `resetSchedulerWarning()` to clear the warning.
