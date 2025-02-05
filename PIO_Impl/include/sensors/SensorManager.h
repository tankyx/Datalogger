#pragma once

#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Adafruit_LSM6DSOX.h>
#include <Wire.h>

struct GNSSData {
    double latitude;
    double longitude;
    double altitude;
    double speed;      // Ground speed in m/s
    uint8_t satellites;
    uint8_t fixType;
    bool isValid;
    uint32_t timestamp;
};

struct IMUData {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    uint32_t timestamp;
};

class SensorManager {
public:
    SensorManager() : gnss(), imu() {}

    bool begin() {
        Wire.begin();
        
        // Initialize GNSS
        if (!gnss.begin(Wire)) {
            Serial.println("Failed to initialize GNSS!");
            return false;
        }

        // Configure GNSS settings
        gnss.setI2COutput(COM_TYPE_UBX);  // Set the I2C port to output UBX only
        gnss.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save the communications port settings
        
        // Configure GNSS update rate to 25Hz
        if (!gnss.setNavigationFrequency(25)) {
            Serial.println("Warning: Failed to set GNSS rate to 25Hz!");
            return false;
        }
        
        // Verify the navigation rate was set correctly
        uint8_t rate = gnss.getNavigationFrequency();
        if (rate != 25) {
            Serial.printf("Warning: GNSS rate is %dHz instead of 25Hz\n", rate);
            return false;
        }
        
        // Initialize IMU
        if (!imu.begin_I2C()) {
            Serial.println("Failed to initialize IMU!");
            return false;
        }

        // Configure IMU settings
        // Set accelerometer range (±2, ±4, ±8, ±16 g)
        imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
        
        // Set gyro range (±125, ±250, ±500, ±1000, ±2000 deg/s)
        imu.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
        
        // Set data rate (Hz)
        imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
        imu.setGyroDataRate(LSM6DS_RATE_104_HZ);

        return true;
    }

    GNSSData readGNSS() {
        GNSSData data;
        data.timestamp = millis();
        
        // Read position data
        data.latitude = gnss.getLatitude() / 10000000.0; // Convert to degrees
        data.longitude = gnss.getLongitude() / 10000000.0;
        data.altitude = gnss.getAltitude() / 1000.0; // Convert to meters
        
        // Read speed
        data.speed = gnss.getGroundSpeed() / 1000.0; // Convert to m/s
        
        // Read fix information
        data.satellites = gnss.getSIV();
        data.fixType = gnss.getFixType();
        
        // Check if we have a valid fix
        data.isValid = (data.fixType == 3 && data.satellites >= 4);
        
        return data;
    }

    IMUData readIMU() {
        IMUData data;
        data.timestamp = millis();
        
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;
        
        imu.getEvent(&accel, &gyro, &temp);
        
        // Store accelerometer data (in m/s²)
        data.accelX = accel.acceleration.x;
        data.accelY = accel.acceleration.y;
        data.accelZ = accel.acceleration.z;
        
        // Store gyroscope data (in rad/s)
        data.gyroX = gyro.gyro.x;
        data.gyroY = gyro.gyro.y;
        data.gyroZ = gyro.gyro.z;
        
        return data;
    }

    // Optional: Check if new GNSS data is available
    bool isGNSSDataAvailable() {
        return gnss.getPVT();
    }

private:
    SFE_UBLOX_GNSS gnss;
    Adafruit_LSM6DSOX imu;
};