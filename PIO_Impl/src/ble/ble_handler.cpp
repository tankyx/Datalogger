// src/ble/ble_handler.cpp
#include "ble/ble_handler.h"
#include <Arduino.h>

class RaceBoxCallbacks : public BLEClientCallbacks {
    BLEHandler& handler;
public:
    RaceBoxCallbacks(BLEHandler& h) : handler(h) {}
    
    void onConnect(BLEClient* client) override {
        handler.bleConnected = true;
        if (handler.connectionCallback) {
            handler.connectionCallback(true);
        }
    }

    void onDisconnect(BLEClient* client) override {
        handler.bleConnected = false;
        if (handler.connectionCallback) {
            handler.connectionCallback(false);
        }
    }
};

class RaceBoxScanCallbacks: public BLEAdvertisedDeviceCallbacks {
    BLEHandler& handler;
public:
    RaceBoxScanCallbacks(BLEHandler& h) : handler(h) {}
    
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        String deviceName = advertisedDevice.getName().c_str();
        if(deviceName.startsWith("RaceBox Micro")) {
            if (handler.pServerAddress == nullptr) {
                handler.pServerAddress = new BLEAddress(advertisedDevice.getAddress());
                log_d("Found RaceBox Micro: %s", deviceName.c_str());
            }
        }
    }
};

BLEHandler::BLEHandler() 
    : bleConnected(false), rbmClient(nullptr), txChar(nullptr), rxChar(nullptr),
      pBLEScan(nullptr), bleState(BLEState::IDLE), pServerAddress(nullptr),
      stateStartTime(0), configurationSent(false), packetBufferIndex(0),
      packetInProgress(false)
{}

BLEHandler::~BLEHandler() {
    cleanup();
}

void BLEHandler::cleanup() {
    if (rbmClient != nullptr) {
        if (rbmClient->isConnected()) {
            rbmClient->disconnect();
        }
        delete rbmClient;
        rbmClient = nullptr;
    }
    
    if (pServerAddress != nullptr) {
        delete pServerAddress;
        pServerAddress = nullptr;
    }
    
    txChar = nullptr;
    rxChar = nullptr;
    bleConnected = false;
    configurationSent = false;
    packetBufferIndex = 0;
    packetInProgress = false;
}

bool BLEHandler::init() {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new RaceBoxScanCallbacks(*this));
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    return true;
}

void BLEHandler::calculateChecksum(uint8_t *packet, size_t length, uint8_t *ckA, uint8_t *ckB) {
    *ckA = 0;
    *ckB = 0;
    
    for (size_t i = 2; i < length - 2; i++) {
        *ckA = *ckA + packet[i];
        *ckB = *ckB + *ckA;
    }
}

void BLEHandler::configureDevice() {
    if (!rxChar || configurationSent) return;

    uint8_t config[] = {
        0xB5, 0x62,       // Header
        0xFF, 0x27,       // Message class and ID
        0x03, 0x00,       // Payload length (3 bytes)
        0x04,             // Platform model (4 = automotive)
        0x00,             // Disable 3D speed (use ground speed)
        0x05,             // Min accuracy (5 = 0.5m)
        0x00, 0x00        // Checksum placeholder
    };
    
    uint8_t ckA, ckB;
    calculateChecksum(config, sizeof(config), &ckA, &ckB);
    config[sizeof(config)-2] = ckA;
    config[sizeof(config)-1] = ckB;

    log_d("Sending config packet with checksum: %02X %02X", ckA, ckB);
    rxChar->writeValue(config, sizeof(config));
    configurationSent = true;
    log_i("RaceBox configuration sent");
}

void BLEHandler::processPacket(uint8_t* pData, size_t length) {
    uint8_t ckA = 0, ckB = 0;
    calculateChecksum(pData, length, &ckA, &ckB);

    if (ckA != pData[length-2] || ckB != pData[length-1]) {
        log_w("Invalid packet checksum");
        return;
    }

    if (pData[2] == 0xFF && pData[3] == 0x01) {
        RaceBoxDataMessage* data = (RaceBoxDataMessage*)(pData + 6);
        if (packetCallback) {
            packetCallback(data);
        }
    }
}

