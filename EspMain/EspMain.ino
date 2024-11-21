#ifndef BASE_STATION

#include "Application.hpp"
#include "HumidityTempSensor.hpp"
#include "WaterSensor.hpp"
#include "AccGyroSensor.hpp"
#include "RTC_Display.hpp"

HumidityTempSensor humidityTempSensor;
WaterSensor waterSensor;
AccGyroSensor gyroSensor{"acc-gyro-sensor-1"};
RTC_Display rtcDisplay{"rtc-display-1",2,15,19,4,5,18,23,12,14,13};

void setup() {
    Serial.begin(115200);

    Application::get().addDevice(&humidityTempSensor);
    Application::get().addDevice(&waterSensor);
    Application::get().addDevice(&gyroSensor);
    Application::get().addDevice(&rtcDisplay);
    Application::get().init("esp-smartwatch");
}

void loop() {
    Application::get().loop();
}

#else



#include "Application.hpp"
#include "LCDDisplay.hpp"

LCDDisplay lcd;

void setup() {
    Application::get().addDevice(&lcd);
    Application::get().init("esp-basestation");
}

void loop() {
    Application::get().loop();
}


#endif