//
//  Sensor.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Sensor_hpp
#define Sensor_hpp

#include <vector>
#include "Development.h"
#include "Actuator.hpp"

// =============================================================================
// Sensor : This class abstracts the functionality of Sensor I/O modules.
// The Sensor class is capable of reading data from a module in the I/O rail.
// =============================================================================
class Sensor : protected Actuator
{
public:
    typedef unsigned char Byte;
    typedef std::vector<Byte> Data;
    
    virtual Data sense();// = 0;
    
    Sensor(Actuator::Pins const &pins);
    virtual ~Sensor();
};

#endif /* Sensor_hpp */

