// =============================================================================
// Thermostat : This class abstracts the functionality of an HVAC control system
// and provides a simplistic interface for interacting with it.
// =============================================================================

#include "Temperature.hpp"
#include "Scheduler.hpp"

class Thermostat : protected Scheduler::Event 
{
public:
	enum Mode 
	{
		Off,
		Cooling,
		Heating,
		Auto
	};
	
	enum Perception 		// Really don't like this, the name sucks.
	{
		TemperatureUnit,	// Control based on temperature
		HeatIndexUnit		// Control based on heat index
	};	
	
	virtual Mode mode() const;
	void setMode(const Mode mode = Off);

	// This method returns the temperature as perceived by this instance.
	// Remember that the perception type affects this value.
	virtual Temperature<int> temperature(int const sensorID = 0) const;
	
	virtual Temperature<int> targetTemperature() const;
	virtual void setTargetTemperature(Temperature<int> const  targetTemperature);
	
	Perception perceptionType() const;
	void setPerceptionType(Perception const perceptionType);
	
	//Sensor sensor(unsigned short i = 0); 

	virtual ~Thermostat(Scheduler::Time const updateInterval = 5);
	Thermostat();
	
protected:
	Mode _mode;
	Temperature<int> _temperature;
	Scheduler _scheduler;
	Scheduler::Timer _updateInterval;
	Scheduler::Timer _updateTime;
	
	// Scheduler::Event methods
	Scheduler::Time time() const;
	bool finished() const;
	int execute(Scheduler::Time const updateTime, 
				Scheduler::Time const updateDelay);
};