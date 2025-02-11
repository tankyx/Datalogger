#pragma once

#include <Diablo_Serial_4DLib.h>

class DeltaBar {
private:
    Diablo_Serial_4DLib* display;
    uint16_t barWidth;
    uint16_t barHeight;
    uint16_t centerX;
    uint16_t centerY;
    
    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t RED = 0xF800;
    static constexpr uint16_t GREEN = 0x07E0;

    void drawDeltaText(const char* text, uint16_t color) {
        display->txt_Width(2);
        display->txt_Height(2);
        uint16_t textWidth = strlen(text) * 12;  // Approximate width
        display->txt_FGcolour(color);
        display->gfx_MoveTo(centerX - (textWidth/2), centerY + (barHeight/2) - 12);
        display->putStr(text);
    }
    
    static uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

public:
    DeltaBar(Diablo_Serial_4DLib* disp) 
        : display(disp)
        , barWidth(180)
        , barHeight(60)
        , centerX(200)  // Center position
        , centerY(40)   // Top of screen with margin
    {}

    void draw() {
        // Draw background bar
        display->gfx_RectangleFilled(centerX - barWidth, centerY,
                                   centerX + barWidth, centerY + barHeight,
                                   BLACK);
        
        // Draw center divider
        display->gfx_Line(centerX, centerY, centerX, centerY + barHeight, WHITE);
        
        // Draw status text
        drawDeltaText("0.000", GREEN);
    }

    void update(int32_t deltaMs) {
        const uint16_t maxDelta = 1500;  // Max delta to show (1.5 seconds)
        
        // Clear previous bar
        display->gfx_RectangleFilled(centerX - barWidth, centerY,
                                   centerX + barWidth, centerY + barHeight,
                                   BLACK);
        
        // Calculate bar width based on delta
        uint16_t deltaWidth = map(abs(deltaMs), 0, maxDelta, 0, barWidth);
        
        // Choose color based on delta
        uint16_t barColor = (deltaMs < 0) ? GREEN : RED;
        
        // Draw the bar on appropriate side
        if (deltaMs < 0) {
            // Faster - Green bar on right
            display->gfx_RectangleFilled(centerX, centerY,
                                       centerX + deltaWidth, centerY + barHeight,
                                       barColor);
        } else {
            // Slower - Red bar on left
            display->gfx_RectangleFilled(centerX - deltaWidth, centerY,
                                       centerX, centerY + barHeight,
                                       barColor);
        }
        
        // Draw delta text
        char deltaText[16];
        if (deltaMs >= 0) {
            snprintf(deltaText, sizeof(deltaText), "+%.3f", deltaMs / 1000.0f);
        } else {
            snprintf(deltaText, sizeof(deltaText), "%.3f", deltaMs / 1000.0f);
        }
        drawDeltaText(deltaText, barColor);
    }
};
