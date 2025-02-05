#include <Arduino.h>
#include "display/diablo16_driver.h"
#include "sensors/SensorManager.h"

#define DISPLAY_RESET_PIN 4
#define DISPLAY_RX_PIN 18  // GPIO18 for RX
#define DISPLAY_TX_PIN 17  // GPIO17 for TX

Diablo16Driver* display = nullptr;
SensorManager sensors;

void setup() {
    // Start with debug serial and wait for it to be ready
    Serial.begin(115200);
    delay(5000);  // Give serial monitor time to connect
    
    Serial.println("Initializing sensors...");

    if (!sensors.begin()) {
        Serial.println("Failed to initialize sensors!");
        while (1) delay(10);
    }

    Serial.println("Initializing display...");
    
    // Initialize display
    display = new Diablo16Driver(DISPLAY_RX_PIN, DISPLAY_TX_PIN, DISPLAY_RESET_PIN);
    if (!display->init()) {
        Serial.println("Failed to initialize display!");
        while (1) delay(10);
    }
    
    // Set white background and clear screen
    display->setBackgroundColor(WHITE);
    display->clear();
    
    if (!sensors.begin()) {
        Serial.println("Failed to initialize sensors!");
        while (1) delay(10);
    }

    Serial.println("Setup sequence complete!");
}

void loop() {
    // Read GNSS data (when available)
    if (sensors.isGNSSDataAvailable()) {
        GNSSData gnssData = sensors.readGNSS();
        if (gnssData.isValid) {
            Serial.printf("Lat: %.6f, Lon: %.6f, Speed: %.2f m/s\n", 
                gnssData.latitude, gnssData.longitude, gnssData.speed);
        }
    }
    
    // Read IMU data
    IMUData imuData = sensors.readIMU();
    Serial.printf("AccelX: %.2f m/s², GyroZ: %.2f rad/s\n", 
        imuData.accelX, imuData.gyroZ);
    
    delay(10); // Adjust based on your needs
}