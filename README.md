# ArduinoServiceScheduler
An Arduino Object Oriented Service Scheduler to Replace Them All

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
  
`Service` is a base class that should extended to handle your particular Service (such as the examples mentioned above). Each instance of 
`Service` will inherit the following virtual functions:
```
virtual void setup()
```
