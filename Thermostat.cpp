// =============================================================================
// Thermostat : Implementation
// =============================================================================

#include "Thermostat.hpp"

Thermostat::Mode Thermostat::mode() const
{
	return _mode;
}

void Thermostat::setMode(const Thermostat::Mode mode)
{
	_mode = mode;
}

Temperature<int> Thermostat::temperature(int const sensorID) const
{
	
}

void Thermostat::setTargetTemperature(Temperature<int> const targetTemperature)
{
	_temperature = temperature;
}

Scheduler::Time Thermostat::time() const
{
	return _updateTime;
}

bool Thermostat::finished() const
{
	return false;
}

int Thermostat::execute(Scheduler::Time const updateTime, 
						Scheduler::Time const updateDelay)
{
	switch (this->mode())
	{
		case Off:
			break;
		case Cooling:
			break;
		case Heating:
			break;
		case Auto:
			break;
		default:
			break;
	}
	
	_updateTime = updateTime + _updateInterval;
}

Thermostat::~Thermostat()
{
}

Thermostat::Thermostat(Scheduler::Time const updateInterval):
_updateInterval(updateInterval)
{
}