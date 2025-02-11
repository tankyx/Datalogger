#pragma once

#include <Diablo_Serial_4DLib.h>
#include "delta_bar.h"
#include "speedometer.h"
#include "sector_display.h"
#include "lap_timer.h"
#include "status_bar.h"
#include "delta_calculator.h"
#include "track_data.h"

class RacingPanel {
private:
    Diablo_Serial_4DLib* display;
    DeltaBar deltaBar;
    Speedometer speedometer;
    SectorDisplay sectorDisplay;
    LapTimer lapTimer;
    StatusBar statusBar;
    delta_calculator deltaCalculator;

    bool isLapActive;
    uint32_t currentLapStartTime;
    bool stintActive;
    uint32_t stintStartTime;
    
    // Track specific variables
    uint32_t currentSector1Time;
    uint32_t currentSector2Time;
    uint32_t currentSector3Time;
    uint32_t sector1CrossTime;
    uint32_t sector2CrossTime;
    bool inSector1;
    bool inSector2;
    bool inSector3;
    bool inPitLane;

    void updateStintTimer() {
        uint32_t elapsed = millis() - stintStartTime;
        uint32_t minutes = elapsed / 60000;
        uint32_t seconds = (elapsed / 1000) % 60;
        
        char timeStr[10];
        snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu", minutes, seconds);
        statusBar.updateStintTimer(timeStr);
    }

    void checkPosition(double lat, double lon, int32_t speed) {
        // Create point for current position
        GpsPoint currentPos = {lat, lon, true, 4.0f};
        
        // Check for start/finish line crossing
        if (isNearPoint(currentPos, TrackData::startFinish)) {
            handleStartFinishCrossing(speed);
        }
        
        // Check sector points if lap is active
        if (isLapActive) {
            checkSectorCrossings(currentPos, speed);
            checkPitLane(currentPos);
        }
    }

    void handleStartFinishCrossing(int32_t speed) {
        if (isLapActive) {
            // Complete lap
            uint32_t lapTime = millis() - currentLapStartTime;
            
            // Calculate final sector time
            if (inSector3) {
                currentSector3Time = lapTime - sector2CrossTime;
                updateSectorTime(2, currentSector3Time);
            }
            
            lapTimer.completeLap();
            deltaCalculator.completeLap(lapTime);
            
            // Reset sector states
            inSector1 = false;
            inSector2 = false;
            inSector3 = false;
        }
        
        // Start new lap if speed is above minimum threshold (5 km/h)
        if (speed > 1.389) { // 5 km/h in m/s
            isLapActive = true;
            currentLapStartTime = millis();
            lapTimer.startLap();
            
            // Reset sector times
            currentSector1Time = 0;
            currentSector2Time = 0;
            currentSector3Time = 0;
            sector1CrossTime = 0;
            sector2CrossTime = 0;
        }
    }

    void checkSectorCrossings(const GpsPoint& currentPos, int32_t speed) {
        // Sector 1
        if (isNearPoint(currentPos, TrackData::sector1) && !inSector1) {
            inSector1 = true;
            sector1CrossTime = millis() - currentLapStartTime;
            currentSector1Time = sector1CrossTime;
            updateSectorTime(0, currentSector1Time);
        }
        
        // Sector 2
        if (isNearPoint(currentPos, TrackData::sector2) && !inSector2 && inSector1) {
            inSector2 = true;
            sector2CrossTime = millis() - currentLapStartTime;
            currentSector2Time = sector2CrossTime - sector1CrossTime;
            updateSectorTime(1, currentSector2Time);
            inSector3 = true; // Start sector 3 timing
        }
    }

    void checkPitLane(const GpsPoint& currentPos) {
        if (!TrackData::pitEntry.isSet || !TrackData::pitExit.isSet) return;

        if (isNearPoint(currentPos, TrackData::pitEntry) && !inPitLane) {
            inPitLane = true;
            // Optional: Handle pit entry events (e.g., pause stint timer)
        } else if (isNearPoint(currentPos, TrackData::pitExit) && inPitLane) {
            inPitLane = false;
            resetStint(); // Reset stint timer on pit exit
        }
    }

    void updateSectorTime(uint8_t sector, uint32_t time) {
        uint32_t& bestTime = (sector == 0) ? TrackData::sector1BestTime :
                            (sector == 1) ? TrackData::sector2BestTime :
                                          TrackData::sector3BestTime;
        
        if (time > 0 && (bestTime == UINT32_MAX || time < bestTime)) {
            bestTime = time;
        }
        
        sectorDisplay.updateSector(sector, time, bestTime);
        updateTheoreticalBest();
    }

    void updateTheoreticalBest() {
        uint32_t theoretical = 0;
        bool valid = true;

        if (TrackData::sector1BestTime != UINT32_MAX) {
            theoretical += TrackData::sector1BestTime;
        } else {
            valid = false;
        }

        if (TrackData::sector2BestTime != UINT32_MAX) {
            theoretical += TrackData::sector2BestTime;
        } else {
            valid = false;
        }

        if (TrackData::sector3BestTime != UINT32_MAX) {
            theoretical += TrackData::sector3BestTime;
        } else {
            valid = false;
        }

        if (valid) {
            TrackData::theoreticalBest = theoretical;
            sectorDisplay.updateTheoreticalBest(theoretical);
        }
    }

