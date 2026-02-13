#include <jni.h>
#include <string>
#include <chrono>
#include <ctime>

#include "TaipeiMRT/prettify/prettify.h"
#include "TaipeiMRT/utils/utils.h"
#include "TaipeiMRT/routing/routing.h"

std::string DATA_DIR;

const std::vector<Line> LINE_ORDER = {R, O, Y, G, BL, BR};

const std::unordered_map<Line, std::vector<Station>> ARRIVAL_DESTS = {
        {R, {Station{R, 2}, Station{R, 28}, Station{R, 5}, Station{R, 22}}},
        {O, {Station{O, 1}, Station{O, 21}, Station{O, 54}}},
        {Y, {Station{Y, 7}, Station{Y, 20}}},
        {G, {Station{G, 1}, Station{G, 19}, Station{G, 8}}},
        {BL, {Station{BL, 1}, Station{BL, 23}, Station{BL, 5}, Station{BL, 21}}},
        {BR, {Station{BR, 1}, Station{BR, 24}}}
};

extern "C"
JNIEXPORT void JNICALL
Java_com_shun4midx_mrt_MainActivity_setDataDir(
        JNIEnv* env, jobject, jstring path
) {
    const char* raw = env->GetStringUTFChars(path, nullptr);
    DATA_DIR = raw;
    env->ReleaseStringUTFChars(path, raw);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_shun4midx_mrt_MainActivity_getStationsDisplayList(
        JNIEnv* env,
        jobject,
        jstring line_code,
        jint langInt
) {
    const char* raw = env->GetStringUTFChars(line_code, nullptr);
    std::string code(raw);
    env->ReleaseStringUTFChars(line_code, raw);

    Line line = LINES.at(code);
    Language lang = static_cast<Language>(langInt);

    std::vector<std::string> result = {"––"}; // Have an empty thing first

    for (int i = 1; i <= 99; ++i) {
        if (!validStation(line, i)) continue;
        Station s{line, i};
        result.push_back(
                stationToCode(s) + " – " + getName(s, lang)
        );
    }

    jobjectArray arr = env->NewObjectArray(
            result.size(),
            env->FindClass("java/lang/String"),
            nullptr
    );

    for (int i = 0; i < result.size(); ++i) {
        env->SetObjectArrayElement(
                arr, i, env->NewStringUTF(result[i].c_str())
        );
    }

    return arr;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_shun4midx_mrt_MainActivity_getLines(
        JNIEnv* env,
        jobject
) {
    jclass cls = env->FindClass("com/shun4midx/mrt/LineItem");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

    jobjectArray arr = env->NewObjectArray(
            LINE_EMOJIS.size(),
            cls,
            nullptr
    );

    int i = 0;
    for (Line line : LINE_ORDER) {
        const auto& emoji = LINE_EMOJIS.at(line);

        jobject obj = env->NewObject(
                cls,
                ctor,
                env->NewStringUTF(LINE_TO_STR.at(line).c_str()),
                env->NewStringUTF(emoji.c_str())
        );

        env->SetObjectArrayElement(arr, i++, obj);
    }

    return arr;
}

void getTaipeiTime(int* day_type, int* now_mins) {

    using namespace std::chrono;

    // Get current UTC time
    auto now = system_clock::now();
    auto utc_time = system_clock::to_time_t(now);

    // Convert to tm in UTC
    std::tm utc_tm = *std::gmtime(&utc_time);

    // Add 8 hours manually for UTC+8
    utc_tm.tm_hour += 8;

    // Normalize overflow (e.g. hour >= 24)
    std::mktime(&utc_tm);

    // Convert weekday
    // tm_wday: Sunday=0 ... Saturday=6
    int wday = utc_tm.tm_wday;

    *day_type = (wday == 0) ? 7 : wday;  // Monday=1 ... Sunday=7 (Need to add holiday fix later)

    *now_mins = utc_tm.tm_hour * 60 + utc_tm.tm_min;

    // Exceptions (idk like set the cap till 2am)

    if (*now_mins <= 120) {
        *now_mins = *now_mins + 24 * 60;
        *day_type = (*day_type + 6) % 7;
    }
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_shun4midx_mrt_MainActivity_getNextTrainTable(JNIEnv* env, jobject, jstring line_code, jint station, jint max_rows, jint max_cols, jint langInt) {
    jclass stringClass = env->FindClass("java/lang/String");
    jclass stringArrayClass = env->FindClass("[Ljava/lang/String;"); // <-- String[]
    Language lang = static_cast<Language>(langInt);

    // Calculate the next trains
    const char* raw = env->GetStringUTFChars(line_code, nullptr);
    std::string code(raw);
    env->ReleaseStringUTFChars(line_code, raw);

    Line l = LINES.at(code);
    Station stn = Station{l, station};

    int day_type, now_mins;
    getTaipeiTime(&day_type, &now_mins);

    std::vector<Station> stns = ARRIVAL_DESTS.at(l);
    std::vector<std::vector<int>> table_times;
    for (int i = 0; i < stns.size(); ++i) {
        table_times.push_back({});
    }

    if (validStation(stn) && stn.line != BR) {
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

        while (it != train_schedule.end()) {
            // Find position in stns and push back time if < 4
            int idx = 0;

            for (const Station& s : stns) {
                if (sameStation(s, it->train_dest)) {
                    break;
                }
                ++idx;
            }

            if (table_times[idx].size() >= 4) {
                ++it; // Increment
                continue;
            }

            // Push back
            table_times[idx].push_back(it->time - now_mins);

            // Next station
            ++it;
        }
    }

    // Outer: String[][]
    jobjectArray rows = env->NewObjectArray(stns.size(), stringArrayClass, nullptr);

    std::string first_label;
    std::string last_label;

    std::vector<std::vector<Time>> first_last_times;

    if (l == BR) {
        switch (lang) {
            case en:
                first_label = "First: ";
                last_label  = "Last: ";
                break;
            case jp:
                first_label = "始発: ";
                last_label  = "最終: ";
                break;
            case kr:
                first_label = "첫차: ";
                last_label  = "막차: ";
                break;
            default:
                first_label = "首班車: ";
                last_label  = "末班車: ";
        }

        Time first_01 = (station != 1) ? firstTrainTime(stn, day_type, Station{BR, 1}) : INVALID_TIME;
        Time last_01 = (station != 1) ? lastTrainTime(stn, day_type, Station{BR, 1}) : INVALID_TIME;
        Time first_24 = (station != 24) ? firstTrainTime(stn, day_type, Station{BR, 24}) : INVALID_TIME;
        Time last_24 = (station != 24) ? lastTrainTime(stn, day_type, Station{BR, 24}) : INVALID_TIME;

        first_last_times = {{first_01, last_01}, {first_24, last_24}};
    }

    for (int i = 0; i < stns.size(); ++i) {
        // Inner: String[]
        jobjectArray cols = env->NewObjectArray(max_cols, stringClass, nullptr);

        for (int j = 0; j < max_cols; ++j) {
            std::string s;

            if (station == 0) {
                s = "";
            } else {
                if (j == 0) {
                    s = LINE_EMOJIS.at(l) + " " + stationToCode(stns[i]);
                }

                if (j != 0) {
                    if (j - 1 < table_times[i].size() && table_times[i][j - 1] >= 0) {
                        s = std::to_string(table_times[i][j - 1]) + MINS.at(lang);
                    } else if (l != BR) {
                        s = "––";
                    } else { // BR
                        // Output label
                        if (j == 1) {
                            s = first_label;
                        } else if (j == 3) {
                            s = last_label;
                        }

                        // Output train times
                        if (j == 2) { // first train
                            if (!sameTime(first_last_times[i][0], INVALID_TIME)) {
                                s = timeToStr(first_last_times[i][0]);
                            } else {
                                s = "––";
                            }
                        } else if (j == 4) { // last train
                            if (!sameTime(first_last_times[i][1], INVALID_TIME)) {
                                s = timeToStr(first_last_times[i][1]);
                            } else {
                                s = "––";
                            }
                        }
                    }
                }
            }

            jstring js = env->NewStringUTF(s.c_str());
            env->SetObjectArrayElement(cols, j, js);
            env->DeleteLocalRef(js);
        }

        env->SetObjectArrayElement(rows, i, cols);
        env->DeleteLocalRef(cols);
    }

    return rows;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_shun4midx_mrt_MainActivity_getFare(JNIEnv* env, jobject, jstring line1_code, jint st1, jstring line2_code, jint st2, jint ageInt) {
    const char* raw1 = env->GetStringUTFChars(line1_code, nullptr);
    std::string code1(raw1);
    env->ReleaseStringUTFChars(line1_code, raw1);

    const char* raw2 = env->GetStringUTFChars(line2_code, nullptr);
    std::string code2(raw2);
    env->ReleaseStringUTFChars(line2_code, raw2);

    Line l1 = LINES.at(code1);
    Line l2 = LINES.at(code2);

    Station a{l1, (int)st1};
    Station b{l2, (int)st2};

    // ticket type
    TicketType type;

    switch (ageInt) {
        case 0: // Java CHILD
            type = CHILD;
            break;
        case 1: // Java ADULT
            type = ADULT;
            break;
        case 2: // Java ELDERLY
            type = ELDERLY;
            break;
        default:
            type = ADULT;
    }

    int fare = travelPrice(a, b, type);

    return (jint)fare;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_shun4midx_mrt_MainActivity_computeFastestRoute(JNIEnv* env, jobject, jstring line1_code, jint st1, jstring line2_code, jint st2, jint langInt, jint ticketInt) {

    const char* raw1 = env->GetStringUTFChars(line1_code, nullptr);
    std::string code1(raw1);
    env->ReleaseStringUTFChars(line1_code, raw1);

    const char* raw2 = env->GetStringUTFChars(line2_code, nullptr);
    std::string code2(raw2);
    env->ReleaseStringUTFChars(line2_code, raw2);

    Line l1 = LINES.at(code1);
    Line l2 = LINES.at(code2);

    Station src{l1, (int)st1};
    Station dst{l2, (int)st2};

    Language lang = static_cast<Language>(langInt);

    // ticket type
    TicketType type;

    switch (ticketInt) {
        case 0: // Java CHILD
            type = CHILD;
            break;
        case 1: // Java ADULT
            type = ADULT;
            break;
        case 2: // Java ELDERLY
            type = ELDERLY;
            break;
        default:
            type = ADULT;
    }

    int day_type, now_mins;
    getTaipeiTime(&day_type, &now_mins);

    std::vector<RoutedPath> results = routeDefault(src, dst, Time{now_mins / 60, now_mins % 60}, day_type, 3);

    std::string output;

    for (const auto& rp : results) {
        output += pathDetailsToUser(rp.path, Time{now_mins / 60, now_mins % 60}, day_type, lang, type);
        output += "\n\n";
    }

    return env->NewStringUTF(output.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_shun4midx_mrt_MainActivity_computeLeastInterchangeRoute(JNIEnv* env, jobject, jstring line1_code, jint st1, jstring line2_code, jint st2, jint langInt, jint ticketInt) {

    const char* raw1 = env->GetStringUTFChars(line1_code, nullptr);
    std::string code1(raw1);
    env->ReleaseStringUTFChars(line1_code, raw1);

    const char* raw2 = env->GetStringUTFChars(line2_code, nullptr);
    std::string code2(raw2);
    env->ReleaseStringUTFChars(line2_code, raw2);

    Line l1 = LINES.at(code1);
    Line l2 = LINES.at(code2);

    Station src{l1, (int)st1};
    Station dst{l2, (int)st2};

    Language lang = static_cast<Language>(langInt);

    // ticket type
    TicketType type;

    switch (ticketInt) {
        case 0: // Java CHILD
            type = CHILD;
            break;
        case 1: // Java ADULT
            type = ADULT;
            break;
        case 2: // Java ELDERLY
            type = ELDERLY;
            break;
        default:
            type = ADULT;
    }

    int day_type, now_mins;
    getTaipeiTime(&day_type, &now_mins);

    std::vector<RoutedPath> results = routeLeastInterchange(src, dst, Time{now_mins / 60, now_mins % 60}, day_type, 3);

    std::string output;

    for (const auto& rp : results) {
        output += pathDetailsToUser(rp.path, Time{now_mins / 60, now_mins % 60}, day_type, lang, type);
        output += "\n\n";
    }

    if (output.size() > 0) {
        output.pop_back();
    }

    return env->NewStringUTF(output.c_str());
}