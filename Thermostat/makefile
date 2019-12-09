all: Program

compiler = g++
flags = -std=c++11 -O3 -Wall -Wextra -Wno-unknown-pragmas

Accessible.o: Accessible.cpp Accessible.hpp Development.hpp
	$(compiler) $(flags) -c Accessible.cpp

Identifiable.o: Identifiable.cpp Identifiable.hpp Development.hpp
	$(compiler) $(flags) -c Identifiable.cpp

Delegable.o: Delegable.cpp Delegable.hpp Development.hpp
	$(compiler) $(flags) -c Delegable.cpp

Temperature.o: Temperature.cpp Temperature.hpp Development.hpp
	$(compiler) $(flags) -c Temperature.cpp

Pin.o: Pin.cpp Pin.hpp Accessible.o
	$(compiler) $(flags) -c Pin.cpp

Scheduler.o: Scheduler.cpp Scheduler.hpp Identifiable.o Accessible.o Delegable.o
	$(compiler) $(flags) -c Scheduler.cpp

Actuator.o: Actuator.cpp Actuator.hpp Scheduler.o Pin.o
	$(compiler) $(flags) -c Actuator.cpp

Sensor.o: Sensor.cpp Sensor.hpp Actuator.o
	$(compiler) $(flags) -c Sensor.cpp

Thermometer.o: Thermometer.cpp Thermometer.hpp Temperature.o Sensor.o
	$(compiler) $(flags) -c Thermometer.cpp

DHT22.o: DHT22.cpp DHT22.hpp Thermometer.o
	$(compiler) $(flags) -c DHT22.cpp

Thermostat.o: Thermostat.cpp Thermostat.hpp Thermometer.o Scheduler.o
	$(compiler) $(flags) -c Thermostat.cpp

Tester.o: Tester.cpp Thermostat.o DHT22.o
	$(compiler) $(flags) -c Tester.cpp

Program: Tester.o Thermostat.ino
	mkdir -p bin
	$(compiler) $(flags) Tester.o Thermostat.o DHT22.o Thermometer.o Sensor.o Actuator.o Scheduler.o Pin.o Temperature.o Delegable.o Identifiable.o Accessible.o -o bin/Thermostat
	chmod u+x bin/Thermostat

clean:
	rm -rfv *.o bin/*
