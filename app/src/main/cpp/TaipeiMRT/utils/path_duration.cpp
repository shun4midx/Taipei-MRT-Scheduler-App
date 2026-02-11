/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: path_duration.cpp                  *
 ****************************************** */

#include "path_duration.h"
#include "arrival_times.h"
#include "interchange.h"
#include <stdexcept>

// ======== DATA ======== //
const int INVALID_DURATION = -114514; // I know I'm supposed to take this seriously but I clearly am not

const std::unordered_map<Line, std::vector<int>> LINE_PREFIX_DURATION = {
    // Red line
    {R, {
        INVALID_DURATION, INVALID_DURATION,
        0, 2, 4, 6, 8, 19, 13, 15, // R02-09
        17, 18, 20, 21, 23, 26, 28, 30, 31, 33, // R10-19
        35, 37, 39, 41, 44, 45, 48, 51, 54 // R20-28
    }},

    // Green line
    {G, {
        INVALID_DURATION,
        0, 2, 4, 6, 8, 10, 12, 14, 16, // G01-09
        18, 20, 22, 24, 26, 28, 31, 32, 35, 37 // G10-19
    }},

    // Blue line
    {BL, {
        INVALID_DURATION,
        0, 3, 6, 8, 11, 13, 14, 17, 18, // BL01-09
        22, 24, 27, 29, 30, 33, 34, 36, 38, 40, // BL10-19
        41, 44, 46, 48 // BL20-23
    }},

    // Brown line
    {BR, {
        INVALID_DURATION,
        0, 2, 3, 5, 7, 10, 11, 14, 15, // BR01-09
        17, 19, 21, 23, 27, 29, 31, 33, 34, 36, // BR10-19
        37, 40, 42, 43, 45 // BR20-24
    }},

    // Yellow line
    {Y, {
        INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, // empty 00-06
        0, 3, 5, 7, 9, 12, 15, 16, 19, 21, 25, 28, 30, 33
    }},

    // Orange line
    {O, {
        INVALID_DURATION,
        0, 2, 4, 6, 10, 14, 17, 19, 21, 23, 25, 26, // Up til O12
        29, 31, 33, 36, 38, 40, 43, 45, 48, // Up til O21
        INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, // empty 22-29
        INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, // empty 30-39
        INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, INVALID_DURATION, // empty 40-49
        29, 32, 33, 35, 38 // O50 to O54
    }}
};

// ======== FUNCTIONS ======== //
int getLineDuration(const Station& stn1, const Station& stn2) {
    if (!validStation(stn1)) {
        throw std::invalid_argument("Invalid station stn1");
    }

    if (!validStation(stn2)) {
        throw std::invalid_argument("Invalid station stn2");
    }

    if (stn1.line != stn2.line) {
        throw std::invalid_argument("Stations stn1 and stn2 must be on the same line");
    }

    if (stn1.line != O || (stn1.stn_num < 50 && stn2.stn_num < 50)) { // Same branch
        return std::abs(LINE_PREFIX_DURATION.at(stn2.line)[stn2.stn_num] - LINE_PREFIX_DURATION.at(stn1.line)[stn1.stn_num]);
    } else { // Consider O line branching
        if (stn1.stn_num < stn2.stn_num) {
            if ((stn2.stn_num >= 50 && stn1.stn_num <= 12) || (stn1.stn_num >= 50 && stn2.stn_num >= 50)) { // Same branch
                return LINE_PREFIX_DURATION.at(O)[stn2.stn_num] - LINE_PREFIX_DURATION.at(O)[stn1.stn_num];
            } else { // Needs to account for wait time so we can't rly calculate
                throw std::invalid_argument("Although on the same line, it can't be done by being on the same branch, i.e. we need wait time between trains, so this is an invalid input.");
            }
        } else {
            return getLineDuration(stn2, stn1); // DRY
        }
    }
}

PathMins perfectPathETA(const Path& stn_path) {
    // Invalid input
    if (stn_path.size() < 2) {
        throw std::invalid_argument("stn_path must at least have 2 stations");
    }

    // Normal code
    if (!validStation(stn_path[0])) {
        throw std::invalid_argument("Invalid station stn_path[0]");
    }

    PathMins pm;

    pm.push_back(0);

    for (int i = 0; i < stn_path.size() - 1; ++i) {// consider i and i+1
        if (!validStation(stn_path[i + 1])) {
            throw std::invalid_argument("Invalid station stn_path[" + std::to_string(i + 1) + "]");
        }

        if (stn_path[i].line == stn_path[i + 1].line) { // Take a train
            try {
                pm.push_back(pm.back() + getLineDuration(stn_path[i], stn_path[i + 1]));
            } catch (const std::exception& e) {
                throw std::invalid_argument("No valid path from stn_path[" + std::to_string(i) + "] to stn_path[" + std::to_string(i + 1) + "]");
            }
        } else { // Check if it's interchange
            if (canTransfer(stn_path[i], stn_path[i + 1])) {
                pm.push_back(pm.back() + getTransferTime(stn_path[i], stn_path[i + 1]));
            } else {
                throw std::invalid_argument("No valid path from stn_path[" + std::to_string(i) + "] to stn_path[" + std::to_string(i + 1) + "]");
            }
        }
    }

    return pm;
}

int perfectPathDuration(const Path& stn_path) {
    return perfectPathETA(stn_path).back();
}

PathTimes pathETA(const Path& stn_path, Time curr_time, int day_type) {
    // Invalid input
    if (stn_path.size() < 2) {
        throw std::invalid_argument("stn_path must at least have 2 stations");
    }

    if (day_type <= 0 || day_type > 7) {
        throw std::invalid_argument("Invalid day_type: " + std::to_string(day_type));
    }

    // Remember to check invalid time
    if (curr_time.hr < 0 || curr_time.min < 0) {
        throw std::invalid_argument("Invalid time curr_time");
    }

    // Normal code
    if (!validStation(stn_path[0])) {
        throw std::invalid_argument("Invalid station stn_path[0]");
    }

    PathTimes arrival_times;

    // Push first time
    StationTime temp;
    temp.first = curr_time;

    // Iterate for all remaining times
    for (int i = 0; i < stn_path.size() - 1; ++i) {// consider i and i+1
        if (!validStation(stn_path[i + 1])) {
            throw std::invalid_argument("Invalid station stn_path[" + std::to_string(i + 1) + "]");
        }

        if (stn_path[i].line == stn_path[i + 1].line) { // Take a train
            try {
                // Calculate train arrival time and then calculate the time it'll take that train to reach i + 1
                // Fill "departure time"
                temp.second = nextTrainTime(stn_path[i], day_type, temp.first, stn_path[i + 1]);

                arrival_times.push_back(temp);

                // Next temp
                temp.first = minsAfter(temp.second, getLineDuration(stn_path[i], stn_path[i + 1]));
            } catch (const std::exception& e) {
                throw std::invalid_argument("No valid path from stn_path[" + std::to_string(i) + "] to stn_path[" + std::to_string(i + 1) + "]");
            }
        } else { // Check if it's interchange
            if (canTransfer(stn_path[i], stn_path[i + 1])) {
                // Fill "departure time"
                temp.second = temp.first;

                arrival_times.push_back(temp);
                
                // Next temp
                temp.first = minsAfter(temp.second, getTransferTime(stn_path[i], stn_path[i + 1]));
            } else {
                throw std::invalid_argument("No valid path from stn_path[" + std::to_string(i) + "] to stn_path[" + std::to_string(i + 1) + "]");
            }
        }
    }

    temp.second = temp.first;

    arrival_times.push_back(temp);

    return arrival_times;
}