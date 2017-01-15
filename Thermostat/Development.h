//
//  Development.h
//  ThermostatDebugging
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef Development_h
#define Development_h

#ifndef XCODE_IDE
#warning Uncomment the macro below to simulate/debug on a PC (XCode should automatically do it)

// #define HARDWARE_INDEPENDENT

#warning The code must be modified to compile on Visual Studio due to changes in the Microsoft-provided libraries.
#endif

#ifdef XCODE_IDE
#define HARDWARE_INDEPENDENT
#endif

// Comment or undefine the following macro (3 lines) to strip debug messages.
#ifndef DEBUG
#define DEBUG
#endif

#define CYCLE_LOGS
#define SCHEDULER_LOGS
//#define RTTI_LOGS

#endif /* Development_h */
