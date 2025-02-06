#include <Arduino.h>
#include "display/sharp_driver.h"
#include "sensors/SensorManager.h"

#define SHARP_SCK  36
#define SHARP_MOSI 35
#define SHARP_SS   5
#define DISPLAY_WIDTH 400
#define DISPLAY_HEIGHT 240

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, DISPLAY_WIDTH, DISPLAY_HEIGHT);
SensorManager sensors;

void displaySensorData(const GNSSData& gnss, const IMUData& imu) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(0x0000);
    
    // GNSS Data
    display.setCursor(10, 10);
    display.print("Lat: "); display.println(gnss.latitude, 6);
    display.setCursor(10, 20);
    display.print("Lon: "); display.println(gnss.longitude, 6);
    
    // IMU Data
    display.setCursor(10, 40);
    display.print("AccelX: "); display.println(imu.accelX);
    
    display.refresh();
}

void setup() {
    // Start with debug serial and wait for it to be ready
    Serial.begin(115200);
    delay(5000);  // Give serial monitor time to connect

    // Initialize display
    display.begin();
    display.clearDisplay();
    
    Serial.println("Initializing sensors...");

    if (!sensors.begin()) {
        Serial.println("Failed to initialize sensors!");
        while (1) delay(10);
    }

    Serial.println("Initializing display...");

    if (!sensors.begin()) {
        Serial.println("Failed to initialize sensors!");
        while (1) delay(10);
    }

    Serial.println("Setup sequence complete!");
}

void loop() {
    static uint32_t lastUpdate = 0;
    const uint32_t UPDATE_INTERVAL = 50; // Update display every 50ms
    
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        if (sensors.isGNSSDataAvailable()) {
            GNSSData gnss = sensors.readGNSS();
            IMUData imu = sensors.readIMU();
            displaySensorData(gnss, imu);
        }
        lastUpdate = millis();
    }
}