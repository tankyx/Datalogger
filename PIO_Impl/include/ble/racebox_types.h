#pragma once

#pragma pack(1)
struct RaceBoxDataMessage {
    uint32_t iTOW;           // GPS time of week in milliseconds
    uint16_t year;           // Year (UTC)
    uint8_t month;           // Month, 1-12 (UTC)
    uint8_t day;            // Day of month, 1-31 (UTC)
    uint8_t hour;           // Hour of day, 0-23 (UTC)
    uint8_t minute;         // Minute of hour, 0-59 (UTC)
    uint8_t second;         // Seconds of minute, 0-59 (UTC)
    uint8_t validityFlags;  // Validity flags for date & time
    uint32_t timeAccuracy;  // Time accuracy estimate in ns
    int32_t nanoseconds;    // Fraction of second in ns
    uint8_t fixStatus;      // GNSS fix Status
    uint8_t fixStatusFlags; // Fix status flags
    uint8_t dateTimeFlags;  // Additional date/time flags
    uint8_t numSV;         // Number of satellites used
    int32_t longitude;     // Longitude in 1e-7 degrees
    int32_t latitude;      // Latitude in 1e-7 degrees
    int32_t wgsAltitude;   // Height above WGS ellipsoid in mm
    int32_t mslAltitude;   // Height above mean sea level in mm
    uint32_t horizontalAcc; // Horizontal accuracy in mm
    uint32_t verticalAcc;  // Vertical accuracy in mm
    int32_t speed;         // Ground speed in mm/s
    int32_t heading;       // Heading of motion in 1e-5 degrees
    uint32_t speedAcc;     // Speed accuracy in mm/s
    uint32_t headingAcc;   // Heading accuracy in 1e-5 degrees
    uint16_t pDOP;         // Position DOP * 100
    uint8_t latLonFlags;   // Flags for lat/lon
    uint8_t batteryStatus; // Battery percentage or input voltage * 10
    int16_t gForceX;       // X axis G-Force in milli-g
    int16_t gForceY;       // Y axis G-Force in milli-g
    int16_t gForceZ;       // Z axis G-Force in milli-g
    int16_t rotationX;     // X axis rotation rate in centi-deg/s
    int16_t rotationY;     // Y axis rotation rate in centi-deg/s
    int16_t rotationZ;     // Z axis rotation rate in centi-deg/s
};
#pragma pack()
