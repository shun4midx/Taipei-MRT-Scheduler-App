/********************************************
 * Copyright (c) 2026 Shun/翔海 (@shun4midx) *
 * Project: Taipei-MRT-Scheduler            *
 * File Type: C++ file                      *
 * File: basic.cpp                          *
 ****************************************** */

#include "basic.h"
#include <stdexcept>
#include <regex>

// ======== BASIC DEFINITIONS ======== //
const std::unordered_map<Line, std::string> LINE_TO_STR = {
    {R, "R"}, {O, "O"}, {G, "G"}, {BL, "BL"}, {BR, "BR"}, {Y, "Y"}
};

const std::unordered_map<std::string, Line> LINES = {
    {"R", R}, {"O", O}, {"G", G}, {"BL", BL}, {"BR", BR}, {"Y", Y}
};

const Station INVALID_STATION = Station{R, -1};

// ======== DEFINE VARIABLES ======== //
const std::string INVALID = "N/A";

const Time INVALID_TIME = Time{-1, -1};

// For convenience, we make the array index align with the number of the station (except for orange line, that's an exception, for O50 branch we minus 50)
std::vector<std::unordered_map<Language, std::string>> R_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "象山"}, {en, "Xiangshan"}, {jp, "象山"}, {kr, "샹산"}}, // R02
    {{zh, "台北101/世貿"}, {en, "Taipei 101/World Trade Center"}, {jp, "台北101/世界貿易センター"}, {kr, "타이베이101/세계무역센터"}}, // R03
    {{zh, "信義安和"}, {en, "Xinyi Anhe"}, {jp, "信義安和"}, {kr, "신이 안허"}}, // R04
    {{zh, "大安"}, {en, "Daan"}, {jp, "大安"}, {kr, "다안"}}, // R05
    {{zh, "大安森林公園"}, {en, "Daan Park"}, {jp, "大安森林公園"}, {kr, "다안 삼림 공원"}}, // R06
    {{zh, "東門"}, {en, "Dongmen"}, {jp, "東門"}, {kr, "둥먼"}}, // R07
    {{zh, "中正紀念堂"}, {en, "Chiang Kai-Shek Memorial Hall"}, {jp, "中正紀念堂"}, {kr, "중정 기념당"}}, // R08
    {{zh, "台大醫院"}, {en, "NTU Hospital"}, {jp, "台湾大学病院"}, {kr, "대만 대학 병원"}}, // R09
    {{zh, "台北車站"}, {en, "Taipei Main Station"}, {jp, "台北駅"}, {kr, "타이베이 역"}}, // R10
    {{zh, "中山"}, {en, "Zhongshan"}, {jp, "中山"}, {kr, "중산"}}, // R11
    {{zh, "雙連"}, {en, "Shuanglian"}, {jp, "双連"}, {kr, "솽롄"}}, // R12
    {{zh, "民權西路"}, {en, "Minquan W. Rd."}, {jp, "民権西路"}, {kr, "민취안시루"}}, // R13
    {{zh, "圓山"}, {en, "Yuanshan"}, {jp, "圓山"}, {kr, "위안산"}}, // R14
    {{zh, "劍潭"}, {en, "Jiantan"}, {jp, "剣潭"}, {kr, "젠탄"}}, // R15
    {{zh, "士林"}, {en, "Shilin"}, {jp, "士林"}, {kr, "스린"}}, // R16
    {{zh, "芝山"}, {en, "Zhishan"}, {jp, "芝山"}, {kr, "즈산"}}, // R17
    {{zh, "明德"}, {en, "Mingde"}, {jp, "明徳"}, {kr, "밍더"}}, // R18
    {{zh, "石牌"}, {en, "Shipai"}, {jp, "石牌"}, {kr, "스파이"}}, // R19
    {{zh, "唭哩岸"}, {en, "Qilian"}, {jp, "唭哩岸"}, {kr, "치리안"}}, // R20
    {{zh, "奇岩"}, {en, "Qiyan"}, {jp, "奇岩"}, {kr, "치옌"}}, // R21
    {{zh, "北投"}, {en, "Beitou"}, {jp, "北投"}, {kr, "베이터우"}}, // R22
    {{zh, "復興崗"}, {en, "Fuxinggang"}, {jp, "復興崗"}, {kr, "푸싱강"}}, // R23
    {{zh, "忠義"}, {en, "Zhongyi"}, {jp, "忠義"}, {kr, "중이"}}, // R24
    {{zh, "關渡"}, {en, "Guandu"}, {jp, "関渡"}, {kr, "관두"}}, // R25
    {{zh, "竹圍"}, {en, "Zhuwei"}, {jp, "竹囲"}, {kr, "주웨이"}}, // R26
    {{zh, "紅樹林"}, {en, "Hongshulin"}, {jp, "紅樹林"}, {kr, "훙수린"}}, // R27
    {{zh, "淡水"}, {en, "Tamsui"}, {jp, "淡水"}, {kr, "단수이"}} // R28
};

