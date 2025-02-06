#include "display/diablo16_driver.h"

Diablo16Driver::Diablo16Driver(uint8_t rxPin, uint8_t txPin, uint8_t resetPin)
    : display(&Serial2)
    , serial(&Serial2)
    , resetPin(resetPin)
    , width(0)
    , height(0)
    , backgroundColor(BLACK)
{
    // Initialize pins early to avoid floating states
    Serial.println("Initializing display pins...");
    pinMode(rxPin, INPUT);
    Serial.println("RX pin set to input");
    pinMode(txPin, OUTPUT);
    Serial.println("TX pin set to output");
    pinMode(resetPin, OUTPUT);
    Serial.println("Reset pin set to output");
    digitalWrite(resetPin, HIGH);  // Start with reset inactive
    Serial.println("Display pins initialized");
}

bool Diablo16Driver::init() {
    Serial.println("Initializing Diablo16 display...");
    
    // Stop any existing serial communication
    if (serial->available()) {
        serial->flush();
    }
    
    // Configure serial with lower baud rate first
    serial->begin(9600, SERIAL_8N1, 18, 17);
    delay(100);  // Give some time for serial to stabilize

    Serial.println("Serial configured, performing hardware reset...");
    
    // Perform hardware reset
    hardwareReset();
    
    // Try to get screen dimensions
    updateScreenDimensions();
    
    if (width == 0 || height == 0) {
        Serial.println("Failed to get screen dimensions!");
        return false;
    }
    
    Serial.printf("Display initialized successfully. Dimensions: %dx%d\n", width, height);
    return true;
}

void Diablo16Driver::clear() {
    Serial.println("Clearing display");
    display.gfx_Cls();
}

void Diablo16Driver::setBackgroundColor(uint16_t color) {
    backgroundColor = color;
    display.gfx_BGcolour(color);
}

uint16_t Diablo16Driver::getWidth() {
    return width;
}

uint16_t Diablo16Driver::getHeight() {
    return height;
}

void Diablo16Driver::hardwareReset() {
    Serial.println("Performing hardware reset...");
    digitalWrite(resetPin, LOW);
    delay(200);  // Increased reset time
    digitalWrite(resetPin, HIGH);
    delay(1000); // Reduced post-reset delay, was 3000
    Serial.println("Hardware reset complete");
}

void Diablo16Driver::updateScreenDimensions() {
    // Add retry mechanism for getting screen dimensions
    Serial.println("Getting screen dimensions...");
    for (int i = 0; i < 3; i++) {
        width = display.gfx_Get(X_MAX);
        Serial.printf("Width: %d\n", width);
        height = display.gfx_Get(Y_MAX);
        Serial.printf("Height: %d\n", height);
        
        if (width > 0 && height > 0) {
            Serial.printf("Screen dimensions: %dx%d\n", width, height);
            break;
        }
        
        Serial.printf("Retry %d getting screen dimensions...\n", i + 1);
        delay(100);
    }
}