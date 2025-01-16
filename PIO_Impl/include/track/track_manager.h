#pragma once

#include <Preferences.h>
#include <vector>
#include <string>
#include "track_config.h"

class TrackManager {
public:
    TrackManager();
    ~TrackManager();
    
    bool init();
    bool saveTrack(const char* name, const TrackConfig& config);
    bool loadTrackByName(const char* name, TrackConfig* config);
    bool deleteTrack(const char* name);
    bool resetStorage();
    
    std::vector<std::string> getTrackNames();
    size_t getTrackCount() { return tracks.size(); }
    
    bool getCurrentTrack(TrackConfig* config);
    bool setCurrentTrack(const char* name);
    
private:
    Preferences preferences;
    static const char* STORAGE_NAMESPACE;
    std::vector<TrackConfig> tracks;
    size_t currentTrackIndex;

    bool saveTrackByIndex(size_t index, const TrackConfig& config);
    bool loadTrackByIndex(size_t index, TrackConfig* config);
};
