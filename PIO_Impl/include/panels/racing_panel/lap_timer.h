#pragma once

#include <Diablo_Serial_4DLib.h>

class LapTimer {
private:
    Diablo_Serial_4DLib* display;
    uint16_t posX;
    uint16_t posY;
    uint32_t currentLapTime;
    uint32_t bestLapTime;
    uint32_t lapStartTime;
    bool isActive;

    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t GREEN = 0x07E0;

    void drawTime(uint32_t timeMs) {
        // Clear previous time area
        display->gfx_RectangleFilled(posX - 160, posY, posX, posY + 40, BLACK);
        
        // Format time as M:SS.mmm
        char timeStr[16];
        formatLapTime(timeMs, timeStr);
        
        // Draw new time
        display->txt_Width(3);
        display->txt_Height(3);
        display->txt_FGcolour(WHITE);
        display->gfx_MoveTo(posX - 150, posY + 10);
        display->putStr(timeStr);
    }

    void drawBestLap() {
        // Clear area below current lap time
        display->gfx_RectangleFilled(posX - 160, posY + 50, posX, posY + 80, BLACK);
        
        if (bestLapTime != UINT32_MAX) {
            char timeStr[32];
            char lapTime[16];
            formatLapTime(bestLapTime, lapTime);
            snprintf(timeStr, sizeof(timeStr), "Best: %s", lapTime);
            
            display->txt_Width(2);
            display->txt_Height(2);
            display->txt_FGcolour(GREEN);
            display->gfx_MoveTo(posX - 150, posY + 60);
            display->putStr(timeStr);
        }
    }

    void formatLapTime(uint32_t timeMs, char* buffer) {
        uint32_t minutes = timeMs / 60000;
        uint32_t seconds = (timeMs / 1000) % 60;
        uint32_t millis = timeMs % 1000;
        
        snprintf(buffer, 16, "%d:%02d.%03d", minutes, seconds, millis);
    }

public:
    LapTimer(Diablo_Serial_4DLib* disp)
        : display(disp)
        , posX(380)  // Right side of screen
        , posY(20)   // Top with margin
        , currentLapTime(0)
        , bestLapTime(UINT32_MAX)
        , isActive(false)
    {}

    void draw() {
        // Initial display
        drawTime(0);
    }

    void startLap() {
        lapStartTime = millis();
        isActive = true;
    }

    void updateCurrentLap() {
        if (!isActive) return;
        
        currentLapTime = millis() - lapStartTime;
        drawTime(currentLapTime);
    }

    void completeLap() {
        isActive = false;
        
        // Update best lap if this was faster
        if (currentLapTime > 0 && (bestLapTime == UINT32_MAX || currentLapTime < bestLapTime)) {
            bestLapTime = currentLapTime;
            drawBestLap();
        }
    }

    void reset() {
        isActive = false;
        currentLapTime = 0;
        bestLapTime = UINT32_MAX;
        drawTime(0);
        drawBestLap();
    }
};
