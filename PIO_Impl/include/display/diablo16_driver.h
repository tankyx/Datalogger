#pragma once

#include "display_driver.h"
#include <Diablo_Serial_4DLib.h>

class Diablo16Driver : public IDisplay {
public:
    Diablo16Driver(uint8_t rxPin, uint8_t txPin, uint8_t resetPin);
    ~Diablo16Driver() override = default;

    // IDisplay implementation
    bool init() override;
    void clear() override;
    void setBackgroundColor(uint16_t color) override;
    uint16_t getWidth() override;
    uint16_t getHeight() override;

private:
    Diablo_Serial_4DLib display;
    HardwareSerial* serial;
    uint8_t resetPin;
    uint16_t width;
    uint16_t height;
    uint16_t backgroundColor;

    void hardwareReset();
    void updateScreenDimensions();
};