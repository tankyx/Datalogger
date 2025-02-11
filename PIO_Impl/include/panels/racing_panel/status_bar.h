#pragma once

#include <Diablo_Serial_4DLib.h>

class StatusBar {
private:
    Diablo_Serial_4DLib* display;
    uint16_t startY;
    uint16_t height;
    bool gpsValid;
    uint8_t satelliteCount;
    bool rbmConnected;

    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t RED = 0xF800;
    static constexpr uint16_t GREEN = 0x07E0;
    static constexpr uint16_t DARK_GRAY = 0x4208;

public:
    StatusBar(Diablo_Serial_4DLib* disp)
        : display(disp)
        , startY(440)  // Bottom of screen
        , height(40)   // Status bar height
        , gpsValid(false)
        , satelliteCount(0)
        , rbmConnected(false)
    {}

    void draw() {
        // Draw background
        display->gfx_RectangleFilled(0, startY, 480, startY + height, DARK_GRAY);
        
        // Draw initial status
        updateGPSStatus(gpsValid, satelliteCount);
        updateRBMStatus(rbmConnected);
        updateStintTimer("00:00");
    }

    void updateGPSStatus(bool valid, uint8_t satCount) {
        gpsValid = valid;
        satelliteCount = satCount;
        
        // Clear GPS status area
        display->gfx_RectangleFilled(10, startY + 5, 160, startY + height - 5, DARK_GRAY);
        
        // Draw GPS status
        display->txt_Width(1);
        display->txt_Height(1);
        display->txt_FGcolour(valid ? GREEN : RED);
        
        char status[32];
        snprintf(status, sizeof(status), "GPS: %s | Sats: %d", 
                valid ? "Yes" : "No", satCount);
        
        display->gfx_MoveTo(20, startY + 12);
        display->putStr(status);
    }

    void updateRBMStatus(bool connected) {
        rbmConnected = connected;
        
        // Clear RBM status area
        display->gfx_RectangleFilled(170, startY + 5, 320, startY + height - 5, DARK_GRAY);
        
        // Draw RBM status
        display->txt_Width(1);
        display->txt_Height(1);
        display->txt_FGcolour(connected ? GREEN : RED);
        
        char status[32];
        snprintf(status, sizeof(status), "RBM: %s", connected ? "Connected" : "Disconnected");
        
        display->gfx_MoveTo(180, startY + 12);
        display->putStr(status);
    }

    void updateStintTimer(const char* time) {
        // Clear stint timer area
        display->gfx_RectangleFilled(330, startY + 5, 470, startY + height - 5, DARK_GRAY);
        
        // Draw stint time
        display->txt_Width(1);
        display->txt_Height(1);
        display->txt_FGcolour(WHITE);
        
        char status[32];
        snprintf(status, sizeof(status), "Stint: %s", time);
        
        display->gfx_MoveTo(340, startY + 12);
        display->putStr(status);
    }
};
