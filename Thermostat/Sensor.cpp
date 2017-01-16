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
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "Called backup sense method (this method should be implemented)!" << std::endl;
#else
    Serial.println("Called backup sense method (this method should be implemented)!");
#endif
#endif
    return Sensor::Data(); // Empty data
}

Sensor::Sensor(Pin::Arrangement const &pins):
Actuator(pins)
{
    
}

Sensor::~Sensor()
{
    
}

