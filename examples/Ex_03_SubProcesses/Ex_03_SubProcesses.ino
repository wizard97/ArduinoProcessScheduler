/*
* Example 03: Ex_03_SubProcesses.ino
* By: GitModu
*
* This example expands the Ex_02_MultiBlink.ino example, and demonstrates sub-instancing of Processes inside a class.
* This allows class definition with a custom Process without needing extra static declarations, sharing the same scheduler.
* BlinkProcess has 3 BlinkProcess objects that will blink at a diff. period and add them to the scheduler:
* - blink250 (pin 13)
* - blink500 (pin 12)
* - blink1000 (pin 11)
* Connect an LED to each of these pins and watch them blink.
* Also added a simple LogProcess to demonstrate the usage of the same scheduler.
*/

#include <ProcessScheduler.h>

// Create my custom Blink Process
class BlinkProcess : public Process
{
public:
	// Call the Process constructor
	BlinkProcess(Scheduler &manager, ProcPriority pr, unsigned int period, int pin)
		: Process(manager, pr, period)
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

class BlinkerProcesses
{
public:
	// Call the Processes constructor's
	BlinkerProcesses(Scheduler &manager) :
		blink250(manager, HIGH_PRIORITY, 250, 13)
		, blink500(manager, HIGH_PRIORITY, 500, 12)
		, blink1000(manager, HIGH_PRIORITY, 1000, 11)
	{

	}

	void Start()
	{
		// Add and enable our blink processes
		blink250.add(true); // Same as calling blink250.add() and blink250.enable();
		blink500.add(true);
		blink1000.add(true);
	}

	void Stop()
	{
		blink250.disable();
		blink500.disable();
		blink1000.disable();
	}


private:
	// Create our blink processes
	BlinkProcess blink250;// Blink 13 every 250 ms
	BlinkProcess blink500; // Blink 12 every 500 ms
	BlinkProcess blink1000; // Blink 11 every 1000 ms
};

class LogProcess : public Process
{
public:
	LogProcess(Scheduler &manager, ProcPriority pr, unsigned int period)
		: Process(manager, pr, period)
	{
		
	}
protected:
	//setup the pins
	virtual void setup()
	{
		Serial.begin(57600);
	}

	// Create our service routine
	virtual void service()
	{
		Serial.println(F("Log Message."));
	}
};

Scheduler sched; // Create a global Scheduler object

BlinkerProcesses Blinkers(sched); // Has 3 independent BlinkProcesses.
LogProcess LogService(sched, LOW_PRIORITY, 5000); // Show a log message every 5000 ms

void setup()
{
	// Add and enable our blink processes and custom process, sharing the scheduler.
	LogService.add(true);
	Blinkers.Start();
}

void loop()
{
	sched.run();
}
