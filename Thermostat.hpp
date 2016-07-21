// =============================================================================
// Thermostat : This class abstracts the functionality of an HVAC control system
// and provides a simplistic interface for interacting with it.
// =============================================================================

#include "Temperature.hpp"

class Thermostat {
public:
	enum Mode {
		Off,
		Cooling,
		Heating,
		Auto
	};
	
	enum Measurement {
		HI,
		Temp
	};
	
	virtual Mode mode() const;
	void setMode(const Mode mode = Off);

	virtual Temperature<int> temperature() const;
	void setTemperature(const Temperature<int>& temperature);
	
	//Sensor sensor(unsigned short i = 0); 

	virtual ~Thermostat();
	Thermostat();
	
protected:
	Mode _mode;
	Temperature<int> _temperature;
};