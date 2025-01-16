#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <functional>
#include "racebox_types.h"

// UUIDs from RaceBox documentation
#define UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_CHAR_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_TX_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

enum class BLEState {
    IDLE,
    SCANNING,
    CONNECTING,
    DISCOVERING_SERVICES,
    CONNECTED
};

class BLEHandler {
public:
    using PacketCallback = std::function<void(const RaceBoxDataMessage*)>;
    using ConnectionCallback = std::function<void(bool)>;

    BLEHandler();
    ~BLEHandler();

    bool init();
    void update();
    bool isConnected() const { return bleConnected; }
    void setPacketCallback(PacketCallback cb) { packetCallback = cb; }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback = cb; }

private:
    static constexpr size_t MAX_PACKET_SIZE = 256;
    static constexpr uint32_t CONNECTION_TIMEOUT = 10000; // 10 seconds
    static constexpr uint32_t SCAN_TIMEOUT = 5000; // 5 seconds

    bool bleConnected;
    BLEClient* rbmClient;
    BLERemoteCharacteristic* txChar;
    BLERemoteCharacteristic* rxChar;
    BLEScan* pBLEScan;
    BLEState bleState;
    BLEAddress* pServerAddress;
    unsigned long stateStartTime;
    bool configurationSent;
    
    // Packet reassembly
    uint8_t packetBuffer[MAX_PACKET_SIZE];
    size_t packetBufferIndex;
    bool packetInProgress;

    // Callbacks
    PacketCallback packetCallback;
    ConnectionCallback connectionCallback;

    void processPacket(uint8_t* data, size_t length);
    void configureDevice();
    void calculateChecksum(uint8_t* packet, size_t length, uint8_t* ckA, uint8_t* ckB);
    void handleScanResults();
    bool attemptConnection();
    bool discoverServices();
    void cleanup();
    
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                             uint8_t* pData, size_t length, bool isNotify);
    
    friend class RaceBoxCallbacks;
};