std::vector<std::unordered_map<Language, std::string>> G_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "新店"}, {en, "Xindian"}, {jp, "新店"}, {kr, "신뎬"}}, // G01
    {{zh, "新店區公所"}, {en, "Xindian District Office"}, {jp, "新店区役所"}, {kr, "신뎬 구청"}}, // G02
    {{zh, "七張"}, {en, "Qizhang"}, {jp, "七張"}, {kr, "치장"}}, // G03
    {{zh, "大坪林"}, {en, "Dapinglin"}, {jp, "大坪林"}, {kr, "다핑린"}}, // G04
    {{zh, "景美"}, {en, "Jingmei"}, {jp, "景美"}, {kr, "징메이"}}, // G05
    {{zh, "萬隆"}, {en, "Wanlong"}, {jp, "万隆"}, {kr, "완룽"}}, // G06
    {{zh, "公館"}, {en, "Gongguan"}, {jp, "公館"}, {kr, "궁관"}}, // G07
    {{zh, "台電大樓"}, {en, "Taipower Building"}, {jp, "台湾電力ビル"}, {kr, "대만 전력공사 빌딩"}}, // G08
    {{zh, "古亭"}, {en, "Guting"}, {jp, "古亭"}, {kr, "구팅"}}, // G09
    {{zh, "中正紀念堂"}, {en, "Chiang Kai-Shek Memorial Hall"}, {jp, "中正紀念堂"}, {kr, "중정 기념당"}}, // G10
    {{zh, "小南門"}, {en, "Xiaonanmen"}, {jp, "小南門"}, {kr, "샤오난먼"}}, // G11
    {{zh, "西門"}, {en, "Ximen"}, {jp, "西門"}, {kr, "시먼"}}, // G12
    {{zh, "北門"}, {en, "Beimen"}, {jp, "北門"}, {kr, "베이머"}}, // G13
    {{zh, "中山"}, {en, "Zhongshan"}, {jp, "中山"}, {kr, "중산"}}, // G14
    {{zh, "松江南京"}, {en, "Songjiang Nanjing"}, {jp, "松江南京"}, {kr, "송장 난징"}}, // G15
    {{zh, "南京復興"}, {en, "Nanjing Fuxing"}, {jp, "南京復興"}, {kr, "난징 푸싱"}}, // G16
    {{zh, "台北小巨蛋"}, {en, "Taipei Arena"}, {jp, "台北アリーナ"}, {kr, "타이베이 아레나"}}, // G17
    {{zh, "南京三民"}, {en, "Nanjing Sanmin"}, {jp, "南京三民"}, {kr, "난징 싼민"}}, // G18
    {{zh, "松山"}, {en, "Songshan"}, {jp, "松山"}, {kr, "송산"}} // G19
};

