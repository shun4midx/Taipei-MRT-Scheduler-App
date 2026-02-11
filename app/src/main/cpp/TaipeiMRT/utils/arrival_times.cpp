/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: arrival_times.cpp                  *
 ****************************************** */

#include "arrival_times.h"
#include "basic.h"
#include "utils.h"

#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>

// ======== DEFINITIONS ======== //
// Make the following 1-idxed

// {increasing dir/nangang exhib center, decreasing dir/taipei zoo}

const std::vector<std::vector<Time>> BR_FIRST_TRAINS = {
    {INVALID_TIME, INVALID_TIME},
    {Time{6, 0}, INVALID_TIME}, // BR01
    {Time{6, 1}, Time{6, 4}}, // BR02
    {Time{6, 2}, Time{6, 3}}, // BR03
    {Time{6, 4}, Time{6, 1}}, // BR04
    {Time{6, 0}, Time{6, 0}}, // BR05
    {Time{6, 1}, Time{6, 3}}, // BR06
    {Time{6, 3}, Time{6, 1}}, // BR07
    {Time{6, 0}, Time{6, 0}}, // BR08
    {Time{6, 1}, Time{6, 5}}, // BR09
    {Time{6, 3}, Time{6, 3}}, // BR10
    {Time{6, 5}, Time{6, 1}}, // BR11
    {Time{6, 0}, Time{6, 0}}, // BR12
    {Time{6, 2}, Time{6, 2}}, // BR13
    {Time{6, 0}, Time{6, 0}}, // BR14
    {Time{6, 1}, Time{6, 3}}, // BR15
    {Time{6, 3}, Time{6, 1}}, // BR16
    {Time{6, 0}, Time{6, 0}}, // BR17
    {Time{6, 1}, Time{6, 5}}, // BR18
    {Time{6, 2}, Time{6, 3}}, // BR19
    {Time{6, 4}, Time{6, 1}}, // BR20
    {Time{6, 0}, Time{6, 0}}, // BR21
    {Time{6, 1}, Time{6, 3}}, // BR22
    {Time{6, 3}, Time{6, 1}}, // BR23
    {INVALID_TIME, Time{6, 0}} // BR24
};

const std::vector<std::vector<Time>> BR_LAST_TRAINS = {
    {INVALID_TIME, INVALID_TIME},
    {Time{24, 0}, INVALID_TIME}, // BR01
    {Time{24, 1}, Time{24, 53}}, // BR02
    {Time{24, 2}, Time{24, 52}}, // BR03
    {Time{24, 5}, Time{24, 49}}, // BR04
    {Time{24, 7}, Time{24, 47}}, // BR05
    {Time{24, 10}, Time{24, 44}}, // BR06
    {Time{24, 12}, Time{24, 42}}, // BR07
    {Time{24, 15}, Time{24, 39}}, // BR08
    {Time{24, 33}, Time{24, 37}}, // BR09
    {Time{24, 35}, Time{24, 35}}, // BR10
    {Time{24, 38}, Time{24, 32}}, // BR11
    {Time{24, 40}, Time{24, 30}}, // BR12
    {Time{24, 43}, Time{24, 27}}, // BR13
    {Time{24, 46}, Time{24, 23}}, // BR14
    {Time{24, 49}, Time{24, 20}}, // BR15
    {Time{24, 52}, Time{24, 18}}, // BR16
    {Time{24, 54}, Time{24, 15}}, // BR17
    {Time{24, 56}, Time{24, 13}}, // BR18
    {Time{24, 58}, Time{24, 11}}, // BR19
    {Time{25, 0}, Time{24, 9}}, // BR20
    {Time{25, 3}, Time{24, 5}}, // BR21
    {Time{25, 5}, Time{24, 3}}, // BR22
    {Time{25, 7}, Time{24, 1}}, // BR23
    {INVALID_TIME, Time{24, 0}} // BR24
};

