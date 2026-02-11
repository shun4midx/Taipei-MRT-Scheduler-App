/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: routing.cpp                        *
 ****************************************** */

#include "routing.h"
#include <queue>
#include <unordered_set>
#include <iostream>

// ======== HELPERS ======== //
bool stationInList(const Station& s, const std::vector<Station>& v) {
    for (const Station& stn : v) {
        if (sameStation(stn, s)) {
            return true;
        }
    }

    return false;
}

bool forbiddenStation(const Station& s, const RouteConstraints& c) {
    std::vector<Station> all_avoid_stations;

    for (auto& stn : c.avoid_stations) {
        std::vector<Station> alt_names = getEquivalentStations(stn);

        all_avoid_stations.insert(all_avoid_stations.end(), alt_names.begin(), alt_names.end());
    }

    if (stationInList(s, all_avoid_stations)) {
        return true;
    }

    for (Line l : c.avoid_lines) {
        if (s.line == l) {
            return true;
        }
    }

    return false;
}

int countInterchanges(const Path& path) {
    int count = 0;
    bool counted_orange_branch = false;

    for (int i = 0; i < path.size() - 1; ++i) {
        if (path[i].line != path[i + 1].line) {
            ++count;
            counted_orange_branch = false; // reset when leaving O
        } else if (path[i].line == O && i > 0 && path[i - 1].line == O && !counted_orange_branch) {
            if (path[i].stn_num >= 1 && path[i].stn_num <= 12 && ((path[i - 1].stn_num >= 50 && path[i + 1].stn_num < 50) || (path[i - 1].stn_num < 50 && path[i + 1].stn_num >= 50))) {
                ++count;
                counted_orange_branch = true;
            }
        }
    }

    return count;
}

bool usesLine(const Path& path, Line line) {
    for (const Station& s : path) {
        if (s.line == line) {
            return true;
        }
    }

    return false;
}

std::string hashPath(const Path& p) {
    std::string hash;

    for (const Station& s : p) {
        hash += stationToCode(s) + "-";
    }

    return hash;
}

bool isCheckpoint(const Station& s, const RouteConstraints& c) {
    for (const Station& ms : c.must_stations) {
        if (sameStation(s, ms)) {
            return true;
        }
    }
    return false;
}

Path simplifyPath(const Path& p, const RouteConstraints& c) {
    if (p.size() <= 2) {
        return p;
    }

    Path out;
    out.push_back(p[0]); // Always add start
    
    for (int i = 1; i < p.size() - 1; ++i) {
        bool must_include = false;
        
        // Must include checkpoints
        if (isCheckpoint(p[i], c)) {
            must_include = true;
        }
        
        // Must include if line changes
        if (p[i - 1].line != p[i].line || p[i].line != p[i + 1].line) {
            must_include = true;
        }
        
        // Must include for Orange line branch switches at O12
        if (p[i].line == O && sameStation(p[i], Station{O, 12})) {
            bool from_luzhou = (p[i - 1].stn_num >= 50);
            bool to_luzhou = (p[i + 1].stn_num >= 50);
            bool from_huilong = (p[i - 1].stn_num >= 13 && p[i - 1].stn_num < 50);
            bool to_huilong = (p[i + 1].stn_num >= 13 && p[i + 1].stn_num < 50);
            
            // Only show O12 if switching between Luzhou and Huilong branches
            if ((from_luzhou && to_huilong) || (from_huilong && to_luzhou)) {
                must_include = true;
            }
        }
        
        if (must_include) {
            out.push_back(p[i]);
        }
    }
    
    out.push_back(p.back()); // Always add end
    return out;
}

Path mergePaths(const Path& a, const Path& b) {
    if (a.size() == 0 && b.size() == 0) {
        throw std::invalid_argument("Path length is 0");
    } else if (a.size() == 0) {
        return b;
    } else if (b.size() == 0) {
        return a;
    }

    Path p = a;

    if (!sameStation(a.back(), b.front())) {
        p.insert(p.end(), b.begin(), b.end());   
    } else {
        p.insert(p.end(), b.begin() + 1, b.end());   
    }

    return p;
}