std::vector<std::unordered_map<Language, std::string>> BL_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "頂埔"}, {en, "Dingpu"}, {jp, "頂埔"}, {kr, "딩푸"}}, // BL01
    {{zh, "永寧"}, {en, "Yongning"}, {jp, "永寧"}, {kr, "융닝"}}, // BL02
    {{zh, "土城"}, {en, "Tucheng"}, {jp, "土城"}, {kr, "투청"}}, // BL03
    {{zh, "海山"}, {en, "Haishan"}, {jp, "海山"}, {kr, "하이산"}}, // BL04
    {{zh, "亞東醫院"}, {en, "Far Eastern Hospital"}, {jp, "亜東病院"}, {kr, "야둥 병원"}}, // BL05
    {{zh, "府中"}, {en, "Fuzhong"}, {jp, "府中"}, {kr, "푸중"}}, // BL06
    {{zh, "板橋"}, {en, "Banqiao"}, {jp, "板橋"}, {kr, "반차오"}}, // BL07
    {{zh, "新埔"}, {en, "Xinpu"}, {jp, "新埔"}, {kr, "신푸"}}, // BL08
    {{zh, "江子翠"}, {en, "Jiangzicui"}, {jp, "江子翠"}, {kr, "장쯔추이"}}, // BL09
    {{zh, "龍山寺"}, {en, "Longshan Temple"}, {jp, "龍山寺"}, {kr, "용산사"}}, // BL10
    {{zh, "西門"}, {en, "Ximen"}, {jp, "西門"}, {kr, "시먼"}}, // BL11
    {{zh, "台北車站"}, {en, "Taipei Main Station"}, {jp, "台北駅"}, {kr, "타이베이 역"}}, // BL12
    {{zh, "善導寺"}, {en, "Shandao Temple"}, {jp, "善導寺"}, {kr, "산다오사"}}, // BL13
    {{zh, "忠孝新生"}, {en, "Zhongxiao Xinsheng"}, {jp, "忠孝新生"}, {kr, "중샤오 신성"}}, // BL14
    {{zh, "忠孝復興"}, {en, "Zhongxiao Fuxing"}, {jp, "忠孝復興"}, {kr, "중샤오 푸싱"}}, // BL15
    {{zh, "忠孝敦化"}, {en, "Zhongxiao Dunhua"}, {jp, "忠孝敦化"}, {kr, "중샤오 둔화"}}, // BL16
    {{zh, "國父紀念館"}, {en, "Sun Yat-Sen Memorial Hall"}, {jp, "国父紀念館"}, {kr, "국부 기념관"}}, // BL17
    {{zh, "市政府"}, {en, "Taipei City Hall"}, {jp, "台北市政府"}, {kr, "타이베이 시청"}}, // BL18
    {{zh, "永春"}, {en, "Yongchun"}, {jp, "永春"}, {kr, "융춘"}}, // BL19
    {{zh, "後山埤"}, {en, "Houshanpi"}, {jp, "後山埤"}, {kr, "허우산피"}}, // BL20
    {{zh, "昆陽"}, {en, "Kunyang"}, {jp, "昆陽"}, {kr, "쿤양"}}, // BL21
    {{zh, "南港"}, {en, "Nangang"}, {jp, "南港"}, {kr, "난강"}}, // BL22
    {{zh, "南港展覽館"}, {en, "Taipei Nangang Exhibition Center"}, {jp, "南港展覧館"}, {kr, "난강 전람관"}} // BL23
};

