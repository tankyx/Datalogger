#pragma once

#include <Diablo_Serial_4DLib.h>
#include <math.h>

class Speedometer {
private:
    Diablo_Serial_4DLib* display;
    uint16_t centerX;
    uint16_t centerY;
    uint16_t radius;
    int32_t lastSpeed;
    int lastIndicatorX;
    int lastIndicatorY;

    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t GREEN = 0x07E0;
    static constexpr uint16_t DARK_GRAY = 0x4208;

    void drawArc() {
        // Draw outer arc (135° to 405° = 270° span)
        const int segments = 90;  // Number of segments for smooth arc
        const float startAngle = 135 * M_PI / 180;
        const float endAngle = 405 * M_PI / 180;
        const float angleStep = (endAngle - startAngle) / segments;

        // Draw background arc
        for (int i = 0; i < segments; i++) {
            float angle1 = startAngle + (i * angleStep);
            float angle2 = angle1 + angleStep;
            
            int x1 = centerX + radius * cos(angle1);
            int y1 = centerY + radius * sin(angle1);
            int x2 = centerX + radius * cos(angle2);
            int y2 = centerY + radius * sin(angle2);
            
            display->gfx_Line(x1, y1, x2, y2, DARK_GRAY);
        }

        // Draw speed markers
        for (int speed = 0; speed <= 100; speed += 20) {
            float angle = startAngle + (speed / 100.0f) * (endAngle - startAngle);
            int markerX = centerX + (radius - 10) * cos(angle);
            int markerY = centerY + (radius - 10) * sin(angle);
            
            display->gfx_Circle(markerX, markerY, 2, WHITE);
            
            // Draw speed text
            char speedText[4];
            snprintf(speedText, sizeof(speedText), "%d", speed);
            display->txt_Width(1);
            display->txt_Height(1);
            display->gfx_MoveTo(markerX - 8, markerY - 8);
            display->putStr(speedText);
        }
    }

    void updateArcIndicator(int32_t speed) {
        const float startAngle = 135 * M_PI / 180;
        const float endAngle = 405 * M_PI / 180;
        
        // Clamp speed between 0 and 100
        speed = (speed < 0) ? 0 : ((speed > 100) ? 100 : speed);
        
        // Calculate angle for current speed
        float speedAngle = startAngle + (speed / 100.0f) * (endAngle - startAngle);
        
        // Clear previous indicator area
        display->gfx_CircleFilled(lastIndicatorX, lastIndicatorY, 5, BLACK);
        
        // Calculate new indicator position
        int indicatorX = centerX + (radius - 20) * cos(speedAngle);
        int indicatorY = centerY + (radius - 20) * sin(speedAngle);
        
        // Draw new indicator
        display->gfx_CircleFilled(indicatorX, indicatorY, 5, GREEN);
        
        // Store position for next update
        lastIndicatorX = indicatorX;
        lastIndicatorY = indicatorY;
    }

public:
    Speedometer(Diablo_Serial_4DLib* disp)
        : display(disp)
        , centerX(200)
        , centerY(240)
        , radius(120)
        , lastSpeed(0)
        , lastIndicatorX(0)
        , lastIndicatorY(0)
    {}

    void draw() {
        drawArc();
        updateSpeed(0);
    }

    void updateSpeed(int32_t speedKph) {
        if (speedKph == lastSpeed) {
            return;
        }

        // Clear previous text area
        display->gfx_RectangleFilled(centerX - 50, centerY - 20,
                                   centerX + 50, centerY + 20,
                                   BLACK);

        // Draw new speed
        char speedText[8];
        snprintf(speedText, sizeof(speedText), "%d", speedKph);
        
        display->txt_Width(3);
        display->txt_Height(3);
        display->txt_FGcolour(WHITE);
        
        // Center text
        uint16_t textWidth = strlen(speedText) * 18;  // Approximate width
        display->gfx_MoveTo(centerX - (textWidth/2), centerY - 15);
        display->putStr(speedText);
        
        // Draw "km/h" below
        display->txt_Width(1);
        display->txt_Height(1);
        display->gfx_MoveTo(centerX - 15, centerY + 10);
        display->putStr("km/h");

        // Update arc indicator
        updateArcIndicator(speedKph);
        
        lastSpeed = speedKph;
    }
};
