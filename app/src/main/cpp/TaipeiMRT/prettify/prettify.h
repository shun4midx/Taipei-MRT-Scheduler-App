/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ Header file               *
 * File: prettify.h                         *
 ****************************************** */

// Prettify structures when printing them out to be visible, or just like converting between natural language and station codes

#pragma once

#include "../utils/utils.h"

#include <unordered_map>

// ======== DEFINITIONS ======= //
extern const std::unordered_map<Line, std::string> LINE_EMOJIS;
extern const std::unordered_map<Language, std::string> MINS;

// ======== MISC OUTPUT STUFF ======== //
std::string colon(const Language& lang);

// ======== STATION I/O TO USER ======== //
std::string prettifyStation(const Station& stn, const Language& lang = zh);

// ======== PATH OUTPUTS ======== //
std::string stationTimeToStr(const StationTime& st, const Language& lang = zh);
std::string pathTimesToStr(const PathTimes& pt, const Language& lang = zh);
std::string pathHeaderStr(const Path& p, const PathTimes& pt, const Language& lang = zh, const TicketType& tt = ADULT);
std::string pathHeaderStr(const Path& p, const PathMins& pm, const Language& lang = zh, const TicketType& tt = ADULT);

std::string namedPathTimesToStr(const Path& p, const PathTimes& pt, const Language& lang = zh, const TicketType& tt = ADULT);
std::string pathToStr(const Path& p);
std::string pathMinsToStr(const PathMins& pm);
std::string namedPathMinsToStr(const Path& p, const PathMins& pm, const Language& lang = zh, const TicketType& tt = ADULT);

// ======== PATH I/O TO USER ======== //
std::string pathDetailsToUser(const Path& p, Time begin_time, int day_type, const Language& lang = zh, const TicketType& tt = ADULT);

std::string pathDetailsToUser(const Path& p, const Language& lang = zh, const TicketType& tt = ADULT); // ETA not based on time

// ======== INTERCHANGE I/O TO USER ======== //
std::string interchangeLinesToUser(const Line& a, const Line& b, const Language& lang = zh); // Get the stations that you can transfer between these lines with

std::string allStnCodesToUser(const Station& stn, const Language& lang = zh); // getEquivalentStations basically but outputted with the corresponding lines as the main point and also corresponding name (sometimes interchange stations are called diff names)