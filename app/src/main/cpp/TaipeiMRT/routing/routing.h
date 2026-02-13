/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: routing.h                          *
 ****************************************** */

#pragma once

#include "../utils/utils.h"

#include <vector>
#include <unordered_set>

// ======== STRUCTS ======== //
typedef struct routedpath {
    Path path;
    PathTimes times; // Arrival/departure timeline
    int total_mins;
    int interchange_count;
} RoutedPath;

typedef struct routeconstraints {
    std::vector<Station> must_stations; // ensure these are in a specific order already, and also only suggest one shortest path lol
    std::vector<Station> avoid_stations;
    std::vector<Line> avoid_lines;
    std::vector<Line> must_lines;

    // Ranking pref
    bool minimize_time = true;
    bool minimize_interchanges = true;

    // Search limits
    int max_interchanges = 30;

    std::unordered_set<int> avoid_station_keys;
} RouteConstraints;

typedef struct candstate {
    Station stn;
    Path path;
    int interchange_count;
    int checkpoint_mask;
    int line_mask;
} CandState;

// ======== HELPERS ======== //
bool stationInList(const Station& s, const std::vector<Station>& v);
bool forbiddenStation(const Station& s, const RouteConstraints& c);

int countInterchanges(const Path& path);
bool usesLine(const Path& path, Line line);

std::string hashPath(const Path& p);
Path simplifyPath(const Path& p, const RouteConstraints& c); // E.g. R07 R06 R05 gets simplifed to R07 R05
Path mergePaths(const Path& a, const Path& b);
bool betterThan(const RoutedPath& a, const RoutedPath& b, const RouteConstraints& c); // Ranking/tie breaker

// ======== LAYER 1: UNTIMED ======== //
// Generate all paths without schedules, with avoid constraints applied
// must_stations can be handled by concatenation later and must_lines is global and filtered later

std::vector<Path> candidatePaths(const Station& src, const Station& dst, int max_paths, int max_interchanges, const RouteConstraints& constraints);

// ======== LAYER 2: REAL LIFE PATH ======== //
// Default: top 3 by time (tie break by interchanges), fixed candidate budget
std::vector<RoutedPath> routeDefault(const Station& src, const Station& dst, Time curr_time, int day_type, int k = 3);

// Least interchange: top 3 by interchanges (tie-break by time), fixed candidate budget
std::vector<RoutedPath> routeLeastInterchange(const Station& src, const Station& dst, Time curr_time, int day_type, int k = 3);

// Custom: supports must/avoid, uses adaptive widening if needed
std::vector<RoutedPath> routeCustom(const Station& src, const Station& dst, Time curr_time, int day_type, const RouteConstraints& constraints, int k = 3);

// ======== CORE ========= //
std::vector<RoutedPath> routeEngine(const Station& src, const Station& dst, Time curr_time, int day_type, const RouteConstraints& constraints, int k, int initial_budget, int hard_cap); // Takes all candidates, evaluates them wrt real time, filter must_lines, then rank