/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: interchange.cpp                    *
 ****************************************** */

#include "interchange.h"
#include "basic.h"

#include <stdexcept>

// ======== DATA ======== //
const StationNode INVALID_STATION_NODE = StationNode{{}};

// All physical stations in the system
const std::unordered_map<Line, std::vector<StationNode>> STATION_NODES = {
    // Red Line
    {R, {
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        StationNode{{Station{R, 2}}}, // R02: Xiangshan
        StationNode{{Station{R, 3}}}, // R03: Tpe 101
        StationNode{{Station{R, 4}}}, // R04: Xinyi Anhe
        StationNode{{Station{R, 5}, Station{BR, 9}}}, // R05: Daan
        StationNode{{Station{R, 6}}}, // R06: Daan Park
        StationNode{{Station{R, 7}, Station{O, 6}}}, // R07: Dongmen
        StationNode{{Station{R, 8}, Station{G, 10}}}, // R08: CKSMH
        StationNode{{Station{R, 9}}}, // R09: NTUH
        StationNode{{Station{R, 10}, Station{BL, 12}}}, // R10: TPEMS
        StationNode{{Station{R, 11}, Station{G, 14}}}, // R11: Zhongshan
        StationNode{{Station{R, 12}}}, // R12: Shuanglian
        StationNode{{Station{R, 13}, Station{O, 11}}}, // R13: Minquan W Road
        StationNode{{Station{R, 14}}}, // R14
        StationNode{{Station{R, 15}}}, // R15
        StationNode{{Station{R, 16}}}, // R16
        StationNode{{Station{R, 17}}}, // R17
        StationNode{{Station{R, 18}}}, // R18
        StationNode{{Station{R, 19}}}, // R19
        StationNode{{Station{R, 20}}}, // R20
        StationNode{{Station{R, 21}}}, // R21
        StationNode{{Station{R, 22}}}, // R22
        StationNode{{Station{R, 23}}}, // R23
        StationNode{{Station{R, 24}}}, // R24
        StationNode{{Station{R, 25}}}, // R25
        StationNode{{Station{R, 26}}}, // R26
        StationNode{{Station{R, 27}}}, // R27
        StationNode{{Station{R, 28}}} // R28
    }},
    
    // Green Line
    {G, {
        INVALID_STATION_NODE,
        StationNode{{Station{G, 1}}}, // G01: Xindian
        StationNode{{Station{G, 2}}}, // G02: Xindian Dist Office
        StationNode{{Station{G, 3}}}, // G03: Qizhang
        StationNode{{Station{G, 4}, Station{Y, 7}}}, // G04: Dapinglin
        StationNode{{Station{G, 5}}}, // G05: Jingmei
        StationNode{{Station{G, 6}}}, // G06: Wanlong
        StationNode{{Station{G, 7}}}, // G07: Gongguan
        StationNode{{Station{G, 8}}}, // G08: Taipwr bldg
        StationNode{{Station{G, 9}, Station{O, 5}}}, // G09: Guting
        StationNode{{Station{G, 10}, Station{R, 8}}}, // G10: CKSMH
        StationNode{{Station{G, 11}}}, // G11: Xiaonanmen
        StationNode{{Station{G, 12}, Station{BL, 11}}}, // G12: Ximen
        StationNode{{Station{G, 13}}}, // G13: Beimen
        StationNode{{Station{G, 14}, Station{R, 11}}}, // G14: Zhongshan
        StationNode{{Station{G, 15}, Station{O, 8}}}, // G15: Songjiang Nanjing
        StationNode{{Station{G, 16}, Station{BR, 11}}}, // G16: Nanjing Fuxing
        StationNode{{Station{G, 17}}}, // G17
        StationNode{{Station{G, 18}}}, // G18
        StationNode{{Station{G, 19}}} // G19
    }},

    // Blue Line
    {BL, {
        INVALID_STATION_NODE,
        StationNode{{Station{BL, 1}}}, // BL01
        StationNode{{Station{BL, 2}}}, // BL02
        StationNode{{Station{BL, 3}}}, // BL03
        StationNode{{Station{BL, 4}}}, // BL04
        StationNode{{Station{BL, 5}}}, // BL05
        StationNode{{Station{BL, 6}}}, // BL06
        StationNode{{Station{BL, 7}, Station{Y, 16}}}, // BL07: Banqiao
        StationNode{{Station{BL, 8}, Station{Y, 17}}}, // BL08: Xinpu
        StationNode{{Station{BL, 9}}}, // BL09
        StationNode{{Station{BL, 10}}}, // BL10
        StationNode{{Station{BL, 11}, Station{G, 12}}}, // BL11: Ximen
        StationNode{{Station{BL, 12}, Station{R, 10}}}, // BL12: TPEMS
        StationNode{{Station{BL, 13}}}, // BL13
        StationNode{{Station{BL, 14}, Station{O, 7}}}, // BL14: Zhongxiao Xinsheng
        StationNode{{Station{BL, 15}, Station{BR, 10}}}, // BL15: Zhongxiao Fuxing
        StationNode{{Station{BL, 16}}}, // BL16
        StationNode{{Station{BL, 17}}}, // BL17
        StationNode{{Station{BL, 18}}}, // BL18
        StationNode{{Station{BL, 19}}}, // BL19
        StationNode{{Station{BL, 20}}}, // BL20
        StationNode{{Station{BL, 21}}}, // BL21
        StationNode{{Station{BL, 22}}}, // BL22
        StationNode{{Station{BL, 23}, Station{BR, 24}}} // BL23: Nangang exhib center
    }},

    // Brown Line
    {BR, {
        INVALID_STATION_NODE,
        StationNode{{Station{BR, 1}}}, // BR01
        StationNode{{Station{BR, 2}}}, // BR02
        StationNode{{Station{BR, 3}}}, // BR03
        StationNode{{Station{BR, 4}}}, // BR04
        StationNode{{Station{BR, 5}}}, // BR05
        StationNode{{Station{BR, 6}}}, // BR06
        StationNode{{Station{BR, 7}}}, // BR07
        StationNode{{Station{BR, 8}}}, // BR08
        StationNode{{Station{BR, 9}, Station{R, 5}}}, // BR09: Daan
        StationNode{{Station{BR, 10}, Station{BL, 15}}}, // BR10: Zhongxiao Fuxing
        StationNode{{Station{BR, 11}, Station{G, 16}}}, // BR11: Nanjing Fuxing
        StationNode{{Station{BR, 12}}}, // BR12
        StationNode{{Station{BR, 13}}}, // BR13
        StationNode{{Station{BR, 14}}}, // BR14
        StationNode{{Station{BR, 15}}}, // BR15
        StationNode{{Station{BR, 16}}}, // BR16
        StationNode{{Station{BR, 17}}}, // BR17
        StationNode{{Station{BR, 18}}}, // BR18
        StationNode{{Station{BR, 19}}}, // BR19
        StationNode{{Station{BR, 20}}}, // BR20
        StationNode{{Station{BR, 21}}}, // BR21
        StationNode{{Station{BR, 22}}}, // BR22
        StationNode{{Station{BR, 23}}}, // BR23
        StationNode{{Station{BR, 24}, Station{BL, 23}}} // BR24: Nangang exhib center
    }},

    // Yellow Line
    {Y, {
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        INVALID_STATION_NODE,
        StationNode{{Station{Y, 7}, Station{G, 4}}}, // Y07: Dapinglin
        StationNode{{Station{Y, 8}}}, // Y08
        StationNode{{Station{Y, 9}}}, // Y09
        StationNode{{Station{Y, 10}}}, // Y10
        StationNode{{Station{Y, 11}, Station{O, 2}}}, // Y11: Jingan
        StationNode{{Station{Y, 12}}}, // Y12
        StationNode{{Station{Y, 13}}}, // Y13
        StationNode{{Station{Y, 14}}}, // Y14
        StationNode{{Station{Y, 15}}}, // Y15
        StationNode{{Station{Y, 16}, Station{BL, 7}}}, // Y16: Banqiao
        StationNode{{Station{Y, 17}, Station{BL, 8}}}, // Y17: Xinpu Minsheng
        StationNode{{Station{Y, 18}, Station{O, 17}}}, // Y18: Touqianzhuang
        StationNode{{Station{Y, 19}}}, // Y19
        StationNode{{Station{Y, 20}}} // Y20
    }},

    // Orange Line
    {O, {
        INVALID_STATION_NODE,
        StationNode{{Station{O, 1}}}, // O01
        StationNode{{Station{O, 2}, Station{Y, 11}}}, // O02: Jingan
        StationNode{{Station{O, 3}}}, // O03
        StationNode{{Station{O, 4}}}, // O04
        StationNode{{Station{O, 5}, Station{G, 9}}}, // O05: Guting
        StationNode{{Station{O, 6}, Station{R, 7}}}, // O06: Dongmen
        StationNode{{Station{O, 7}, Station{BL, 14}}}, // O07: Zhongxiao Xinsheng
        StationNode{{Station{O, 8}, Station{G, 15}}}, // O08: Songjiang Nanjing
        StationNode{{Station{O, 9}}}, // O09
        StationNode{{Station{O, 10}}}, // O10
        StationNode{{Station{O, 11}, Station{R, 13}}}, // O11: Minquan W Road
        StationNode{{Station{O, 12}}}, // O12
        StationNode{{Station{O, 13}}}, // O13
        StationNode{{Station{O, 14}}}, // O14
        StationNode{{Station{O, 15}}}, // O15
        StationNode{{Station{O, 16}}}, // O16
        StationNode{{Station{O, 17}, Station{Y, 18}}}, // O17: Touqianzhuang
        StationNode{{Station{O, 18}}}, // O18
        StationNode{{Station{O, 19}}}, // O19
        StationNode{{Station{O, 20}}}, // O20
        StationNode{{Station{O, 21}}}, // O21,
        INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, // 22-29 empty
        INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, // 30-39 empty
        INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, INVALID_STATION_NODE, // 40-49 empty
        StationNode{{Station{O, 50}}}, // O50
        StationNode{{Station{O, 51}}}, // O51
        StationNode{{Station{O, 52}}}, // O52
        StationNode{{Station{O, 53}}}, // O53
        StationNode{{Station{O, 54}}} // O54
    }}
};

