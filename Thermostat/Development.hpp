//
//  Development.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef Development_hpp
#define Development_hpp

#define MJB_DEBUG_LOGGING

#if defined(ARDUINO) || defined(__AVR__) || defined(ESP8266)
    #define MJB_DEBUG_LOG_HW_DEPENDENT
    #define MJB_ARDUINO_LIB_API
    #define MJB_HW_IO_PINS_AVAILABLE
#elif defined(__linux__) || defined(__unix__) || defined(__MACH__) || defined(_WIN32)
    #define MJB_MULTITHREAD_CAPABLE
#endif

#if defined(MJB_DEBUG_LOGGING)
    #if defined(MJB_DEBUG_LOG_HW_DEPENDENT)
        #define MJB_DEBUG_LOG_HEX HEX
        #define MJB_DEBUG_LOG_BIN BIN
        #define MJB_DEBUG_LOG_OCT OCT
        #define MJB_DEBUG_LOG_DEC DEC
        #define MJB_DEBUG_LOG(msg) Serial.print(msg)
        #define MJB_DEBUG_LOG_FORMAT(msg, format) Serial.print(msg, format)
        #define MJB_DEBUG_LOG_LINE(msg) Serial.println(msg)
        #define MJB_DEBUG_LOG_LINE_FORMAT(msg, format) Serial.println(msg, format)
    #else
        #include <iostream>
        #define MJB_DEBUG_LOG_HEX std::hex
        #define MJB_DEBUG_LOG_BIN std::hex // No std::bin exists
        #define MJB_DEBUG_LOG_OCT std::oct
        #define MJB_DEBUG_LOG_DEC std::dec
        #define MJB_DEBUG_LOG(msg) std::cout << msg
        #define MJB_DEBUG_LOG_FORMAT(msg, format) MJB_DEBUG_LOG(format << msg)
        #define MJB_DEBUG_LOG_LINE(msg) MJB_DEBUG_LOG(msg) << std::endl
        #define MJB_DEBUG_LOG_LINE_FORMAT(msg, format) MJB_DEBUG_LOG(format << msg) << std::endl
    #endif
#else
    #define MJB_DEBUG_LOG_HEX 0
    #define MJB_DEBUG_LOG(msg)
    #define MJB_DEBUG_LOG_FORMAT(msg, format)
    #define MJB_DEBUG_LOG_LINE(msg)
    #define MJB_DEBUG_LOG_LINE_FORMAT(msg, format)
#endif

#if defined(MJB_DEBUG_LOGGING)

// Uncomment/Comment the following macros to add/remove debug messages.
//#define MJB_DEBUG_LOGGING_CYCLE
//#define MJB_DEBUG_LOGGING_IDENTIFIABLE
//#define MJB_DEBUG_LOGGING_DELEGABLE
//#define MJB_DEBUG_LOGGING_SCHEDULER
//#define MJB_DEBUG_LOGGING_PIN
#define MJB_DEBUG_LOGGING_ACTUATOR
#define MJB_DEBUG_LOGGING_DHT22
#define MJB_DEBUG_LOGGING_THERMOSTAT

#endif

#endif /* Development_hpp */
