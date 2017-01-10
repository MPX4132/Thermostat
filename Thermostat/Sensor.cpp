//
//  Sensor.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Sensor.hpp"

// =============================================================================
#pragma mark - Sensor : Implementation
// =============================================================================
Sensor::Data Sensor::sense() {
    return static_cast<Sensor::Data>(0);
}

Sensor::Sensor(Actuator::Pins const &pins):
Actuator(pins)
{
    
}

Sensor::~Sensor()
{
    
}

