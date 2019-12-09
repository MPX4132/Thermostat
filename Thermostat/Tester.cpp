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

Scheduler::Time micros()
{
    static Scheduler::Time fakeTime = 0;
    return fakeTime += 20;
}

int main(int argc, const char * argv[]) {
    setup();
    for (;;) loop();
    return 0;
}

#endif
