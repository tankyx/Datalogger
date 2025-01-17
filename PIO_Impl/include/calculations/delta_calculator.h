#pragma once


#pragma once
#include <stdint.h>
#include <vector>
#include <cmath>
#include "track_types.h"

struct LapPoint {
    GpsPoint position;     // Position with isSet and radius from track_types.h
    uint32_t timestamp;    // Time since lap start in milliseconds
    int32_t speed;        // Speed in mm/s
};

struct LapData {
    std::vector<LapPoint> points;
    uint32_t lapTime;      // Total lap time in milliseconds
    bool isValid;
    
    LapData() : lapTime(0), isValid(false) {
        points.reserve(3000);  // Pre-allocate for 2-minute lap at 25Hz
    }
    
    void clear() {
        points.clear();
        lapTime = 0;
        isValid = false;
    }
};

class delta_calculator {
private:
    LapData bestLap;
    LapData currentLap;
    static const uint32_t SAMPLE_INTERVAL_MS = 40;  // 25Hz data rate
    static const uint32_t MAX_POINTS = 3000;        // 2-minute lap at 25Hz
    static constexpr double EARTH_RADIUS = 6371000.0;     // Earth radius in meters
    
    // Calculate Haversine distance between two points in meters
    double calculateDistance(const GpsPoint& p1, const GpsPoint& p2) {
        double lat1 = p1.latitude * M_PI / 180.0;
        double lat2 = p2.latitude * M_PI / 180.0;
        double deltaLat = (p2.latitude - p1.latitude) * M_PI / 180.0;
        double deltaLon = (p2.longitude - p1.longitude) * M_PI / 180.0;

        double a = sin(deltaLat/2) * sin(deltaLat/2) +
                  cos(lat1) * cos(lat2) *
                  sin(deltaLon/2) * sin(deltaLon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));

        return EARTH_RADIUS * c;
    }

    // Find the closest point in bestLap to the given position
    int findClosestPoint(const GpsPoint& position, size_t searchStart, size_t searchEnd) {
        int closestIdx = -1;
        double minDistance = position.radius; // Use radius from GpsPoint for threshold

        // Local search around expected position
        for (size_t i = searchStart; i < searchEnd && i < bestLap.points.size(); i++) {
            double distance = calculateDistance(position, bestLap.points[i].position);
            if (distance < minDistance) {
                minDistance = distance;
                closestIdx = i;
            }
        }

        return closestIdx;
    }

public:
    delta_calculator() {}

    void reset() {
        bestLap.clear();
        currentLap.clear();
    }

    void storePoint(uint32_t currentLapTime, const GpsPoint& point, int32_t speed) {
        if (currentLap.points.size() >= MAX_POINTS || !point.isSet) {
            return;
        }

        // Only store points at fixed intervals
        if (currentLap.points.empty() || 
            (currentLapTime - currentLap.points.back().timestamp) >= SAMPLE_INTERVAL_MS) {
            
            LapPoint lapPoint;
            lapPoint.position = point;
            lapPoint.timestamp = currentLapTime;
            lapPoint.speed = speed;
            currentLap.points.push_back(lapPoint);
        }
    }

    void completeLap(uint32_t finalLapTime) {
        currentLap.lapTime = finalLapTime;
        currentLap.isValid = true;

        // Update best lap if this lap is faster or no valid best lap exists
        if (!bestLap.isValid || finalLapTime < bestLap.lapTime) {
            bestLap = currentLap;
        }

        currentLap.clear();
    }

    /**
     * Calculate delta time using spatial matching and interpolation.
     * 
     * The algorithm:
     * 1. Find the closest point in the best lap to current position
     * 2. Use local search window to improve matching efficiency
     * 3. Interpolate between closest points for smooth delta
     * 
     * @param currentLapTime Current time in milliseconds
     * @param position Current GPS position
     * @return Delta in ms (positive = slower, negative = faster)
     */
    int32_t calculateDelta(uint32_t currentLapTime, const GpsPoint& position) {
        if (!bestLap.isValid || bestLap.points.empty() || currentLap.points.empty() || !position.isSet) {
            return 0;
        }

        // Calculate expected search window based on current time
        double lapProgressRatio = (double)currentLapTime / bestLap.lapTime;
        size_t expectedIndex = (size_t)(lapProgressRatio * bestLap.points.size());
        
        // Define search window (20% of lap points)
        size_t windowSize = bestLap.points.size() / 5;
        size_t searchStart = (expectedIndex > windowSize) ? expectedIndex - windowSize : 0;
        size_t searchEnd = std::min(expectedIndex + windowSize, bestLap.points.size());

        // Find closest point using GpsPoint's radius as threshold
        int closestIdx = findClosestPoint(position, searchStart, searchEnd);
        
        if (closestIdx < 0) {
            return 0;  // No matching point found within threshold
        }

        // Find second closest point for interpolation
        int nextIdx = closestIdx + 1;
        if (nextIdx >= (int)bestLap.points.size()) {
            return currentLapTime - bestLap.lapTime;  // At end of lap
        }

        // Calculate interpolation ratio based on relative distances
        double d1 = calculateDistance(position, bestLap.points[closestIdx].position);
        double d2 = calculateDistance(position, bestLap.points[nextIdx].position);
        double totalDist = d1 + d2;
        
        if (totalDist < 0.1) {  // Avoid division by very small numbers
            return currentLapTime - bestLap.points[closestIdx].timestamp;
        }

        // Interpolate between points
        double ratio = d1 / totalDist;
        uint32_t expectedTime = bestLap.points[closestIdx].timestamp +
                              (uint32_t)((bestLap.points[nextIdx].timestamp - 
                                        bestLap.points[closestIdx].timestamp) * ratio);

        // Return delta (positive = slower, negative = faster)
        return currentLapTime - expectedTime;
    }

    uint32_t getBestLapTime() const {
        return bestLap.isValid ? bestLap.lapTime : 0;
    }

    bool hasBestLap() const {
        return bestLap.isValid;
    }

    // Get current best lap point count
    size_t getBestLapPointCount() const {
        return bestLap.points.size();
    }
};