std::vector<std::unordered_map<Language, std::string>> BR_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "動物園"}, {en, "Taipei Zoo"}, {jp, "動物園"}, {kr, "동물원"}}, // BR01
    {{zh, "木柵"}, {en, "Muzha"}, {jp, "木柵"}, {kr, "무자"}}, // BR02
    {{zh, "萬芳社區"}, {en, "Wanfang Community"}, {jp, "万芳コミュニティ"}, {kr, "완팡 단지"}}, // BR03
    {{zh, "萬芳醫院"}, {en, "Wanfang Hospital"}, {jp, "万芳病院"}, {kr, "완팡 병원"}}, // BR04
    {{zh, "辛亥"}, {en, "Xinhai"}, {jp, "辛亥"}, {kr, "신하이"}}, // BR05
    {{zh, "麟光"}, {en, "Linguang"}, {jp, "麟光"}, {kr, "린광"}}, // BR06
    {{zh, "六張犁"}, {en, "Liuzhangli"}, {jp, "六張犁"}, {kr, "류장리"}}, // BR07
    {{zh, "科技大樓"}, {en, "Technology Building"}, {jp, "テクノロジービル"}, {kr, "테크놀로지 빌딩"}}, // BR08
    {{zh, "大安"}, {en, "Daan"}, {jp, "大安"}, {kr, "다안"}}, // BR09
    {{zh, "忠孝復興"}, {en, "Zhongxiao Fuxing"}, {jp, "忠孝復興"}, {kr, "중샤오 푸싱"}}, // BR10
    {{zh, "南京復興"}, {en, "Nanjing Fuxing"}, {jp, "南京復興"}, {kr, "난징 푸싱"}}, // BR11
    {{zh, "中山國中"}, {en, "Zhongshan Junior High School"}, {jp, "中山中学校"}, {kr, "중산 중학교"}}, // BR12
    {{zh, "松山機場"}, {en, "Songshan Airport"}, {jp, "松山空港"}, {kr, "송산 공항"}}, // BR13
    {{zh, "大直"}, {en, "Dazhi"}, {jp, "大直"}, {kr, "다즈"}}, // BR14
    {{zh, "劍南路"}, {en, "Jiannan Rd."}, {jp, "剣南路"}, {kr, "젠난루"}}, // BR15
    {{zh, "西湖"}, {en, "Xihu"}, {jp, "西湖"}, {kr, "시후"}}, // BR16
    {{zh, "港墘"}, {en, "Gangqian"}, {jp, "港墘"}, {kr, "강첸"}}, // BR17
    {{zh, "文德"}, {en, "Wende"}, {jp, "文徳"}, {kr, "원더"}}, // BR18
    {{zh, "內湖"}, {en, "Neihu"}, {jp, "内湖"}, {kr, "네이후"}}, // BR19
    {{zh, "大湖公園"}, {en, "Dahu Park"}, {jp, "大湖公園"}, {kr, "다후 공원"}}, // BR20
    {{zh, "葫州"}, {en, "Huzhou"}, {jp, "葫洲"}, {kr, "후저우"}}, // BR21
    {{zh, "東湖"}, {en, "Donghu"}, {jp, "東湖"}, {kr, "둥후"}}, // BR22
    {{zh, "南港軟體園區"}, {en, "Nangang Software Park"}, {jp, "南港ソフトウェアパーク"}, {kr, "난강 소프트웨어 단지"}}, // BR23
    {{zh, "南港展覽館"}, {en, "Taipei Nangang Exhibition Center"}, {jp, "南港展覧館"}, {kr, "난강 전람관"}} // BR24
};

std::vector<std::unordered_map<Language, std::string>> Y_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "大坪林"}, {en, "Dapinglin"}, {jp, "大坪林"}, {kr, "다핑린"}}, // Y07
    {{zh, "十四張"}, {en, "Shisizhang"}, {jp, "十四張"}, {kr, "스쓰장"}}, // Y08
    {{zh, "秀朗橋"}, {en, "Xiulang Bridge"}, {jp, "秀朗橋"}, {kr, "시우랑챠오"}}, // Y09
    {{zh, "景平"}, {en, "Jingping"}, {jp, "景平"}, {kr, "징핑"}}, // Y10
    {{zh, "景安"}, {en, "Jingan"}, {jp, "景安"}, {kr, "징안"}}, // Y11
    {{zh, "中和"}, {en, "Zhonghe"}, {jp, "中和"}, {kr, "중허"}}, // Y12
    {{zh, "橋和"}, {en, "Qiaohe"}, {jp, "橋和"}, {kr, "챠오허"}}, // Y13
    {{zh, "中原"}, {en, "Zhongyuan"}, {jp, "中原"}, {kr, "중위엔"}}, // Y14
    {{zh, "板新"}, {en, "Banxin"}, {jp, "板新"}, {kr, "반신"}}, // Y15
    {{zh, "板橋"}, {en, "Banqiao"}, {jp, "板橋"}, {kr, "반차오"}}, // Y16
    {{zh, "新埔民生"}, {en, "Xinpu Minsheng"}, {jp, "新埔民生"}, {kr, "신푸민셩"}}, // Y17
    {{zh, "頭前庄"}, {en, "Touqianzhuang"}, {jp, "頭前庄"}, {kr, "터우첸좡"}}, // Y18
    {{zh, "幸福"}, {en, "Xingfu"}, {jp, "幸福"}, {kr, "씽푸"}}, // Y19
    {{zh, "新北產業園區"}, {en, "New Taipei Industrial Park"}, {jp, "新北産業園区"}, {kr, "신베이 산업원 단지"}} // Y20
};

