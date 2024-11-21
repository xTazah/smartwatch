#include "LCDDisplay.hpp"
#include <IRremote.h>
#include <cmath>
#include <WiFi.h>

#define BS_IR_RECEIVER_PIN 27

String LCDDisplay::name() {
    return "lcd_display_1";
}
String LCDDisplay::type() {
    return "lcd_display";
}

void LCDDisplay::init() {
    lcd.init(); 
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    IrReceiver.begin(BS_IR_RECEIVER_PIN);
    Serial.println("LCD Display init");
}

void LCDDisplay::sendToServer(JsonDocument& out) {
    for(size_t i = 0; i < currentDisplayMessages.size(); ++i) {
        out["rows"][i] = currentDisplayMessages.at(i);
    }
}
void LCDDisplay::recvFromServer(const JsonDocument& in, JsonDocument& out) {

    if(in["command"] == "set_text") {
        if(!in.containsKey("rows")) {
            out["type"] = "error";
            out["msg"] = "missing rows";
            return;
        }

        if(in["rows"].size() == 0) {
            out["type"] = "error";
            out["msg"] = "need at least one row";
            return;
        }
        
        auto oldRows = std::move(currentDisplayMessages);
        currentDisplayMessages = {};
        for(int i = 0; i < in["rows"].size(); ++i) {
            auto rowText = in["rows"][i].as<const char*>();
            if(rowText != nullptr) {
                currentDisplayMessages.emplace_back(rowText);
            } else {
                out["type"] = "error";
                out["msg"] = "rows need to be strings";
                return;
            }
        }
        // only do something is the rows changed
        if(oldRows != currentDisplayMessages) {
            if(mRowOffset >= currentDisplayMessages.size()) {
                mRowOffset = currentDisplayMessages.size() - 1;
            }
            //mScrollColumnOffset.fill(0);
            for(size_t i = 0; i < mScrollColumnOffset.size(); ++i) {
                mScrollColumnOffset[i] = std::min(mScrollColumnOffset[i], currentDisplayMessages[i - mRowOffset].length());
                if(currentDisplayMessages[i - mRowOffset].length() < BS_LCD_COLUMNS) {
                    mScrollColumnOffset[i] = 0;
                }
            }
            mLastScrollTick = millis();
            displayText();
        }
    }
}

void LCDDisplay::wifiConnected() {
    lcd.clear();
    currentDisplayMessages = {"IP: ", WiFi.localIP().toString()};
    displayText();
    Serial.println("LCD Display wifi connected");
}

void LCDDisplay::wifiDisonnected() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No WiFi");
}

void LCDDisplay::loop() {
    if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            Serial.println("Overflow detected");
        } else {
            if(IrReceiver.decodedIRData.command == 0x44 && IrReceiver.decodedIRData.decodedRawData != 0) {
                // left arrow clicked on remote
                if(currentDisplayMessages.empty()) {
                    mRowOffset = 0;
                } else if(mRowOffset == 0) {
                    mRowOffset = currentDisplayMessages.size() - 1;
                } else {
                    mRowOffset -= 1;
                }
                mScrollColumnOffset.fill(0);
                mLastScrollTick = millis();
                displayText();
                
            } else if(IrReceiver.decodedIRData.command == 0x40 && IrReceiver.decodedIRData.decodedRawData != 0) {
                // right arrow clicked on remote
                if(currentDisplayMessages.empty()) {
                    mRowOffset = 0;
                } else if(mRowOffset == currentDisplayMessages.size() - 1) {
                    mRowOffset = 0;
                } else {
                    mRowOffset += 1;
                }
                mScrollColumnOffset.fill(0);
                mLastScrollTick = millis();
                displayText();
            } else {
                Serial.println("Unknown ir command: 0x" + String(IrReceiver.decodedIRData.command, 16));
            }

        }
        IrReceiver.resume();
    }
    for(size_t i = mRowOffset; i < currentDisplayMessages.size() && i < mRowOffset + BS_LCD_ROWS; ++i) {
        // scroll all lines horizontally
        if(currentDisplayMessages.at(i).length() > BS_LCD_COLUMNS && (millis() - mLastScrollTick) > BS_LCD_SCROLL_TICK_RATE) {
            mLastScrollTick = millis();
            size_t index = i - mRowOffset;
            mScrollColumnOffset[index] += 1;
            if(mScrollColumnOffset[index] >= (currentDisplayMessages.at(i).length())) {
                mScrollColumnOffset[index] = 0;
            }
            displayRow(i);
        }
    }
}

void LCDDisplay::displayText() {
    lcd.clear();
    for(size_t i = mRowOffset; i < currentDisplayMessages.size() && i < mRowOffset + BS_LCD_ROWS; ++i) {
        displayRow(i);
    }
}
void LCDDisplay::displayRow(size_t rowIndex) {
    size_t displayRow = rowIndex - mRowOffset;
    lcd.setCursor(0, displayRow);

    // this funky logic is for scrolling the text horizontally
    char toPrint[BS_LCD_COLUMNS + 1];
    memset(toPrint, ' ', BS_LCD_COLUMNS);
    toPrint[BS_LCD_COLUMNS] = 0;
    int32_t existingCharsCount = std::min<int32_t>(currentDisplayMessages.at(rowIndex).length() - mScrollColumnOffset[displayRow], BS_LCD_COLUMNS);
    memcpy(toPrint, currentDisplayMessages.at(rowIndex).c_str() + mScrollColumnOffset[displayRow], existingCharsCount);
    lcd.print(toPrint);
}