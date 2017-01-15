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

#ifdef HARDWARE_INDEPENDENT
#include <iostream>

Scheduler::Time micros() {
    static Scheduler::Time fakeTime = 0;
    return fakeTime++;
}


int main(int argc, const char * argv[]) {
    
    Thermostat::Thermometers thermometers = {Thermometer({14})};
    Thermostat thermostat({2,12,13}, thermometers);
    
    thermostat.setTargetTemperature(Temperature<float>(70));
    thermostat.setMode(Thermostat::Mode::Cooling);
    
    for (;;) {
        // Scheduler at microsecond resolution.
        Scheduler::Time const now = micros();
        Scheduler::UpdateInstances(now);
        
#ifdef CYCLE_LOGS
        std::cout << "[Cycle] Completed at: " << std::dec << now << std::endl;
#endif
    }
    
    return 0;
}
#endif
