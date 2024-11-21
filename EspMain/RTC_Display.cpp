#include "RTC_Display.hpp"

void RTC_Display::init() {
    // setup pins for digits
    pinMode(D4, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D1, OUTPUT);

    // setup pins for shift register
    pinMode(clockPin, OUTPUT); // shifts bits into register on rising edge
    pinMode(latchPin, OUTPUT); // sets all bits to output
    pinMode(dataPin, OUTPUT);  // data line for the bit being shifted

    // turn off all digit pins by default
    digitalWrite(D4, HIGH);
    digitalWrite(D3, HIGH);
    digitalWrite(D2, HIGH);
    digitalWrite(D1, HIGH);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    std::unique_lock<std::mutex> lock{mMutex}; // lock rtc to prevent parallel access
    rtc.begin();
    if (!rtc.isrunning()) {
        rtc.adjust(DateTime(__DATE__, __TIME__));
    }
    lock.unlock();

    TaskHandle_t Task2;

    // run background task (displaying time on segment display) on another core
    xTaskCreatePinnedToCore(
        [](void* self) {
            auto selfCasted = (RTC_Display*)self;
            selfCasted->backgroundTask();
        },
        "update_display_task", 10000, this, 1, &Task2, 0);
}

void RTC_Display::backgroundTask() {
    // display current time on the segment display
    while (true) {
        auto now = readDateTime();

        number = now.hour() * 100 + now.minute(); // format time as HHMM
        if (number > 9999) {
            Serial.println("invalid time");
            continue;
        }
        Display();
    }
}

DateTime RTC_Display::readDateTime() {
    // get current time from rtc
    std::unique_lock<std::mutex> lock{mMutex};
    return rtc.now();
}

void RTC_Display::recvFromServer(const JsonDocument& in, JsonDocument& out) {
    // set rtc time from server for better accuracy
    if (in["command"] == "set_unix_time") {
        std::unique_lock<std::mutex> lock{mMutex};
        DateTime dt;
        dt.setunixtime(in["unixtime"].as<uint32_t>());
        rtc.adjust(dt);
        Serial.println("adjusted time to " + String(dt.hour()) + ":" + String(dt.minute()));
    }
}

void RTC_Display::sendToServer(JsonDocument& out) {
    // send unix time and displayed time (HH:MM) to server
    out["unixtime"] = readDateTime().unixtime();
    out["display_number"] = number;
}
