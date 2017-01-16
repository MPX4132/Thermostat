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
    return Sensor::Data(); // Empty data
}

Sensor::Sensor(Actuator::Pins const &pins):
Actuator(pins)
{
    
}

Sensor::~Sensor()
{
    
}

