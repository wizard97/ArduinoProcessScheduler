/*
* Example 01: Ex_01_SayHello.ino
* By: D. Aaron Wisner
*
* In this example we are creating a "SayHelloProcess" Class that prints "Hello from Process: 'id'" at a constant period
*/

#include <ProcessScheduler.h>

// Create my custom process SayHelloProcess
class SayHelloProcess : public Process
{
public:
    // Call the Process constructor
    SayHelloProcess(Scheduler &manager, ProcPriority pr, unsigned int period, int iterations)
        :  Process(manager, pr, period, iterations) {}

protected:
    // Create our service routine
    virtual void service()
    {
        Serial.print("Hello from Process: ");
        Serial.println(getID());
    }
};

Scheduler sched; // Create a global Scheduler object

// Create our high priority process that will get serviced as often as possible and run forever
SayHelloProcess myProc(sched, HIGH_PRIORITY, SERVICE_CONSTANTLY, RUNTIME_FOREVER);

void setup()
{
    Serial.begin(9600);
    // Add our process to the scheduler
    myProc.add();
    //enable it
    myProc.enable();
}

void loop()
{
    sched.run();
}
