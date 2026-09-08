#include "utils.h"

#include <cassert>
#include <iostream>
#include <vector>

void testBasic() {
    Station r01{R, 1};
    Station r02{R, 2};

    assert(validStation(r01));
    assert(validStation(r02));

    assert(stationToCode(r01) == "R01");
    assert(stationToCode(r02) == "R02");

    assert(sameStation(codeToStation("R01"), r01));
    assert(sameStation(codeToStation("R02"), r02));

    std::cout << "[PASS] basic R01 station support\n";
}

void testDuration() {
    Station r01{R, 1};
    Station r02{R, 2};
    Station r03{R, 3};
    Station r28{R, 28};

    // If your prefix values are R01=-3, R02=0
    assert(getLineDuration(r01, r02) == 3);
    assert(getLineDuration(r02, r01) == 3);

    // Existing durations should still behave normally
    assert(getLineDuration(r01, r03)
           == getLineDuration(r01, r02)
            + getLineDuration(r02, r03));

    assert(getLineDuration(r01, r28)
           == getLineDuration(r01, r02)
            + getLineDuration(r02, r28));

    std::cout << "[PASS] R01 durations\n";
}

void testScheduleSorted(const Station& stn, int day) {
    std::vector<Train> trains = loadStationSchedule(stn, day);

    assert(!trains.empty());

    for (size_t i = 1; i < trains.size(); ++i) {
        assert(trains[i - 1].time <= trains[i].time);
    }

    std::cout
        << "[PASS] "
        << stationToCode(stn)
        << " day "
        << day
        << " schedule sorted ("
        << trains.size()
        << " trains)\n";
}

void testR01Destinations(int day) {
    Station r01{R, 1};

    auto trains = loadStationSchedule(r01, day);
    assert(!trains.empty());

    for (const Train& train : trains) {
        // Every train leaving R01 is northbound
        assert(train.direction == 0);

        // And terminates at either Beitou or Tamsui
        assert(
            sameStation(train.train_dest, Station{R, 22}) ||
            sameStation(train.train_dest, Station{R, 28})
        );
    }

    std::cout
        << "[PASS] R01 destination/direction data day "
        << day
        << "\n";
}

void testR02SouthboundDestinations(int day) {
    Station r02{R, 2};

    auto trains = loadStationSchedule(r02, day);
    assert(!trains.empty());

    bool foundSouthbound = false;

    for (const Train& train : trains) {
        if (train.direction == 1) {
            foundSouthbound = true;

            // All decreasing-direction trains at R02
            // should now terminate at R01.
            assert(sameStation(train.train_dest, Station{R, 1}));
        }
    }

    assert(foundSouthbound);

    std::cout
        << "[PASS] R02 southbound trains terminate at R01 day "
        << day
        << "\n";
}

void testFirstTrains() {
    Station r01{R, 1};
    Station r02{R, 2};
    Station r28{R, 28};

    // Weekday R01 -> north
    Time t1 = nextTrainTime(
        r01,
        1,
        Time{6, 0},
        r28
    );

    std::cout
        << "Weekday R01 -> R28 next train at 06:00: "
        << timeToStr(t1)
        << "\n";

    // Based on your entered weekday R01 timetable
    assert(sameTime(t1, Time{6, 0}));

    // R02 -> R01
    Time t2 = nextTrainTime(
        r02,
        1,
        Time{6, 0},
        r01
    );

    std::cout
        << "Weekday R02 -> R01 next train at 06:00: "
        << timeToStr(t2)
        << "\n";

    // Your R02 weekday raw data starts at 06:05 southbound
    assert(sameTime(t2, Time{6, 5}));

    std::cout << "[PASS] first train lookup\n";
}

void testLateTrain() {
    Station r01{R, 1};
    Station r28{R, 28};

    Time t = nextTrainTime(
        r01,
        1,
        Time{23, 55},
        r28
    );

    std::cout
        << "Weekday R01 -> R28 after 23:55: "
        << timeToStr(t)
        << "\n";

    // Your R01 weekday file contains 24:00.
    assert(sameTime(t, Time{24, 0}));

    std::cout << "[PASS] post-midnight timetable lookup\n";
}

void testReachability() {
    Station r01{R, 1};

    Train tamsuiTrain{
        r01,
        360,
        0,
        Station{R, 28}
    };

    Train beitouTrain{
        r01,
        360,
        0,
        Station{R, 22}
    };

    assert(oneTrainReachDest(r01, Station{R, 2}, tamsuiTrain));
    assert(oneTrainReachDest(r01, Station{R, 28}, tamsuiTrain));

    assert(oneTrainReachDest(r01, Station{R, 2}, beitouTrain));
    assert(oneTrainReachDest(r01, Station{R, 22}, beitouTrain));

    // Beitou shuttle must NOT claim it reaches Tamsui.
    assert(!oneTrainReachDest(r01, Station{R, 28}, beitouTrain));

    std::cout << "[PASS] R01 train reachability\n";
}

void testFare() {
    Station r01{R, 1};
    Station r02{R, 2};
    Station r03{R, 3};
    Station r10{R, 10};

    // Temporary free extension
    assert(travelPrice(r01, r02) == 0);
    assert(travelPrice(r02, r01) == 0);

    // Other R01 fares should behave like starting from R02.
    assert(
        travelPrice(r01, r03)
        == travelPrice(r02, r03)
    );

    assert(
        travelPrice(r01, r10)
        == travelPrice(r02, r10)
    );

    std::cout << "[PASS] temporary R01 fare logic\n";
}

void testPathETA() {
    Path p = {
        Station{R, 1},
        Station{R, 2},
        Station{R, 3}
    };

    PathTimes pt = pathETA(
        p,
        Time{6, 0},
        1
    );

    assert(pt.size() == 3);

    std::cout << "R01 -> R02 -> R03 ETA:\n";

    for (size_t i = 0; i < pt.size(); ++i) {
        std::cout
            << "  "
            << stationToCode(p[i])
            << ": arrive "
            << timeToStr(pt[i].first)
            << ", depart "
            << timeToStr(pt[i].second)
            << "\n";
    }

    std::cout << "[PASS] pathETA through R01\n";
}

int main() {
    try {
        testBasic();
        testDuration();

        // Monday / Saturday / Sunday
        for (int day : {1, 6, 7}) {
            testScheduleSorted(Station{R, 1}, day);
            testScheduleSorted(Station{R, 2}, day);

            testR01Destinations(day);
            testR02SouthboundDestinations(day);
        }

        testFirstTrains();
        testLateTrain();
        testReachability();
        testFare();
        testPathETA();

        std::cout << "\n==============================\n";
        std::cout << "ALL R01/R02 TESTS PASSED :D\n";
        std::cout << "==============================\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED WITH EXCEPTION:\n";
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}