int bonus(const RoutedPath& rp, const RouteConstraints& c) {
    int b = 0;
    for (Line l : c.must_lines) {
        if (usesLine(rp.path, l)) {
            ++b;
        }
    }
    return b;
}


bool betterThan(const RoutedPath& a, const RoutedPath& b, const RouteConstraints& c) { // Ranking/tie breaker
    // If time
    if (c.minimize_time) {
        if (a.total_mins != b.total_mins) {
            return a.total_mins < b.total_mins;
        }

        if (a.interchange_count != b.interchange_count) {
            return a.interchange_count < b.interchange_count;
        }

        // Fallback
        return a.path.size() < b.path.size();
    }

    // Only interchange
    if (!c.minimize_time && c.minimize_interchanges) {
        if (a.interchange_count != b.interchange_count) {
            return a.interchange_count < b.interchange_count;
        }

        if (a.total_mins != b.total_mins) {
            return a.total_mins < b.total_mins;
        }

        // Fallback
        return a.path.size() < b.path.size();
    }

    // Bonus lines
    int scoreA = bonus(a, c);
    int scoreB = bonus(b, c);
    if (scoreA != scoreB) {
        return scoreA > scoreB;
    }

    // Neither
    return a.path.size() < b.path.size();
}

// ======== LAYER 1: UNTIMED ======== //
// Generate all paths without schedules, with avoid constraints applied
// must_stations can be handled by concatenation later and must_lines is global and filtered later

std::vector<Path> candidatePaths(const Station& src, const Station& dst, int max_paths, int max_interchanges, const RouteConstraints& constraints) {
    if (!validStation(src) || !validStation(dst)) {
        throw std::invalid_argument("candidatePaths: invalid src/dst station");
    }

    if (max_paths <= 0 || max_interchanges < 0) {
        return {};
    }

    // BFS-like exploration, prunes forbidden nodes & cycles
    std::queue<CandState> q;
    std::vector<Path> results;

    if (forbiddenStation(src, constraints) || forbiddenStation(dst, constraints)) {
        return {};
    }

    q.push({src, {src}, 0});

    while (!q.empty() && results.size() < max_paths) {
        CandState curr = q.front();
        q.pop();

        if (curr.interchange_count > max_interchanges) {
            continue;
        }

        if (sameStation(curr.stn, dst)) {
            results.push_back(curr.path);

            if (results.size() >= max_paths) {
                break;
            }

            continue;
        }

        // Same line neighbors: +/- station number
        for (int delta: {-1, +1}) {
            Station next{curr.stn.line, curr.stn.stn_num + delta};

            if (curr.stn.line == O && curr.stn.stn_num == 50 && delta == -1) {
                next.stn_num = 12; // O edge case
            }

            if (!validStation(next) || forbiddenStation(next, constraints)) {
                continue;
            }

            // Avoid cycles
            if (stationInList(next, curr.path)) {
                continue;
            }

            Path np = curr.path;
            np.push_back(next);
            q.push({next, np, curr.interchange_count});
        }

        // Edge case for O line
        if (sameStation(curr.stn, Station{O, 12})) {
            Station next{O, 50};
            
            if (!validStation(next) || forbiddenStation(next, constraints)) {
                continue;
            }
    
            // Avoid cycles
            if (stationInList(next, curr.path)) {
                continue;
            }
    
            Path np = curr.path;
            np.push_back(next);
            q.push({next, np, curr.interchange_count});
        }
    
        // Interchange neighbors
        try {
            auto transfers = getTransfers(curr.stn);

            for (const auto& [to, _mins] : transfers) {
                if (!validStation(to) || forbiddenStation(to, constraints)) {
                    continue;
                }

                // Avoid cycles
                if (stationInList(to, curr.path)) {
                    continue;
                }

                Path np = curr.path;
                np.push_back(to);
                q.push({to, np, curr.interchange_count + 1});
            }
        } catch (...) {
            // No transfers or invalid transfer table -> ignore
        }
    }

    return results;
}

