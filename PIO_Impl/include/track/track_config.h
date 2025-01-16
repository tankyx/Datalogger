#pragma once
#include <Arduino.h>
#include "track_types.h"

const size_t MAX_TRACKS = 10;
const size_t MAX_TRACK_NAME_LENGTH = 32;
const uint32_t CURRENT_CONFIG_VERSION = 1;

struct TrackConfig {
    char name[MAX_TRACK_NAME_LENGTH];
    GpsPoint startFinish;
    GpsPoint sector2Start;
    GpsPoint sector3Start;
    uint32_t configVersion;
    bool isValid;

    TrackConfig() :
        startFinish(),
        sector2Start(),
        sector3Start(),
        configVersion(CURRENT_CONFIG_VERSION),
        isValid(false)
    {
        name[0] = '\0';
    }
};
