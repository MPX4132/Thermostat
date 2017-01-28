//
//  Sensor.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Sensor.hpp"

// =============================================================================
// Sensor : Implementation
// =============================================================================
Sensor::Data Sensor::sense() {
    this->actuate({}); // Activate timeout.
    return Sensor::Data(); // Empty data
}

Sensor::Sensor(Pin::Arrangement const &pins, Scheduler::Time const senseTimeout):
Actuator(pins, senseTimeout)
{
    
}

Sensor::~Sensor()
{
    
}

