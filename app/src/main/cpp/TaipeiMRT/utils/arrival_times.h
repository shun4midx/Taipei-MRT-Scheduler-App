/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: arrival_times.h                    *
 ****************************************** */

#pragma once

#include "basic.h"

#include <string>
#include <vector>

extern std::string DATA_DIR;

// Read from the arrival_times/generated folder
// National holidays count as 7

// ======== STRUCTS ======== //
typedef struct train {
    Station arrive; // station it will arrive
    int time; // minutes since midnight (can exceed 1440)
    int direction; // 0 (incr) / 1 (decr)
    Station train_dest; // final station
} Train;

// ======== DEFINITIONS ======== //
extern const std::vector<std::vector<Time>> BR_FIRST_TRAINS;
extern const std::vector<std::vector<Time>> BR_LAST_TRAINS;

// ======== LOADING ======== //
std::string dayGroup(const Line& line, int day_type);

// Reads arrival_times/generated/{LINE}/{STATION}_{DAYS}.csv (only R does 6 and 7 separately, others do 67)
std::vector<Train> loadStationSchedule(const Station& stn, int day_type); // day_type: 1-7

void printTrainSchedule(const std::vector<Train>& train_schedule);

// ======== QUERY ======== //
bool oneTrainReachDest(const Station& stn, const Station& dest, Train train);

Time nextTrainTime(const Station& stn, int day_type, const Time& curr_time, const Station& dest); // Returns next arrival time in minutes, or {-1, -1} if none
Time nextTrainTime(const Station& stn, int day_type, int now_mins, const Station& dest);

Time firstTrainTime(const Station& stn, int day_type, const Station& dest);
Time lastTrainTime(const Station& stn, int day_type, const Station& dest);