std::vector<std::unordered_map<Language, std::string>> O01_NAMES = {
    {{zh, INVALID}, {en, INVALID}, {jp, INVALID}, {kr, INVALID}},
    {{zh, "南勢角"}, {en, "Nanshijiao"}, {jp, "南勢角"}, {kr, "난스자오"}}, // O01
    {{zh, "景安"}, {en, "Jingan"}, {jp, "景安"}, {kr, "징안"}}, // O02
    {{zh, "永安市場"}, {en, "Yongan Market"}, {jp, "永安市場"}, {kr, "융안 시장"}}, // O03
    {{zh, "頂溪"}, {en, "Dingxi"}, {jp, "頂渓"}, {kr, "딩시"}}, // O04
    {{zh, "古亭"}, {en, "Guting"}, {jp, "古亭"}, {kr, "구팅"}}, // O05
    {{zh, "東門"}, {en, "Dongmen"}, {jp, "東門"}, {kr, "둥먼"}}, // O06
    {{zh, "忠孝新生"}, {en, "Zhongxiao Xinsheng"}, {jp, "忠孝新生"}, {kr, "중샤오 신성"}}, // O07
    {{zh, "松江南京"}, {en, "Songjiang Nanjing"}, {jp, "松江南京"}, {kr, "송장 난징"}}, // O08
    {{zh, "行天宮"}, {en, "Xingtian Temple"}, {jp, "行天宮"}, {kr, "싱톈궁"}}, // O09
    {{zh, "中山國小"}, {en, "Zhongshan Elementary School"}, {jp, "中山小学校"}, {kr, "중산 초등학교"}}, // O10
    {{zh, "民權西路"}, {en, "Minquan W. Rd."}, {jp, "民権西路"}, {kr, "민취안시루"}}, // O11
    {{zh, "大橋頭"}, {en, "Daqiaotou"}, {jp, "大橋頭"}, {kr, "다차오터우"}}, // O12
    // ~~~~~~~~ Y SHAPE SPLIT ~~~~~~~~~ //
    {{zh, "台北橋"}, {en, "Taipei Bridge"}, {jp, "台北橋"}, {kr, "타이베이 대교"}}, // O13
    {{zh, "菜寮"}, {en, "Cailiao"}, {jp, "菜寮"}, {kr, "차이랴오"}}, // O14
    {{zh, "三重"}, {en, "Sanchong"}, {jp, "三重"}, {kr, "싼충"}}, // O15
    {{zh, "先嗇宮"}, {en, "Xianse Temple"}, {jp, "先嗇宮"}, {kr, "셴써궁"}}, // O16
    {{zh, "頭前庄"}, {en, "Touqianzhuang"}, {jp, "頭前庄"}, {kr, "터우첸좡"}}, // O17
    {{zh, "新莊"}, {en, "Xinzhuang"}, {jp, "新荘"}, {kr, "신좡"}}, // O18
    {{zh, "輔大"}, {en, "Fu Jen University"}, {jp, "輔仁大学"}, {kr, "푸런 대학교"}}, // O19
    {{zh, "丹鳳"}, {en, "Danfeng"}, {jp, "丹鳳"}, {kr, "단펑"}}, // O20
    {{zh, "迴龍"}, {en, "Huilong"}, {jp, "迴龍"}, {kr, "후이룽"}} // O21
};

std::vector<std::unordered_map<Language, std::string>> O50_NAMES = {
    {{zh, "三重國小"}, {en, "Sanchong Elementary School"}, {jp, "三重小学校"}, {kr, "싼충 초등학교"}}, // O50
    {{zh, "三和國中"}, {en, "Sanhe Junior High School"}, {jp, "三和中学校"}, {kr, "싼허 중학교"}}, // O51
    {{zh, "徐匯中學"}, {en, "St. Ignatius High School"}, {jp, "徐匯高校"}, {kr, "쉬후이 고등학교"}}, // O52
    {{zh, "三民高中"}, {en, "Sanmin Senior High School"}, {jp, "三民高校"}, {kr, "싼민 고등학교"}}, // O53
    {{zh, "蘆洲"}, {en, "Luzhou"}, {jp, "蘆洲"}, {kr, "루저우"}} // O54
};

