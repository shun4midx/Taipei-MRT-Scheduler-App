/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: cost.h                             *
 ****************************************** */

#pragma once

#include "utils.h"

#include <unordered_map>
#include <vector>

// ======== DEFINITIONS ======== //
enum TicketType {ADULT, CHILD, ELDERLY};

extern const std::vector<Station> STATION_ORDER;

extern const std::unordered_map<int, int> ADULT_TO_CHILD_PRICE;
extern const std::unordered_map<int, int> ADULT_TO_ELDERLY_PRICE;

extern const std::vector<std::vector<int>> PRICE_TABLE; // adult price

// ======== FUNCTIONS ======== //
int stationOrderIdx(const Station& s);
int travelPrice(const Station& s1, const Station& s2, const TicketType& tt = ADULT);