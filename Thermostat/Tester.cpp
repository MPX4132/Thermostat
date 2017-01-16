//
//  Tester.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#include "Development.hpp"
#include "Thermostat.hpp"
#include "Scheduler.hpp"
#include "Sensor.hpp"
#include "DHT22.hpp"

#ifdef HARDWARE_INDEPENDENT
#include <iostream>

Scheduler::Time micros() {
    static Scheduler::Time fakeTime = 0;
    return fakeTime++;
}


int main(int argc, const char * argv[]) {
    
    /*Thermostat::Thermometers thermometers = {Thermometer({14})};
    Thermostat thermostat({14,12,13}, thermometers);
    
    thermostat.setTargetTemperature(Temperature<float>(70));
    thermostat.setMode(Thermostat::Mode::Cooling);*/
    
    DHT22 thermometer(2);
    
    for (;;) {
        // Scheduler at microsecond resolution.
//        Scheduler::Time const now = micros();
//        Scheduler::UpdateInstances(now);
    
        Sensor::Data data = thermometer.sense();
        
        if (!data.size())
        {
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
            std::cout << "FAILURE!" << std::endl;
#else
            Serial.println("FAILURE!");
#endif
#endif
        }
        
#ifdef CYCLE_LOGS
        std::cout << "[Cycle] Completed at: " << std::dec << now << std::endl;
#endif
    }
    
    return 0;
}
#endif
