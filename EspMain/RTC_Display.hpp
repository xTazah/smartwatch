#include "AbstractDevice.hpp"
#include <ThreeWire.h>
#include <RTClib.h>
#include <array>
#include <mutex>
#include <atomic>

class RTC_Display : public AbstractDevice {
public:
    RTC_Display(String name, int pinD1, int pinD2, int pinD3, int pinD4, // name and pins for segment digits
                int pinLatch, int pinClock, int pinData, int pinIO, int pinSCLK, int pinCE) // pins for shift register and rtc
                : nameSensor(name), rtc(pinCE, pinSCLK, pinIO) {
        D1 = pinD1;
        D2 = pinD2;
        D3 = pinD3;
        D4 = pinD4;
        latchPin = pinLatch;
        clockPin = pinClock;
        dataPin = pinData;
        ioPin = pinIO;
        sclkPin = pinSCLK;
        cePin = pinCE;

        // assign cathode pins manually
        cathodePins[0] = D1;
        cathodePins[1] = D2;
        cathodePins[2] = D3;
        cathodePins[3] = D4;
    }

    String name() override {
        // return name of the sensor
        return nameSensor;
    }

    String type() override {
        // type of this device as a string
        return "4x7_segment_display_shift_register_RTC";
    }

    void init() override;
    void sendToServer(JsonDocument &out) override;
    void recvFromServer(const JsonDocument &in, JsonDocument &out) override;

private:
    void separate() {
        // split time in HHMM format into four digits
        num1 = number / 1000;
        numbers[0] = num1;
        int num1Remove = number - (num1 * 1000);
        num2 = num1Remove / 100;
        numbers[1] = num2;
        int num2Remove = num1Remove - (num2 * 100);
        num3 = num2Remove / 10;
        numbers[2] = num3;
        num4 = num2Remove - (num3 * 10);
        numbers[3] = num4;
    }

    void Display() {
        separate();
        // loop through each digit and display at high frequency
        for (size_t i = 0; i < 4; ++i) {
            screenOff();
            digitalWrite(latchPin, LOW);
            // combine bits (segments) to show the correct digit
            shiftOut(dataPin, clockPin, LSBFIRST, table[numbers[i]]);
            digitalWrite(cathodePins[i], LOW); // activate the digit
            digitalWrite(latchPin, HIGH); // latch the bits
            delay(1);
        }
        screenOff();
    }

    void screenOff() {
        // turn off all digits
        digitalWrite(D4, HIGH);
        digitalWrite(D3, HIGH);
        digitalWrite(D2, HIGH);
        digitalWrite(D1, HIGH);
    }

    void backgroundTask();
    DateTime readDateTime();

    String nameSensor;
    uint16_t number{0};
    // segment display digits
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    int num4 = 0;

    // timer-related event counter
    int timer_event = 0;

    // pins for each digit of the segment display
    int D1;
    int D2;
    int D3;
    int D4;

    // rtc pins
    int sclkPin; // synchronization pin
    int cePin;   // chip enable pin
    int ioPin;   // bidirectional data pin

    // shift register pins
    int latchPin; // sets bits to output
    int clockPin; // shifts bits on rising edge
    int dataPin;  // input data bit to be shifted

    int count = 0;
    std::array<int, 4> numbers;      // individual digits of the number
    std::array<int, 4> cathodePins; // pins for enabling the digits

    // lookup table for digit-to-segment translation (for the shift register)
    std::array<byte, 10> table{B11111100, B01100000, B11011010, B11110010, B01100110, B10110110, B10111110, B11100000, B11111110, B11110110};
    
    // rtc instance
    DS1302 rtc;

    std::mutex mMutex;
};
