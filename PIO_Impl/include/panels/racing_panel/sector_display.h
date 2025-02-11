#pragma once

#include <Diablo_Serial_4DLib.h>

class SectorDisplay {
private:
    Diablo_Serial_4DLib* display;
    uint16_t startX;
    uint16_t startY;
    uint16_t width;
    uint16_t height;
    uint32_t bestSectorTimes[3];
    uint32_t currentSectorTimes[3];
    uint32_t theoreticalBest;

    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t RED = 0xF800;
    static constexpr uint16_t GREEN = 0x07E0;

    void drawSectorRow(uint8_t sector) {
        uint16_t y = startY + (sector * 50);
        
        // Clear row area
        display->gfx_RectangleFilled(startX, y, startX + width, y + 40, BLACK);
        
        // Draw sector label
        display->txt_Width(2);
        display->txt_Height(2);
        display->txt_FGcolour(WHITE);
        display->gfx_MoveTo(startX + 10, y + 10);
        char label[4];
        snprintf(label, sizeof(label), "S%d:", sector + 1);
        display->putStr(label);
        
        // Draw best time
        if (bestSectorTimes[sector] != UINT32_MAX) {
            char timeStr[16];
            formatTime(bestSectorTimes[sector], timeStr);
            display->gfx_MoveTo(startX + 60, y + 10);
            display->putStr(timeStr);
        } else {
            display->gfx_MoveTo(startX + 60, y + 10);
            display->putStr("--.---");
        }
        
        // Draw delta if we have current time
        if (currentSectorTimes[sector] != UINT32_MAX && 
            bestSectorTimes[sector] != UINT32_MAX) {
            
            int32_t delta = currentSectorTimes[sector] - bestSectorTimes[sector];
            char deltaStr[16];
            if (delta > 0) {
                snprintf(deltaStr, sizeof(deltaStr), "+%.3f", delta / 1000.0f);
                display->txt_FGcolour(RED);
            } else {
                snprintf(deltaStr, sizeof(deltaStr), "%.3f", delta / 1000.0f);
                display->txt_FGcolour(GREEN);
            }
            display->gfx_MoveTo(startX + width - 80, y + 10);
            display->putStr(deltaStr);
        }
    }

    void drawTheoretical() {
        uint16_t y = startY + height - 40;
        
        // Clear theoretical area
        display->gfx_RectangleFilled(startX, y, startX + width, y + 30, BLACK);
        
        // Draw label
        display->txt_Width(2);
        display->txt_Height(2);
        display->txt_FGcolour(WHITE);
        display->gfx_MoveTo(startX + 10, y + 5);
        display->putStr("Best:");

        // Draw theoretical best time if valid
        if (theoreticalBest > 0) {
            char timeStr[16];
            formatTime(theoreticalBest, timeStr);
            display->gfx_MoveTo(startX + 80, y + 5);
            display->txt_FGcolour(GREEN);
            display->putStr(timeStr);
        } else {
            display->gfx_MoveTo(startX + 80, y + 5);
            display->putStr("--:--.---");
        }
    }

    void updateTheoretical() {
        theoreticalBest = 0;
        bool validTheoretical = true;
        
        for (int i = 0; i < 3; i++) {
            if (bestSectorTimes[i] != UINT32_MAX) {
                theoreticalBest += bestSectorTimes[i];
            } else {
                validTheoretical = false;
                break;
            }
        }
        
        drawTheoretical();
    }

    void formatTime(uint32_t timeMs, char* buffer) {
        uint32_t minutes = timeMs / 60000;
        uint32_t seconds = (timeMs / 1000) % 60;
        uint32_t millis = timeMs % 1000;
        
        if (minutes > 0) {
            snprintf(buffer, 16, "%d:%02d.%03d", minutes, seconds, millis);
        } else {
            snprintf(buffer, 16, "%d.%03d", seconds, millis);
        }
    }

public:
    SectorDisplay(Diablo_Serial_4DLib* disp)
        : display(disp)
        , startX(20)
        , startY(120)
        , width(240)
        , height(240)
        , theoreticalBest(0)
    {
        // Initialize sector times
        for (int i = 0; i < 3; i++) {
            bestSectorTimes[i] = UINT32_MAX;
            currentSectorTimes[i] = UINT32_MAX;
        }
    }

    void draw() {
        // Draw background panel
        display->gfx_RectangleFilled(startX, startY, startX + width, startY + height, BLACK);
        
        // Draw sector labels
        for (int i = 0; i < 3; i++) {
            drawSectorRow(i);
        }
        
        // Draw theoretical best time
        drawTheoretical();
    }

    void updateSector(uint8_t sector, uint32_t currentTime, uint32_t bestTime) {
        if (sector >= 3) return;
        
        currentSectorTimes[sector] = currentTime;
        if (bestTime < bestSectorTimes[sector]) {
            bestSectorTimes[sector] = bestTime;
        }
        
        // Redraw the sector row
        drawSectorRow(sector);
        
        // Update theoretical best
        updateTheoretical();
    }
};