// ======== LAYER 2: REAL LIFE PATH ======== //
// Default: top 3 by time (tie break by interchanges), fixed candidate budget
std::vector<RoutedPath> routeDefault(const Station& src, const Station& dst, Time curr_time, int day_type, int k) {
    RouteConstraints c;
    c.minimize_time = true;
    c.minimize_interchanges = true;
    c.max_interchanges = 4;

    return routeEngine(src, dst, curr_time, day_type, c, k, 6, 6);;
}

// Least interchange: top 3 by interchanges (tie-break by time), fixed candidate budget
std::vector<RoutedPath> routeLeastInterchange(const Station& src, const Station& dst, Time curr_time, int day_type, int k) {
    RouteConstraints c;
    c.minimize_time = false;
    c.minimize_interchanges = true;
    c.max_interchanges = 4;

    return routeEngine(src, dst, curr_time, day_type, c, k, 6, 6);
}

// Custom: supports must/avoid, uses adaptive widening if needed
std::vector<RoutedPath> routeCustom(const Station& src, const Station& dst, Time curr_time, int day_type, const RouteConstraints& constraints, int k) {
    if (sameStation(src, dst)) {
        return {{
            .path = {src},
            .times = {{curr_time, curr_time}},
            .total_mins = 0,
            .interchange_count = 0
        }};
    }    

    // If must_pass exists, we do segmented routing:
    // src -> via1 -> via2 -> ... -> dst
    // Each segment uses the same constraints (avoid rules apply), and time advances along the segments.
    if (!constraints.must_stations.empty()) {
        Station cur = src;
        Time t = curr_time;

        std::vector<RoutedPath> partials;
        std::vector<Station> checkpoints = constraints.must_stations;

        // initial empty partial
        partials.push_back(RoutedPath{
            .path = {src},
            .times = {{curr_time, curr_time}},
            .total_mins = 0,
            .interchange_count = 0
        });

        PathTimes full_times; // optional: we'll re-evaluate full_path at the end for a clean timeline

        checkpoints.push_back(dst);

        while (checkpoints.size() != 0) {
            Station target = checkpoints.front();

            std::vector<RoutedPath> next_partials;
        
            for (const RoutedPath& base : partials) {
                if (sameStation(base.path.back(), target)) {
                    next_partials.push_back(base);
                    continue;
                }
        
                RouteConstraints segc = constraints;
                segc.must_lines.clear(); // global constraint
                segc.must_stations = checkpoints;
        
                auto segs = routeEngine(base.path.back(), target, base.times.back().first, day_type, segc, k, 6, 100);
        
                for (const RoutedPath& seg : segs) {
                    RoutedPath combined;
                    combined.path = mergePaths(base.path, simplifyPath(seg.path, segc));
                    combined.times = seg.times; // OK: final ETA recomputed later
                    combined.total_mins = timeToMins(seg.times.back().first) - timeToMins(curr_time);
                    combined.interchange_count = countInterchanges(combined.path);
        
                    next_partials.push_back(combined);
                }
            }
        
            if (next_partials.empty()) {
                return {};
            }
        
            // Beam prune
            std::sort(next_partials.begin(), next_partials.end(),
                [&](const RoutedPath& a, const RoutedPath& b) {
                    return betterThan(a, b, constraints);
                }
            );

            // if (next_partials.size() > k * 10) {
            //     next_partials.resize(k);
            // }

            partials = std::move(next_partials);

            checkpoints.erase(checkpoints.begin()); // Remove one at a time so we don't waste time on unnecessarily stopping at checkpoints if we stopped before
        }        

        partials.erase(
            std::remove_if(partials.begin(), partials.end(),
                [&](const RoutedPath& rp) {
                    for (Line l : constraints.must_lines)
                        if (!usesLine(rp.path, l)) return true;
                    return false;
                }),
            partials.end()
        );    

        // Return results
        std::vector<RoutedPath> results;

        for (auto& cand : partials) {
            try {
                PathTimes pt = pathETA(cand.path, curr_time, day_type);

                RoutedPath rp;
                rp.path = cand.path;
                rp.times = pt;
                rp.total_mins = timeToMins(pt.back().first) - timeToMins(curr_time);
                rp.interchange_count = countInterchanges(cand.path);

                results.push_back(rp);
            } catch (...) {
                // this candidate is infeasible in real time -> discard
            }
        }

        if (results.empty()) {
            return {};
        }
        
        std::sort(results.begin(), results.end(),
            [&](const RoutedPath& a, const RoutedPath& b) {
                return betterThan(a, b, constraints);
            }
        );
        
        if (results.size() > k) {
            results.resize(k);
        }
        
        return results;
    }
    
    // No must_pass: run normally with adaptive widening
    return routeEngine(src, dst, curr_time, day_type, constraints, k, 6, 100);
} 

