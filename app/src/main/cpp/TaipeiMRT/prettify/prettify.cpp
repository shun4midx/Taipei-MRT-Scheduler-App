/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: prettify.cpp                       *
 ****************************************** */

#include "prettify.h"

#include <unordered_map>

// ======== DEFINITIONS ======= //
const std::unordered_map<Line, std::string> LINE_EMOJIS = {
    {R, "🟥"},
    {O, "🟧"},
    {Y, "🟨"},
    {G, "🟩"},
    {BL, "🟦"},
    {BR, "🟫"}
};

const std::unordered_map<Language, std::string> MINS = {
    {en, " min"},
    {zh, "分鐘"},
    {jp, "分"},
    {kr, "분"}
};

// ======== MISC OUTPUT STUFF ======== //
std::string colon(const Language& lang) {
    return (lang == en || lang == kr) ? ": " : "：";
}

// ======== STATION I/O TO USER ======== //
std::string prettifyStation(const Station& stn, const Language& lang) {
    return getName(stn, lang) + " " + LINE_EMOJIS.at(stn.line) + " " + stationToCode(stn);
}

// ======== PATH OUTPUTS ======== //
std::string stationTimeToStr(const StationTime& st, const Language& lang) {
    if (lang == en) {
        return "Arriving at " + timeToStr(st.first) + " / Departing at " + timeToStr(st.second);
    } else if (lang == zh) {
        return timeToStr(st.first) + "抵達 / " + timeToStr(st.second) + "離開";
    } else if (lang == jp) {
        return timeToStr(st.first) + "到着 / " + timeToStr(st.second) + "出発";  
    } else if (lang == kr) {
        return timeToStr(st.first) + "도착 / " + timeToStr(st.second) + "출발";
    }
}

std::string pathTimesToStr(const PathTimes& pt, const Language& lang) {
    std::string result = "";

    for (const auto& t : pt) {
        result += stationTimeToStr(t, lang) + "\n";
    }

    if (result.length() > 0) {
        result.pop_back();
    }

    return result;
}

std::string pathHeaderStr(const Path& p, const PathTimes& pt, const Language& lang, const TicketType& tt) {
    std::string output = std::to_string(timeToMins(pt.back().second) - timeToMins(pt.front().first)) + MINS.at(lang) + " $" + std::to_string(travelPrice(p.front(), p.back(), tt)) + " ";

    if (tt == ADULT) {
        if (lang == en) {
            output += "Adult ";
        } else if (lang == zh) {
            output += "成人 ";
        } else if (lang == jp) {
            output += "大人 ";
        } else if (lang == kr) {
            output += "성인 ";
        }
    } else if (tt == CHILD) {
        if (lang == en) {
            output += "Child ";
        } else if (lang == zh) {
            output += "兒童 ";
        } else if (lang == jp) {
            output += "子供 ";
        } else if (lang == kr) {
            output += "어린이 ";
        }
    } else if (tt == ELDERLY) {
        if (lang == en) {
            output += "Elderly ";
        } else if (lang == zh) {
            output += "敬老 ";
        } else if (lang == jp) {
            output += "高齢者 ";
        } else if (lang == kr) {
            output += "노인 ";
        }
    }

    output += LINE_EMOJIS.at(p[0].line);

    Line curr_line = p[0].line;

    for (auto& stn : p) {
        if (stn.line != curr_line) {
            output += LINE_EMOJIS.at(stn.line);
            curr_line = stn.line;
        }
    }

    output += "\n";

    return output;
}

std::string pathHeaderStr(const Path& p, const PathMins& pm, const Language& lang, const TicketType& tt) {
    std::string output = std::to_string(pm.back() - pm.front()) + MINS.at(lang) + + " $" + std::to_string(travelPrice(p.front(), p.back(), tt)) + " ";

    output += LINE_EMOJIS.at(p[0].line);

    Line curr_line = p[0].line;

    for (auto& stn : p) {
        if (stn.line != curr_line) {
            output += LINE_EMOJIS.at(stn.line);
            curr_line = stn.line;
        }
    }

    output += "\n";

    return output;
}