void BLEHandler::handleNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
    for (size_t i = 0; i < length; i++) {
        // Look for packet header
        if (!packetInProgress && pData[i] == 0xB5 && i + 1 < length && pData[i + 1] == 0x62) {
            packetInProgress = true;
            packetBufferIndex = 0;
            packetBuffer[packetBufferIndex++] = 0xB5;
            packetBuffer[packetBufferIndex++] = 0x62;
            i++; // Skip the 0x62 we just processed
            continue;
        }

        // If we're building a packet, add the byte
        if (packetInProgress) {
            packetBuffer[packetBufferIndex++] = pData[i];

            // If we have enough bytes to check the length
            if (packetBufferIndex >= 6) {
                uint16_t payloadLength = packetBuffer[4] | (packetBuffer[5] << 8);
                uint16_t expectedTotalLength = payloadLength + 8;

                // If we have a complete packet
                if (packetBufferIndex == expectedTotalLength) {
                    processPacket(packetBuffer, packetBufferIndex);
                    packetInProgress = false;
                    packetBufferIndex = 0;
                }
                else if (packetBufferIndex > expectedTotalLength || packetBufferIndex >= MAX_PACKET_SIZE) {
                    log_w("Invalid packet or buffer overflow");
                    packetInProgress = false;
                    packetBufferIndex = 0;
                }
            }
        }
    }
}

void IRAM_ATTR BLEHandler::staticNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                              uint8_t* pData, size_t length, bool isNotify) {
    BLEHandler* instance = static_cast<BLEHandler*>(pBLERemoteCharacteristic->getUserData());
    if (instance) {
        instance->handleNotifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

bool BLEHandler::attemptConnection() {
    bool connected = false;
    try {
        // Try with random address type first
        connected = rbmClient->connect(*pServerAddress, BLE_ADDR_TYPE_RANDOM);
        if (!connected) {
            delay(100);
            // If random failed, try public
            connected = rbmClient->connect(*pServerAddress, BLE_ADDR_TYPE_PUBLIC);
        }
    } catch (...) {
        log_e("Connection attempt threw exception");
        connected = false;
    }
    return connected;
}

bool BLEHandler::discoverServices() {
    BLERemoteService* pService = nullptr;
    try {
        pService = rbmClient->getService(UART_SERVICE_UUID);
    } catch (...) {
        log_e("Exception while getting service");
        return false;
    }

    if (pService != nullptr) {
        txChar = pService->getCharacteristic(UART_TX_CHAR_UUID);
        rxChar = pService->getCharacteristic(UART_RX_CHAR_UUID);
        
        if (txChar != nullptr && rxChar != nullptr) {
            txChar->setUserData(this);  // Store this pointer for callback
            txChar->registerForNotify(staticNotifyCallback);
            return true;
        }
    }
    return false;
}

void BLEHandler::update() {
    unsigned long currentMillis = millis();

    switch (bleState) {
        case BLEState::IDLE:
            if (!bleConnected) {
                log_i("Starting scan...");
                if (pBLEScan->start(5, false)) {
                    bleState = BLEState::SCANNING;
                    stateStartTime = currentMillis;
                }
            }
            break;

        case BLEState::SCANNING:
            if (pServerAddress != nullptr) {
                log_i("Device found, attempting connection...");
                delay(100);
                cleanup();
                
                rbmClient = BLEDevice::createClient();
                rbmClient->setClientCallbacks(new RaceBoxCallbacks(*this));
                bleState = BLEState::CONNECTING;
                stateStartTime = currentMillis;
            } else if (currentMillis - stateStartTime > SCAN_TIMEOUT) {
                log_w("Scan timeout");
                pBLEScan->clearResults();
                bleState = BLEState::IDLE;
            }
            break;

        case BLEState::CONNECTING:
            if (attemptConnection()) {
                log_i("Connected, discovering services...");
                delay(200);
                bleState = BLEState::DISCOVERING_SERVICES;
                stateStartTime = currentMillis;
            } else if (currentMillis - stateStartTime > CONNECTION_TIMEOUT) {
                log_w("Connection timeout");
                cleanup();
                bleState = BLEState::IDLE;
            }
            break;

        case BLEState::DISCOVERING_SERVICES:
            if (discoverServices()) {
                log_i("Services discovered successfully");
                bleState = BLEState::CONNECTED;
            } else if (currentMillis - stateStartTime > 5000) {
                log_w("Service discovery timeout");
                cleanup();
                bleState = BLEState::IDLE;
            }
            break;

        case BLEState::CONNECTED:
            if (!bleConnected || !rbmClient->isConnected()) {
                log_w("Connection lost");
                cleanup();
                bleState = BLEState::IDLE;
            } else if (!configurationSent) {
                configureDevice();
            }
            break;
    }
}