    static bool isNearPoint(const GpsPoint& pos, const GpsPoint& target) {
        if (!target.isSet) return false;
        
        // Haversine formula for accurate distance calculation
        const double R = 6371000; // Earth radius in meters
        double lat1 = pos.latitude * M_PI / 180;
        double lat2 = target.latitude * M_PI / 180;
        double dlat = (target.latitude - pos.latitude) * M_PI / 180;
        double dlon = (target.longitude - pos.longitude) * M_PI / 180;

        double a = sin(dlat/2) * sin(dlat/2) +
                  cos(lat1) * cos(lat2) *
                  sin(dlon/2) * sin(dlon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double distance = R * c;

        return distance <= target.radius;
    }

public:
    RacingPanel(Diablo_Serial_4DLib* disp) 
        : display(disp)
        , deltaBar(disp)
        , speedometer(disp)
        , sectorDisplay(disp)
        , lapTimer(disp)
        , statusBar(disp)
        , deltaCalculator()
        , isLapActive(false)
        , stintActive(false)
        , stintStartTime(0)
        , currentSector1Time(0)
        , currentSector2Time(0)
        , currentSector3Time(0)
        , sector1CrossTime(0)
        , sector2CrossTime(0)
        , inSector1(false)
        , inSector2(false)
        , inSector3(false)
        , inPitLane(false)
    {
        display->gfx_Cls();  // Clear screen on init
    }

    void draw() {
        deltaBar.draw();
        speedometer.draw();
        sectorDisplay.draw();
        lapTimer.draw();
        statusBar.draw();
    }

    void updateGPS(double lat, double lon, int32_t speed, bool valid, uint8_t satellites) {
        // Update GPS status
        statusBar.updateGPSStatus(valid, satellites);

        if (!valid) return;

        // Update speed display
        int32_t speedKph = (int32_t)(speed * 3.6f);  // Convert m/s to km/h
        speedometer.updateSpeed(speedKph);

        // Store point for delta calculation if lap is active
        if (isLapActive) {
            uint32_t currentLapTime = millis() - currentLapStartTime;
            GpsPoint point = {lat, lon, true, 4.0f};
            deltaCalculator.storePoint(currentLapTime, point, speed);
            
            // Calculate and update delta time
            int32_t delta = deltaCalculator.calculateDelta(currentLapTime, point);
            deltaBar.update(delta);
            
            // Update lap time
            lapTimer.updateCurrentLap();
        }

        // Check track position and handle sector/lap triggers
        checkPosition(lat, lon, speed);
    }

    void updateRBMStatus(bool connected) {
        statusBar.updateRBMStatus(connected);
    }

    void startStint() {
        stintActive = true;
        stintStartTime = millis();
        updateStintTimer();
    }

    void stopStint() {
        stintActive = false;
    }

    void resetStint() {
        if (stintActive) {
            stintStartTime = millis();
            updateStintTimer();
        }
    }

    void update() {
        if (stintActive) {
            updateStintTimer();
        }
        
        // Update any active animations or displays
        if (isLapActive) {
            lapTimer.updateCurrentLap();
        }
    }

    void resetLap() {
        isLapActive = false;
        currentLapStartTime = 0;
        inSector1 = false;
        inSector2 = false;
        inSector3 = false;
        currentSector1Time = 0;
        currentSector2Time = 0;
        currentSector3Time = 0;
        sector1CrossTime = 0;
        sector2CrossTime = 0;
        lapTimer.reset();
        deltaCalculator.reset();
    }

    void resetAll() {
        resetLap();
        stopStint();
        TrackData::resetTrack();
        draw(); // Redraw everything with reset values
    }

    void loadTrack(const char* name) {
        resetAll();
        TrackData::loadTrack(name);
    }

    void saveTrack(const char* name) {
        TrackData::saveTrack(name);
    }

    // Getters for current state
    bool isLapRunning() const { return isLapActive; }
    bool isStintRunning() const { return stintActive; }
    bool isPitLaneActive() const { return inPitLane; }
    uint32_t getCurrentLapTime() const { return isLapActive ? (millis() - currentLapStartTime) : 0; }
    uint32_t getStintTime() const { return stintActive ? (millis() - stintStartTime) : 0; }
    uint32_t getTheoreticalBest() const { return TrackData::theoreticalBest; }

    // Direct access to components if needed
    DeltaBar& getDeltaBar() { return deltaBar; }
    Speedometer& getSpeedometer() { return speedometer; }
    SectorDisplay& getSectorDisplay() { return sectorDisplay; }
    LapTimer& getLapTimer() { return lapTimer; }
    StatusBar& getStatusBar() { return statusBar; }
};
