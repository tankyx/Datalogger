#include "track/track_manager.h"

const char* TrackManager::STORAGE_NAMESPACE = "trackmgr";

TrackManager::TrackManager() : currentTrackIndex(0) {
    tracks.reserve(MAX_TRACKS);
}

TrackManager::~TrackManager() {
    preferences.end();
}

bool TrackManager::init() {
    if (!preferences.begin(STORAGE_NAMESPACE, true)) {
        Serial.println("Failed to open preferences");
        return false;
    }
    
    // Load track count
    size_t count = preferences.getUInt("track_count", 0);
    preferences.end();
    
    // Load all stored tracks
    for (size_t i = 0; i < count && i < MAX_TRACKS; i++) {
        TrackConfig track;
        if (loadTrackByIndex(i, &track)) {
            tracks.push_back(track);
        }
    }
    
    return true;
}

bool TrackManager::saveTrack(const char* name, const TrackConfig& config) {
    // Check if track with this name already exists
    for (size_t i = 0; i < tracks.size(); i++) {
        if (strcmp(tracks[i].name, name) == 0) {
            // Update existing track
            tracks[i] = config;
            return saveTrackByIndex(i, config);
        }
    }
    
    // Add new track
    if (tracks.size() >= MAX_TRACKS) {
        Serial.println("Maximum number of tracks reached");
        return false;
    }
    
    tracks.push_back(config);
    size_t index = tracks.size() - 1;
    
    if (!preferences.begin(STORAGE_NAMESPACE, false)) {
        Serial.println("Failed to open preferences");
        return false;
    }
    
    // Update track count
    preferences.putUInt("track_count", tracks.size());
    preferences.end();
    
    return saveTrackByIndex(index, config);
}

bool TrackManager::loadTrackByName(const char* name, TrackConfig* config) {
    for (const auto& track : tracks) {
        if (strcmp(track.name, name) == 0) {
            *config = track;
            return true;
        }
    }
    return false;
}

bool TrackManager::deleteTrack(const char* name) {
    for (auto it = tracks.begin(); it != tracks.end(); ++it) {
        if (strcmp(it->name, name) == 0) {
            tracks.erase(it);
            
            // Update storage
            if (!preferences.begin(STORAGE_NAMESPACE, false)) {
                return false;
            }
            
            preferences.putUInt("track_count", tracks.size());
            
            // Resave all tracks to remove gaps
            for (size_t i = 0; i < tracks.size(); i++) {
                saveTrackByIndex(i, tracks[i]);
            }
            
            preferences.end();
            return true;
        }
    }
    return false;
}

bool TrackManager::resetStorage() {
    if (!preferences.begin(STORAGE_NAMESPACE, false)) {
        return false;
    }
    
    bool success = preferences.clear();
    preferences.end();
    
    if (success) {
        tracks.clear();
        currentTrackIndex = 0;
    }
    
    return success;
}

std::vector<std::string> TrackManager::getTrackNames() {
    std::vector<std::string> names;
    names.reserve(tracks.size());
    for (const auto& track : tracks) {
        names.push_back(track.name);
    }
    return names;
}

bool TrackManager::getCurrentTrack(TrackConfig* config) {
    if (tracks.empty() || currentTrackIndex >= tracks.size()) {
        return false;
    }
    *config = tracks[currentTrackIndex];
    return true;
}

bool TrackManager::setCurrentTrack(const char* name) {
    for (size_t i = 0; i < tracks.size(); i++) {
        if (strcmp(tracks[i].name, name) == 0) {
            currentTrackIndex = i;
            return true;
        }
    }
    return false;
}

bool TrackManager::saveTrackByIndex(size_t index, const TrackConfig& config) {
    if (!preferences.begin(STORAGE_NAMESPACE, false)) {
        return false;
    }
    
    char key[16];
    snprintf(key, sizeof(key), "track_%u", (unsigned int)index);
    size_t written = preferences.putBytes(key, &config, sizeof(TrackConfig));
    preferences.end();
    
    return written == sizeof(TrackConfig);
}

bool TrackManager::loadTrackByIndex(size_t index, TrackConfig* config) {
    if (!preferences.begin(STORAGE_NAMESPACE, true)) {
        return false;
    }
    
    char key[16];
    snprintf(key, sizeof(key), "track_%u", (unsigned int)index);
    size_t read = preferences.getBytes(key, config, sizeof(TrackConfig));
    preferences.end();
    
    return read == sizeof(TrackConfig);
}
