/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: basic.h                            *
 ****************************************** */

#pragma once
#include <unordered_map>
#include <string>
#include <vector>

// ======== DEFINITIONS ======== //
enum Language {zh, en, jp, kr};
enum Line {R, O, G, BL, BR, Y};

extern const std::unordered_map<Line, std::string> LINE_TO_STR;
extern const std::unordered_map<std::string, Line> LINES;

// ======== STRUCTS ========= //
typedef struct station {
    Line line;
    int stn_num;
} Station;

typedef struct Time {
    int hr;
    int min;
} Time;

extern const Station INVALID_STATION;

// ======== STORING INFORMATION ======== //
extern const std::string INVALID;

extern const Time INVALID_TIME;

extern std::vector<std::unordered_map<Language, std::string>> R_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> G_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> BL_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> BR_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> Y_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> O01_NAMES;
extern std::vector<std::unordered_map<Language, std::string>> O50_NAMES;

// ======== RETRIEVAL FUNCTIONS ======== //
bool sameTime(const Time& time1, const Time& time2);

int timeToMins(const Time& time); // Time to mins after 00:00
Time minsToTime(int mins);
std::string timeToStr(const Time& time);

Time minsAfter(const Time& time, int mins);

bool sameStation(const Station& stn1, const Station& stn2);

bool validStation(const Station& station);
bool validStation(int line_int, int stn_num);

Station makeStation(int line_int, int stn_num); // Returns station given relevant detail

std::string getName(const Station& station, int lang_int = zh); // Default to Chinese
std::string getName(int line_int, int stn_num, int lang_int = zh);

Station codeToStation(std::string stn_code);
std::string stationToCode(const Station& station);