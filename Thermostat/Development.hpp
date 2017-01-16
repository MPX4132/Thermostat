//
//  Development.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef Development_hpp
#define Development_hpp

#ifndef XCODE_IDE
#warning Uncomment the macro below to simulate/debug on a PC (XCode should automatically do it)

// #define HARDWARE_INDEPENDENT

#warning The code must be modified to compile on Visual Studio due to changes in the Microsoft-provided libraries.
#endif

#ifdef XCODE_IDE
#define HARDWARE_INDEPENDENT
#endif

// Comment/Undefine the following macro (3 lines) to strip debug messages.
#ifndef DEBUG
#define DEBUG
#endif

// Uncomment/Comment the following macros to add/remove debug messages.
//#define PIN_LOGS
//#define CYCLE_LOGS
//#define SCHEDULER_LOGS
//#define RTTI_LOGS
//#define DHT22_LOGS

#endif /* Development_h */
