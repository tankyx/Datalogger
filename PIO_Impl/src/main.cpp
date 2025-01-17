#include <Arduino.h>
#include <Diablo_Serial_4DLib.h>
#include <BLEDevice.h>
#include "ble/ble_handler.h"

// Global instance of BLE handler
BLEHandler* bleHandler = nullptr;

// Function declarations
void printMemoryInfo();
void onRaceBoxData(const RaceBoxDataMessage* data);
void onConnectionChange(bool connected);

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("DataLogger Starting...");

    // Print memory information
    printMemoryInfo();

    // Initialize BLE handler
    bleHandler = new BLEHandler();
    if (!bleHandler->init()) {
        Serial.println("Failed to initialize BLE");
        return;
    }

    // Set callbacks
    bleHandler->setPacketCallback(onRaceBoxData);
    bleHandler->setConnectionCallback(onConnectionChange);

    Serial.println("Setup complete");
}

void loop() {
    static uint32_t lastPrint = 0;
    uint32_t now = millis();

    // Update BLE handler
    if (bleHandler) {
        bleHandler->update();
    }

    // Print status every second
    if (now - lastPrint >= 1000) {
        Serial.printf("BLE Status: %s | Time: %lu\n", 
                     bleHandler && bleHandler->isConnected() ? "Connected" : "Disconnected",
                     now);
        lastPrint = now;
    }

    delay(10); // Small delay to prevent watchdog issues
}

void printMemoryInfo() {
    Serial.println("Memory Info:");
    Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("Total heap: %d bytes\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Minimum free heap: %d bytes\n", ESP.getMinFreeHeap());
}

// Callback functions
void onRaceBoxData(const RaceBoxDataMessage* data) {
    // Process received data
    double lat = data->latitude / 10000000.0;
    double lon = data->longitude / 10000000.0;
    int32_t speed_kph = (int32_t)(data->speed * 0.0036f); // Convert mm/s to km/h
    bool has_fix = (data->fixStatus == 3) && (data->fixStatusFlags & 0x01);
    
    // Print some basic info
    Serial.printf("Position: %.6f, %.6f | Speed: %d km/h | Fix: %s | Sats: %d\n",
                 lat, lon, speed_kph, has_fix ? "Yes" : "No", data->numSV);
}

void onConnectionChange(bool connected) {
    Serial.printf("RaceBox connection status: %s\n", connected ? "Connected" : "Disconnected");
}