std::string namedPathTimesToStr(const Path& p, const PathTimes& pt, const Language& lang, const TicketType& tt) {
    if (p.size() != pt.size()) {
        throw std::invalid_argument("Path size != PathTimes size");
    }

    std::string result = pathHeaderStr(p, pt, lang, tt);

    bool brown_warning = false;

    for (int i = 0; i < p.size(); ++i) {
        result += prettifyStation(p[i], lang) + colon(lang) + stationTimeToStr(pt[i], lang) + "\n";

        if (p[i].line == BR) {
            brown_warning = true;
        }
    }

    if (brown_warning) {
        if (lang == en) {
            result += "⚠️ The train arrival times for the brown line stations are the WORST CASE SCENARIO only and do not reflect current conditions.\n";
        } else if (lang == zh) {
            result += "⚠️ 以上顯示文湖線的列車到達時間，都是以最壞狀況計算，且並非反映現實路線狀況。\n";
        } else if (lang == jp) {
            result += "⚠️ 上記の文湖線（茶色の線）の列車の到着時間は最悪の状況下で計算されており、実際の路線状況を反映するものではありません。\n";
        } else if (lang == kr) {
            result += "⚠️ 위에 표시된 원후선(갈색선) 열차 도착 시간은 최악의 상황을 가정하여 계산된 것이며 실제 운행 상황을 반영하지 않습니다.\n";
        }
    }

    if (result.length() > 0) {
        result.pop_back();
    }

    return result;
}

std::string pathToStr(const Path& p) {
    std::string result = "";

    for (const Station& stn : p) {
        result += stationToCode(stn) + " ";
    }

    if (result.length() > 0) {
        result.pop_back();
    }

    return result;
}

std::string pathMinsToStr(const PathMins& pm) {
    std::string output = "";

    for (const int& m : pm) {
        output += std::to_string(m) + " ";
    }

    if (output.length() > 0) {
        output.pop_back();
    }

    return output;
}

std::string namedPathMinsToStr(const Path& p, const PathMins& pm, const Language& lang, const TicketType& tt) {
    if (p.size() != pm.size()) {
        throw std::invalid_argument("Path size != PathMins size");
    }

    std::string result = pathHeaderStr(p, pm, lang, tt);

    for (int i = 0; i < p.size(); ++i) {
        result += prettifyStation(p[i], lang) + colon(lang) + std::to_string(pm[i]) + MINS.at(lang) + "\n";
    }

    if (result.length() > 0) {
        result.pop_back();
    }

    return result;
}

// ======== PATH OUTPUTS TO USER ======== //
std::string pathDetailsToUser(const Path& p, Time begin_time, int day_type, const Language& lang, const TicketType& tt) {
    PathTimes pt = pathETA(p, begin_time, day_type);
    return namedPathTimesToStr(p, pt, lang, tt);
}

std::string pathDetailsToUser(const Path& p, const Language& lang, const TicketType& tt) { // ETA not based on time
    PathMins pm = perfectPathETA(p);
    return namedPathMinsToStr(p, pm, lang, tt);
}

// ======== INTERCHANGE I/O TO USER ======== //
std::string interchangeLinesToUser(const Line& a, const Line& b, const Language& lang) {
    std::vector<std::pair<StationNode, int>> int_stns = getLineTransferStations(a, b);

    std::string output = "";

    for (const auto& stn : int_stns) {
        output += getName(stn.first.station_codes[0], lang);

        for (const Station& s : stn.first.station_codes) {
            if (s.line == a || s.line == b) {
                output += " " + LINE_EMOJIS.at(s.line) + " " + stationToCode(s);
            }
        }

        output += colon(lang) + std::to_string(stn.second) + MINS.at(lang) + "\n";
    }

    if (output.length() > 0) {
        output.pop_back();
    }

    return output;
}

std::string allStnCodesToUser(const Station& stn, const Language& lang) {
    std::string output = "";

    std::vector<Station> equiv_stns = getEquivalentStations(stn);

    output += getName(stn, lang) + colon(lang);

    for (const Station& s : equiv_stns) {
        output += LINE_EMOJIS.at(s.line) + " " + stationToCode(s) + " ";
    }
    
    if (output.length() > 0) {
        output.pop_back();
    }

    return output;
}