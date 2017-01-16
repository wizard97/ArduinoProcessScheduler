/*
* Example 02: Ex_02_MultiBlink.ino
* By: D. Aaron Wisner
*
* In this example we are creating a "BlinkProcess" Class that will toggle a pin at a constant period
* We create 3 BlinkProcess objects that will blink at a diff. period and add them to the scheduler:
* - blink250 (pin 13)
* - blink500 (pin 12)
* - blink1000 (pin 11)
* Connect an LED to each of these pins and watch them blink
*/

#include <ProcessScheduler.h>

// Create my custom Blink Process
class BlinkProcess : public Process
{
public:
    // Call the Process constructor
    BlinkProcess(Scheduler &manager, ProcPriority pr, unsigned int period, int pin)
        :  Process(manager, pr, period)
        {
          _pinState = LOW; // Set the default state
          _pin = pin; // Store the pin number
        }

protected:
    //setup the pins
    virtual void setup()
    {
      pinMode(_pin, OUTPUT);
      _pinState = LOW;
      digitalWrite(_pin, _pinState);
    }

     // Undo setup()
    virtual void cleanup()
    {
      pinMode(_pin, INPUT);
      _pinState = LOW;
    }
    
    //LEDs should be off when disabled
    virtual void onDisable()
    {
      _pinState = LOW;
      digitalWrite(_pin, _pinState);
    }
    
    //Start the LEDs on
    virtual void onEnable()
    {
      _pinState = HIGH;
      digitalWrite(_pin, _pinState);
    }

    // Create our service routine
    virtual void service()
    {
      // If pin is on turn it off, otherwise turn it on
      _pinState = !_pinState;
      digitalWrite(_pin, _pinState);
    }

private:
    bool _pinState; //the Current state of the pin
    int _pin; // The pin the LED is on
};

Scheduler sched; // Create a global Scheduler object

// Create our blink processes
BlinkProcess blink250(sched, HIGH_PRIORITY, 250, 13); // Blink 13 every 250 ms
BlinkProcess blink500(sched, HIGH_PRIORITY, 500, 12); // Blink 12 every 500 ms
BlinkProcess blink1000(sched, HIGH_PRIORITY, 1000, 11); // Blink 11 every 1000 ms

void setup()
{
  // Add and enable our blink processes
  blink250.add(true); // Same as calling blink250.add() and blink250.enable();
  blink500.add(true);
  blink1000.add(true);
}

void loop()
{
    sched.run();
}
