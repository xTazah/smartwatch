#pragma once

#include "AbstractDevice.hpp"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <array>
#include <vector>

#define BS_LCD_COLUMNS 16
#define BS_LCD_ROWS 2
#define BS_LCD_SCROLL_TICK_RATE 1000

class LCDDisplay : public AbstractDevice {
public:

    String name() override;
    String type() override;

    void init() override;
    
    void sendToServer(JsonDocument& out) override;
    void recvFromServer(const JsonDocument& in, JsonDocument& out) override;
    void loop() override;
    void wifiDisonnected() override;
    void wifiConnected() override;
private:
    // display all rows
    void displayText();
    // display a specific row
    void displayRow(size_t rowIndex);
    
    LiquidCrystal_I2C lcd{0x27,BS_LCD_COLUMNS,BS_LCD_ROWS};
    std::vector<String> currentDisplayMessages;
    size_t mRowOffset = 0;
    unsigned long mLastScrollTick = 0;
    std::array<size_t, BS_LCD_ROWS> mScrollColumnOffset;
};