// ======== CORE ========= //
std::vector<RoutedPath> routeEngine(const Station& src, const Station& dst, Time curr_time, int day_type, const RouteConstraints& constraints, int k, int initial_budget, int hard_cap) { // Takes all candidates, evaluates them wrt real time, filter must_lines, then rank
    if (!validStation(src) || !validStation(dst)) {
        throw std::invalid_argument("routeEngine: invalid src/dst station");
    }
    if (day_type <= 0 || day_type > 7) {
        throw std::invalid_argument("routeEngine: invalid day_type");
    }
    if (curr_time.hr < 0 || curr_time.min < 0) {
        throw std::invalid_argument("routeEngine: invalid curr_time");
    }

    std::vector<RoutedPath> routed;
    std::unordered_set<std::string> seen_paths; // avoid re-evaluating duplicates across budgets

    // If src/dst themselves forbidden, no solution.
    if (forbiddenStation(src, constraints) || forbiddenStation(dst, constraints)) {
        return {};
    }

    int budget = std::max(1, initial_budget);
    int cap = std::max(budget, hard_cap);

    for (; budget <= cap; budget *= 2) {
        auto candidates = candidatePaths(src, dst, budget, constraints.max_interchanges, constraints);

        for (Path& p : candidates) {
            std::string key = hashPath(p);
            if (seen_paths.count(key)) {
                continue;
            }
            seen_paths.insert(key);

            try {
                // p = simplifyPath(p, constraints);

                PathTimes times = pathETA(p, curr_time, day_type);

                // total_mins from query time to final arrival time (times.back().first)
                int total_mins = timeToMins(times.back().first) - timeToMins(curr_time);

                RoutedPath rp;
                rp.path = p;
                rp.times = times;
                rp.total_mins = total_mins;
                rp.interchange_count = countInterchanges(p);

                routed.push_back(rp);
            } catch (...) {
                // invalid at this time / schedule / etc -> skip
            }
        }

        // Enforce must_lines (global path property) *after* evaluation
        if (!constraints.must_lines.empty()) {
            routed.erase(
                std::remove_if(routed.begin(), routed.end(),
                    [&](const RoutedPath& rp) {
                        for (Line l : constraints.must_lines) {
                            if (!usesLine(rp.path, l)) return true;
                        }
                        return false;
                    }
                ),
                routed.end()
            );
        }

        // Rank current pool
        std::sort(routed.begin(), routed.end(),
            [&](const RoutedPath& a, const RoutedPath& b) {
                return betterThan(a, b, constraints);
            }
        );

        // if (routed.size() >= k) {
        //     break;
        // }
    }

    for (auto& rp : routed) {
        rp.path = simplifyPath(rp.path, constraints);
        rp.times = pathETA(rp.path, curr_time, day_type);
        rp.total_mins = timeToMins(rp.times.back().first) - timeToMins(curr_time);
    }

    // Rank final pool
    std::sort(routed.begin(), routed.end(),
        [&](const RoutedPath& a, const RoutedPath& b) {
            return betterThan(a, b, constraints);
        }
    );

    if (routed.size() > k) {
        routed.resize(k);
    }

    return routed;
}