// ======== RETRIEVAL FUNCTIONS ========= //
bool sameTime(const Time& time1, const Time& time2) {
    return time1.hr == time2.hr && time1.min == time2.min;
}

int timeToMins(const Time& time) {
    return time.hr * 60 + time.min;
}

Time minsToTime(int mins) {
    if (mins < 0 || mins > 26 * 60) { // No train arrives at 2am of the next day I think
        throw std::invalid_argument("Invalid argument: " + std::to_string(mins));
    }

    return Time{mins / 60, mins % 60};
}

std::string timeToStr(const Time& time) {
    if (sameTime(time, INVALID_TIME)) {
        return "INVALID_TIME";
    }
    
    return (time.hr < 10 ? "0" : "") + std::to_string(time.hr) + ":" + (time.min < 10 ? "0" : "") + std::to_string(time.min);
}

Time minsAfter(const Time& time, int mins) {
    return minsToTime(timeToMins(time) + mins);
}

bool sameStation(const Station& stn1, const Station& stn2) {
    return stn1.line == stn2.line && stn1.stn_num == stn2.stn_num;
}

bool validStation(const Station& station) {
    return validStation(station.line, station.stn_num);
}

bool validStation(int line_int, int stn_num) {
    try {
        getName(line_int, stn_num);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

Station makeStation(int line_int, int stn_num) {
    if (!validStation(line_int, stn_num)) {
        throw std::invalid_argument("Invalid argument");
    } else {
        return Station{static_cast<Line>(line_int), stn_num};
    }
}

std::string getName(const Station& station, int lang_int) {
    return getName(station.line, station.stn_num, lang_int);
}

std::string getName(int line_int, int stn_num, int lang_int) {
    // Detect invalid inputs
    if (line_int < Line::R || line_int > Line::Y) {
        throw std::invalid_argument("getName: Invalid Line");
    }

    if (lang_int < Language::zh || lang_int > Language::kr) {
        throw std::invalid_argument("getName: Invalid Language");
    }

    // Deal with inputs
    static const std::unordered_map<Line, const std::vector<std::unordered_map<Language, std::string>>*> LINE_NAMES = {
        {R, &R_NAMES},
        {G, &G_NAMES},
        {BL, &BL_NAMES},
        {BR, &BR_NAMES},
        {Y, &Y_NAMES}
    }; // except for O because that's an exception

    Line line = static_cast<Line>(line_int);
    Language lang = static_cast<Language>(lang_int);

    if (line != O) {
        const auto& names = *LINE_NAMES.at(line);
        if (stn_num <= 0 || stn_num >= names.size() || names[stn_num].at(lang) == INVALID) {
            throw std::invalid_argument("getName: No such station");
        }
        return names[stn_num].at(lang);
    } else {
        // Basic invalid
        if (stn_num <= 0 || stn_num >= 50 + O50_NAMES.size()) {
            throw std::invalid_argument("getName: No such station");
        }

        // O01 branch
        else if (stn_num < 50 && stn_num < O01_NAMES.size()) {
            return O01_NAMES[stn_num][lang];
        }

        // O50 branch
        else if (stn_num >= 50) {
            return O50_NAMES[stn_num - 50][lang];
        }

        else {
            throw std::invalid_argument("getName: No such station");
        }
    }
}

Station codeToStation(std::string stn_code) {
    static const std::regex re(R"(^([A-Z]{1,2})([0-9]{2})$)");
    std::smatch m;

    if (!std::regex_match(stn_code, m, re)) {
        throw std::invalid_argument("Invalid station code: " + stn_code);
    }

    std::string line_str = m[1]; // "BL"
    int stn_num = std::stoi(m[2]); // 23

    if (LINES.find(line_str) == LINES.end()) {
        throw std::invalid_argument("Invalid line code: " + line_str);
    } else {
        Line line = LINES.at(line_str);

        if (!validStation(line, stn_num)) {
            throw std::invalid_argument("Invalid station number for this line: " + stn_code);
        } else {
            return Station{line, stn_num};
        }
    }
}

std::string stationToCode(const Station& station) {
    if (!validStation(station)) {
        throw std::invalid_argument("Invalid station");
    }

    return LINE_TO_STR.at(station.line) + (station.stn_num < 10 ? "0" : "") + std::to_string(station.stn_num);
}