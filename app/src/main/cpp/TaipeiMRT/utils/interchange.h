/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: interchange.h                      *
 ****************************************** */

#pragma once

#include "basic.h"
#include <vector>
#include <unordered_map>
#include <utility>

// ======== STRUCTS ======== //
typedef struct stationnode {
    std::vector<Station> station_codes;
} StationNode;

// ======== DATA ======== //
extern const StationNode INVALID_STATION_NODE;

// All physical stations in the system
extern const std::unordered_map<Line, std::vector<StationNode>> STATION_NODES; // Basically make it so that given e.g. R10, we can access [R][10]

// All transfer times in the system
extern const std::unordered_map<Line, std::vector<std::vector<std::pair<Station, int>>>> TRANSFERS; // E.g. R10 to BL12, we can search TRANSFERS[R][10] and find the pair that has Station BL12 and find the int for how many mins the transfer takes

// ======== FUNCTIONS ======== //
const StationNode* getStationNode(const Station& stn); // Get physical station node (nullptr if none)

bool isInterchange(const Station& stn); // Is stn part of an interchange

std::vector<std::pair<Station, int>> getTransfers(const Station& stn); // All transfers from stn
bool canTransfer(const Station& from, const Station& to);
int getTransferTime(const Station& from, const Station& to); // Transfer time between two stations (-1 if none)

std::vector<Station> getEquivalentStations(const Station& stn); // All equivalent station codes at same physical station

std::string strStnVector(const std::vector<Station>& stn_vec);

std::string strTransfersVector(const std::vector<std::pair<Station, int>>& trans_vec);

std::vector<std::pair<StationNode, int>> getLineTransferStations(Line a, Line b); // Returns stations where you can transfer between the lines