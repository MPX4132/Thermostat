//
//  Tester.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#include "Development.hpp"

#if ! defined(MJB_ARDUINO_LIB_API)

#include "Scheduler.hpp"
#include "Thermostat.ino"

constexpr Scheduler::Time TimeIncrement = 1; //static_cast<uint32_t>(static_cast<float>(4294967296) / 100);

Scheduler::Time micros()
{
    static Scheduler::Time fakeTime = 0; //((~static_cast<uint32_t>(0)) - 50);
    return fakeTime += TimeIncrement;
}

int main(int argc, const char * argv[]) {
    // The following done to suppress unused variable warnings.
    (void) argc;
    (void) argv;
    setup();
    for (;;) loop();
    return 0;
}

#endif
