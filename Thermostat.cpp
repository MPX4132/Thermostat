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

bool Thermostat::finished() const
{
	return false;
}

bool Thermostat::execute(Scheduler::Time const updateTime, 
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
}

Thermostat::~Thermostat()
{
}

Thermostat::Thermostat()
{
}