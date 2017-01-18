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

Scheduler::Time micros();

DHT22 * thermometer;
Thermostat * thermostat;

void setup()
{
    thermometer = new DHT22(2);
    thermostat = new Thermostat({14,12,13}, {thermometer});
    
    thermostat->setTargetTemperature(Temperature<float>(72, Temperature<float>::Scale::Fahrenheit));
    thermostat->setMode(Thermostat::Mode::Auto);
}

void loop()
{
    Scheduler::Time const now = micros();
    Scheduler::UpdateInstances(now);
}

int main(int argc, const char * argv[]) {
    setup();
    for (;;) loop();
    return 0;
}

Scheduler::Time micros()
{
    static Scheduler::Time fakeTime = 0;
    return fakeTime++;
}

#endif
