#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include "display/display_driver.h"

class SharpDisplay : public IDisplay {
public:
    SharpDisplay(uint8_t ssPin, uint8_t sckPin, uint8_t mosiPin, uint16_t width = 400, uint16_t height = 240) 
        : display(ssPin, width, height) {
        SPI.begin(sckPin, -1, mosiPin, ssPin);  // SCK, MISO, MOSI, SS
    }

    bool init() override {
        display.begin();
        display.clearDisplay();
        return true;
    }

    void clear() override {
        display.clearDisplay();
        display.refresh();
    }

    void setBackgroundColor(uint16_t color) override {
        display.fillScreen(color ? BLACK : WHITE);
        display.refresh();
    }

    uint16_t getWidth() override { return display_width; }
    uint16_t getHeight() override { return display_height; }

    void drawText(int16_t x, int16_t y, const char* text, uint8_t size = 1, bool color = false) {
        display.setTextSize(size);
        display.setTextColor(color ? BLACK : WHITE);
        display.setCursor(x, y);
        display.print(text);
        display.refresh();
    }

private:
    Adafruit_SharpMem display;
    uint16_t display_width;
    uint16_t display_height;
};