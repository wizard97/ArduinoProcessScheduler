# ArduinoProcessScheduler
An Arduino Object Oriented Cooperative Process Scheduler to Replace Them All

## Features
- Fine Grained Control Over How Often a Process Runs (Periodically, Iterations, or as Often as Possible)
- Exception Handling (wait what?!)
- Atomicity Support
- Dynamically Add/Remove and Enable/Disable Processes
- Automatic Process Monitoring Statistics (Automatically calculates % CPU time for service)
- Truly object oriented (a Process is no longer just a callback function like other libraries)

## Basic Usage
There are two classes to worry about: `Scheduler` and `Process`.

### Class `Scheduler`:
A `Scheduler` oversees and manages `Processes`. There should only be one `Scheduler` in your project. Inside of `void loop()`, the scheduler's `run()` method should be repeatedly called.

### Class `Process`:
A service can be thought of as job or a task that needs to be run at certain times. Each Process had be scheduled to run Periodically
Constantly, or Periodically/Constantly with a set number of run iterations.

Each project will have multiple services, here are some examples of `Processes`:
  - A Process to handle user input
  - A Process to run a basic webserver
  - A Process to update a display

`Process` is a base class that should be extended to handle your particular Process (such as the examples mentioned above). The scheduler's will call the following virtual function as an entry point when it is time for you service to run:
```
    virtual void service();
```
From here your `service()` routine is expected to be non-blocking, and return from `service()` as soon as possible (that means no long `delay()`'s). Fortunately, since your service routine is a method inside a class, you can store the state of your service in custom class attributes before returning, so you know where you left off when the scheduler calls your Process again. As soon you return from `service()`, control is transferred back to the Scheduler (it will also be transferred back if you raise an Exception--advanced).

Additionally, each Process inherits the following virtual functions which are called by the `Scheduler` at the appropriate times. Note you are guaranteed that only one of these methods will exist in the call stack at once, even if an interrupt fires trying to modify this Process at the same time (such as with `add()`, `destroy()`, `enable()`, or `disable()`). This also includes the `service()` method mentioned above. You do not have to worry about concurrency!

**Thanks to the magic of virtual functions, you don't have to impliment any of these unless your particular Process needs them:**
```
    virtual void setup()
    virtual void cleanup()

    virtual void onEnable()
    virtual void onDisable()

    virtual void overScheduledHandler(uint32_t behind)
```

#### `setup()`
This method is called by the scheduler when the Process is being added to the scheduling chain with `add()`, it is guaranteed to only be called once when a Process not part of the scheduling chain is being added (`add()`). Keep in mind it will still be called if the Process is removed from the scheduling chain with `destroy()`, then added again with `add()`.

#### `cleanup()`
This is only called once when a `Process` part of the scheduling chain is being removed from the scheduling chain with `destroy()`. This method should undo whatever was done in `setup()`. You are guaranteed it will not be called unless `setup()` was called previously.

#### `onEnable()`
This method is called only once when a disabled task is being enabled with `enable()`.

#### `onDisable()`
This method is called only once when a enabled task is being disabled with `disable()`. You are guaranteed it will not be called unless `onEnable()` was called previously. It should undo whatever `onEnable()` did.

#### `overScheduledHandler(uint32_t behind)`
This method is called if the scheduler can not meet the current set period for the Process and is falling behind. The scheduler will pass in variable `behind` containing how many milliseconds (or microseconds) behind the scheduler is with this task. Inside this method might be a good time increase the period between when this task is run, then call `resetSchedulerWarning()` to clear the warning.


## API
### `Scheduler` Methods:



#### Constructor
```
Scheduler()
```
Create a Scheduler object.

**Returns:** `Scheduler`
___

#### run()
```
run()
```
This method runs one pass through the scheduler. It is the heart of the scheaduler, this method should be called repeatedly inside of `void loop()`

**Returns:** `void`
___


#### getID()
```
getID(Process &service)
```
Get the unique id for the service. Same as `service.getID()`.

**Returns:** `uint8_t`
___

#### isRunningProcess()
```
isRunningProcess(Process &service)
```
Determine if the scheduler is in the middle of running this service. Same as calling `service.isRunningProcess()`

**Returns:** `bool`
___

#### isEnabled()
```
isEnabled(Process &service)
```
Determines if this service is enabled. Same as calling `service.isEnabled()`

**Returns:** `bool`
___

#### getCurrProcess()
```
getCurrProcess()
```
Get pointer to the current service being run by the scheduler. Return NULL if no service being currently run.

**Returns:** `Process*`
___

#### countProcesses()
```
countProcesses(bool enabledOnly = true)
```
Get number of servies in the scheduler's chain. If enabledOnly is true, only the enabled Processes will be counted.

**Returns:** `uint8_t`
___

#### getCurrTS()
```
getCurrTS()
```
Returns the current internal timestamp. Either will be the same as `millis()` or `micros()`

**Returns:** `uint32_t`
___


#### add()
```
add (Process &service))
```
Add the service to the scheduler, same as calling `service.add()`. Note, this will trigger the service's `setup()` method to fire. This method can only fail inside an interrupt routine, particularly when an interrupt interrupts any call to either `add()` or `remove()`. It will also fail if the service is already added.

**Returns:** type `SchedulerAction`, `ACTION_NONE` on failure, `ACTION_SUCCESS` on success.
___

#### destroy()
```
destroy (Process &service))
```
Remove the service from the scheduler. Same as calling `service.destroy()`. Note, this will trigger the service's `cleanup()` method to fire. If the service is not disabled, it will first call `disable()`. This method can only fail inside an interrupt routine, particularly when an interrupt interrupts any call to either `add()` or `remove()`. It will first try and QUEUE the request for later, before failing. It will also fail if the service is already destroyed.

**Returns:** type `SchedulerAction`, `ACTION_NONE` on failure, `ACTION_QUEUED` on Queuing it, `ACTION_SUCCESS` on success.
___

#### enable()
```
enable (Process &service))
```
Enable a service, same as calling `service.enable()`. Note, this will trigger the service's `onEnable()` method to fire. This method call will always succeed if it was not called on itself from a method inside of this service. If it was, the scheduler will queue the request. Also, the request will fail if the service is already enabled or is destroyed.

**Returns:** type `SchedulerAction`, `ACTION_NONE` on failure, `ACTION_QUEUED` on Queuing it, `ACTION_SUCCESS` on success.
___

#### disable()
```
disable (Process &service))
```
Disable a service, same as calling `service.disable()`. Note, this will trigger the service's `onDisable()` method to fire. This method call will always succeed if it was not called on itself from a method inside of this service. If it was, the scheduler will queue the request. Also, the request will fail if the service is already enabled or is destroyed.

**Returns:** type `SchedulerAction`, `ACTION_NONE` on failure, `ACTION_QUEUED` on Queuing it, `ACTION_SUCCESS` on success.
___
