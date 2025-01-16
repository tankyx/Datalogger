#pragma once

struct GpsPoint {
    double latitude;
    double longitude;
    bool isSet;
    float radius;
    
    GpsPoint() : latitude(0), longitude(0), isSet(false), radius(4.0f) {}
};