// ======== LOADING ======== //
std::string dayGroup(const Line& line, int day_type) {
    if (day_type <= 0 || day_type > 7) {
        throw std::invalid_argument("Invalid day_type: " + std::to_string(day_type));
    } else {
        if (day_type <= 5) {
            return "12345";
        } else if (line == R) { // Only R has 6 and 7 separately
            return std::to_string(day_type);
        } else {
            return "67";
        }
    }
}

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;

    for (char c : s) {
        if (c == ',') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

// Rmb to have exception for BR line
std::vector<Train> loadStationSchedule(const Station& stn, int day_type) {
    // Detect wrong inputs
    if (day_type <= 0 || day_type > 7) {
        throw std::invalid_argument("Invalid day type: " + std::to_string(day_type));
    }

    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station");
    }

    if (stn.line == BR) {
        throw std::invalid_argument("BR line stations don't have station schedules provided, we only have a rough estimate of how many minutes are between trains.");
    }

    // Find correct file
    // Reads arrival_times/generated/{LINE}/{STATION}_{DAYS}.csv
    std::string file_name = DATA_DIR + "/arrival_times/generated/" + LINE_TO_STR.at(stn.line) + "/" + stationToCode(stn) + "_" + dayGroup(stn.line, day_type) + ".csv";

    std::cerr << "DATA_DIR = " << DATA_DIR << std::endl;

    std::ifstream fin(file_name);
    std::string read_line;

    // Parse info
    std::vector<Train> train_schedule;

    while (std::getline(fin, read_line)) {
        std::vector<std::string> train_info = split(read_line); // line,destination,direction,time (where line is like e.g. R-1)

        // typedef struct train {Station arrive; int time; int direction; Station train_dest;} Train;
        // Swallow invalid stations
        try {
            Train new_train;
            new_train.arrive = stn;
            new_train.time = std::stoi(train_info[3]);
            new_train.direction = std::stoi(train_info[2]);
            new_train.train_dest = codeToStation(train_info[1]);

            train_schedule.push_back(new_train);
        } catch (std::exception& e) {
            // Don't do anything
        }
    }

    return train_schedule;
}

void printTrainSchedule(const std::vector<Train>& train_schedule) {
    for (Train t : train_schedule) {
        std::cout << stationToCode(t.arrive) << " " << t.time << " " << t.direction << " " << stationToCode(t.train_dest) << std::endl;
    }
}

// ======== QUERY ======== //
bool oneTrainReachDest(const Station& stn, const Station& dest, Train train) {
    // Detect exceptions
    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station stn");
    }

    if (!validStation(dest)) {
        throw std::invalid_argument("Invalid station dest");
    }

    if (sameStation(stn, dest)) {
        throw std::invalid_argument("stn and dest are the same station");
    }

    // Code
    if (stn.line != dest.line) { // Need interchange
        return false;
    }

    if (stn.line != O) { // Orange line is diff
        if (stn.stn_num < dest.stn_num && stn.stn_num < train.train_dest.stn_num) { // Increasing
            return dest.stn_num <= train.train_dest.stn_num;
        } else if (stn.stn_num > dest.stn_num && stn.stn_num > train.train_dest.stn_num) { // Decreasing
            return dest.stn_num >= train.train_dest.stn_num;
        } else { // Wrong dir
            return false;
        }
    } else {
        // Rule out wrong direction stuff
        if ((stn.stn_num > dest.stn_num && stn.stn_num <= train.train_dest.stn_num) || (stn.stn_num < dest.stn_num && stn.stn_num >= train.train_dest.stn_num)) {
            return false;
        }

        if (stn.stn_num > dest.stn_num) { // Decreasing branch (definitely fine since the train is a valid train)
            return dest.stn_num >= train.train_dest.stn_num;
        } else { // Increasing branch
            // O12 is the last station that overlaps both branches
            if (dest.stn_num <= 12) {
                return dest.stn_num <= train.train_dest.stn_num;
            } else if (dest.stn_num <= train.train_dest.stn_num) { // Check the correct branch given that its number is plausibly reachable
                if (dest.stn_num < 50) { // dest in O01 branch
                    return train.train_dest.stn_num < 50;
                } else { // dest in O50 branch
                    return train.train_dest.stn_num >= 50;
                }
            } else { // Impossible
                return false;
            }
        }
    }
}

Time nextTrainTime(const Station& stn, int day_type, const Time& curr_time, const Station& dest) {
    return nextTrainTime(stn, day_type, timeToMins(curr_time), dest);
}