// All transfer times in the system [may be edited later, I am basing this off of my memory since I'm not in Taipei right now. I'll fix it when I'm back home in Taipei if there are any parts that are inaccurate, since then I can actually test out each interchange, and I really don't mind that as an MRT nerd! Alternatively, if you're familiar with a station's interchange and think I got the time wrong, please tell me and I'll fix it.]
const std::unordered_map<Line, std::vector<std::vector<std::pair<Station, int>>>> TRANSFERS = {
    // Red line
    {R, {
        {},
        {},
        {}, // R02
        {}, // R03
        {}, // R04
        {{{Station{BR, 9}, 5}}}, // R05: Daan
        {}, // R06
        {{{Station{O, 6}, 1}}}, // R07: Dongmen
        {{{Station{G, 10}, 1}}}, // R08: CKSMH
        {}, // R09
        {{{Station{BL, 12}, 4}}}, // R10: TPEMS
        {{{Station{G, 14}, 3}}}, // R11: Zhongshan
        {}, // R12
        {{{Station{O, 11}, 3}}}, // R13: Minquan W Road
        {}, {}, {}, {}, {}, {}, // R14-19
        {}, {}, {}, {}, {}, {}, {}, {}, {} // R20-28
    }},

    // Green line
    {G, {
        {},
        {}, // G01
        {}, // G02
        {}, // G03
        {{{Station{Y, 7}, 3}}}, // G04: Dapinglin
        {}, // G05
        {}, // G06
        {}, // G07
        {}, // G08
        {{{Station{O, 5}, 1}}}, // G09: Guting
        {{{Station{R, 8}, 1}}}, // G10: CKSMH
        {}, // G11
        {{{Station{BL, 11}, 1}}}, // G12: Ximen
        {}, // G13
        {{{Station{R, 11}, 3}}}, // G14: Zhongshan
        {{{Station{O, 8}, 2}}}, // G15: Songjiang Nanjing
        {{{Station{BR, 11}, 5}}}, // G16: Nanjing Fuxing
        {}, // G17
        {}, // G18
        {} // G19
    }},

    // Blue line
    {BL, {
        {},
        {}, {}, {}, {}, {}, {}, // BL1-6
        {{{Station{Y, 16}, 11}}}, // BL07: Banqiao
        {{{Station{Y, 17}, 9}}}, // BL08: Xinpu
        {}, // BL09
        {}, // BL10
        {{{Station{G, 12}, 1}}}, // BL11: Ximen
        {{{Station{R, 10}, 4}}}, // BL12: TPEMS
        {}, // BL13
        {{{Station{O, 7}, 1}}}, // BL14: Zhongxiao Xinsheng
        {{{Station{BR, 10}, 5}}}, // BL15: Zhongxiao Fuxing
        {}, {}, {}, {}, {}, {}, {}, // BL16-22
        {{{Station{BR, 24}, 6}}} // BL23: Nangang exhib center
    }},

    // Brown line
    {BR, {
        {},
        {}, {}, {}, {}, {}, {}, {}, {}, // BR1-8
        {{{Station{R, 5}, 5}}}, // BR09: Daan
        {{{Station{BL, 15}, 5}}}, // BR10: Zhongxiao Fuxing
        {{{Station{G, 16}, 5}}}, // BR11: Nanjing Fuxing
        {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, // BR12-23
        {{{Station{BL, 23}, 6}}} // BR24: Nangang exhib center
    }},

    // Yellow line
    {Y, {
        {},
        {}, {}, {}, {}, {}, {}, // No Y1-6
        {{{Station{G, 4}, 3}}}, // Y07: Dapinglin
        {}, // Y08
        {}, // Y09
        {}, // Y10
        {{{Station{O, 2}, 6}}}, // Y11: Jingan
        {}, // Y12
        {}, // Y13
        {}, // Y14
        {}, // Y15
        {{{Station{BL, 7}, 11}}}, // Y16: Banqiao
        {{{Station{BL, 8}, 9}}}, // Y17: Xinpu Minsheng
        {{{Station{O, 17}, 6}}}, // Y18: Touqianzhuang
        {}, // Y19
        {} // Y20
    }},

    // Orange line
    {O, {
        {},
        {}, // O01
        {{{Station{Y, 11}, 6}}}, // O02: Jingan
        {}, // O03
        {}, // O04
        {{{Station{G, 9}, 1}}}, // O05: Guting
        {{{Station{R, 7}, 1}}}, // O06: Dongmen
        {{{Station{BL, 14}, 1}}}, // O07: Zhongxiao Xinsheng
        {{{Station{G, 15}, 2}}}, // O08: Songjiang Nanjing
        {}, // O09
        {}, // O10
        {{{Station{R, 13}, 3}}}, // O11: Minquan W Road
        {}, {}, {}, {}, {}, // O12-16
        {{{Station{Y, 18}, 6}}}, // O17: Touqianzhuang
        {}, {}, {}, {}, // O18-21
        {}, {}, {}, {}, {}, {}, {}, {}, // empty 22-29
        {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, // empty 30-39
        {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, // empty 40-49
        {}, {}, {}, {}, {} // empty 50-54
    }}
};

// ======== FUNCTIONS ======== //
const StationNode* getStationNode(const Station& stn) {
    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station stn");
    }

    return &STATION_NODES.at(stn.line)[stn.stn_num];
}

bool isInterchange(const Station& stn) {
    return (getStationNode(stn)->station_codes.size() > 1);
}

std::vector<std::pair<Station, int>> getTransfers(const Station& stn) {
    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station stn");
    }

    return TRANSFERS.at(stn.line)[stn.stn_num];
}

bool canTransfer(const Station& from, const Station& to) {
    try {
        getTransferTime(from, to);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

int getTransferTime(const Station& from, const Station& to) {
    if (sameStation(from, to)) {
        return 0;
    }

    const auto& from_transfers = getTransfers(from);

    for (const auto& s_pair : from_transfers) {
        if (sameStation(s_pair.first, to)) {
            return s_pair.second;
        }
    }

    throw std::invalid_argument("Station to is not transferrable from Station from");
}

std::vector<Station> getEquivalentStations(const Station& stn) {
    if (!validStation(stn)) {
        throw std::invalid_argument("Invalid station stn");
    }

    return STATION_NODES.at(stn.line)[stn.stn_num].station_codes;
}

std::string strStnVector(const std::vector<Station>& stn_vec) {
    std::string ret = "";

    for (const Station& stn : stn_vec) {
        ret += stationToCode(stn) + ", ";
    }

    if (ret.length() > 0) {
        ret.pop_back();
        ret.pop_back();
    }

    return ret;
}

std::string strTransfersVector(const std::vector<std::pair<Station, int>>& trans_vec) {
    std::string ret = "";

    for (auto& trans : trans_vec) {
        ret += stationToCode(trans.first) + ": " + std::to_string(trans.second) + "\n";
    }

    if (ret.length() > 0) {
        ret.pop_back();
    }

    return ret;
}

std::vector<std::pair<StationNode, int>> getLineTransferStations(Line a, Line b) {
    // Search for line a ones in line b

    std::vector<std::pair<StationNode, int>> stn_nodes;

    std::vector<std::vector<std::pair<Station, int>>> line_transfers = TRANSFERS.at(b);

    for (const auto& stn_transfers : line_transfers) {
        for (const auto& stn_pair : stn_transfers) {
            if (stn_pair.first.line == a) {
                stn_nodes.push_back({*getStationNode(stn_pair.first), stn_pair.second});
            }
        }
    }

    return stn_nodes;
}