Time nextTrainTime(const Station& stn, int day_type, int now_mins, const Station& dest) {
    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station stn");
    }

    if (!validStation(dest)) {
        throw std::invalid_argument("Invalid station dest");
    }

    // Brown line: Can give worst case approximations based on timeframe
    if (stn.line == BR && dest.line == BR) {
        /*
        平常日（週一至週五）
        (1) 尖峰時段（07:00～09:00，17:00～19:30）：約2～4分鐘。
        (2) 離峰時段：約4～10分鐘。
        (3) 23:00以後：約12分鐘。

        例假日（週六、週日及國定假日）
        (1) 06:00～23:00：約4～10分鐘。
        (2) 23:00以後：約12分鐘。
        */

        Time first_br_train = firstTrainTime(stn, day_type, dest);
        Time last_br_train = lastTrainTime(stn, day_type, dest);

        if (timeToMins(first_br_train) > now_mins) {
            return first_br_train;
        } else if (timeToMins(last_br_train) < now_mins) {
            return INVALID_TIME;
        } else { // Normal: just use worst case approximations
            if (day_type <= 0 || day_type > 7) {
                throw std::invalid_argument("Invalid day_type: " + std::to_string(day_type));
            }

            if (now_mins >= timeToMins(Time{23, 0})) {
                return minsToTime(now_mins + 12);
            } else if (day_type <= 5 && ((now_mins >= timeToMins(Time{7, 0}) && now_mins <= timeToMins(Time{9, 0})) || (now_mins >= timeToMins(Time{17, 0}) && now_mins <= timeToMins(Time{19, 30})))) {
                return minsToTime(now_mins + 4);
            } else {
                return minsToTime(now_mins + 10);
            }
        }
    }

    // Otherwise

    std::vector<Train> train_schedule = loadStationSchedule(stn, day_type);

    // Find the closest entry that has time >= now_mins
    auto it = std::lower_bound(
        train_schedule.begin(),
        train_schedule.end(),
        now_mins,
        [](const Train& t, int value) {
            return t.time < value;
        }
    );
    
    if (it != train_schedule.end()) {
        // *it is the first Train with t.time >= target_time
        while (!oneTrainReachDest(stn, dest, *it)) {
            if (++it == train_schedule.end()) {
                return INVALID_TIME;
            }
        }

        // Otherwise it's a valid time
        return minsToTime(it->time);
    } else { // No entry with such a time
        return INVALID_TIME;
    }
}

Time firstTrainTime(const Station& stn, int day_type, const Station& dest) {
    if (stn.line != BR) { // Given timetable
        // All trains begin at 6am
        return nextTrainTime(stn, day_type, Time{6, 0}, dest);
    } else { // Deal with it separately
        if (!validStation(stn)) {
            throw std::invalid_argument("Invalid station stn");
        } else if (!validStation(dest)) {
            throw std::invalid_argument("Invalid station dest");
        } else if (day_type <= 0 || day_type > 7) {
            throw std::invalid_argument("Invalid day_type: " + std::to_string(day_type));
        } else {
            if (stn.stn_num == dest.stn_num) {
                throw std::invalid_argument("stn and dest are the same station");
            }

            return BR_FIRST_TRAINS[stn.stn_num][stn.stn_num < dest.stn_num ? 0 : 1];
        }
    }
}

Time lastTrainTime(const Station& stn, int day_type, const Station& dest) {
    if (stn.line != BR) { // Given timetable
        std::vector<Train> train_schedule = loadStationSchedule(stn, day_type);

        if (train_schedule.empty()) {
            return INVALID_TIME;
        }

        auto it = train_schedule.rbegin();

        // Traverse from the end
        while (!oneTrainReachDest(stn, dest, *it)) {
            --it;
        }

        return minsToTime(it->time);
    } else { // Deal with it separately
        if (!validStation(stn)) {
            throw std::invalid_argument("Invalid station stn");
        } else if (!validStation(dest)) {
            throw std::invalid_argument("Invalid station dest");
        } else if (day_type <= 0 || day_type > 7) {
            throw std::invalid_argument("Invalid day_type: " + std::to_string(day_type));
        } else {
            if (stn.stn_num == dest.stn_num) {
                throw std::invalid_argument("stn and dest are the same station");
            }

            return BR_LAST_TRAINS[stn.stn_num][stn.stn_num < dest.stn_num ? 0 : 1];
        }
    }
}