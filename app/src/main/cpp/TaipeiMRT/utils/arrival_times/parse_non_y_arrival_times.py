############################################
# Copyright (c) 2026 Shun/翔海 (@shun4midx) #
# Project: Taipei-MRT-Scheduler            #
# File Type: Python file                   #
# File: parse_non_y_arrival_times.py       #
############################################

# There are 12345, 6, 7, 67 as possible CSV names. They are for weekdays, Sat, Sun, Sat and Sun respectively.

import csv, os
from collections import defaultdict

def extract_time(dep):
    # "{1,,,06:23,}" -> "06:23"
    dep = dep.strip("{}")
    return dep.split(",")[3]

def to_mins(hhmm): # Store the number of mins after today's midnight 00:00
    h, m = map(int, hhmm.split(":"))
    minute = h * 60 + m
    day_offset = 1 if h < 3 else 0
    return minute + day_offset * 60 * 24

def parse_raw_csv(path):
    buckets = defaultdict(list)

    with open(path, encoding="cp950", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            # ---- guard: direction ----
            dir_str = row["Direction"].strip()
            if dir_str == "":
                continue
            direction = int(dir_str)

            # ---- guard: departure time ----
            dep = row["DepartureTimes"].strip()
            if not dep or dep == "{}":
                continue

            # ---- now safe to parse ----
            station = row["StationID"]

            buckets[station].append({
                "line": row["RouteID"],
                "destination": row["DestinationStaionID"], # Gov typed "Station" wrong lmfao
                "direction": int(row["Direction"]),
                "time": to_mins(extract_time(row["DepartureTimes"]))
            })

    return buckets

def write_station_csvs(line, suffix, buckets, base_out="arrival_times/generated"):
    out_dir = os.path.join(base_out, line)
    os.makedirs(out_dir, exist_ok=True)

    for station, trains in buckets.items():
        out_path = os.path.join(out_dir, f"{station}_{suffix}.csv")

        with open(out_path, "w", encoding="utf8", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["line", "destination", "direction", "time"])

            for t in sorted(trains, key=lambda x: x["time"]):
                writer.writerow([
                    t["line"],
                    t["destination"],
                    t["direction"],
                    t["time"]
                ])

# Main
if __name__ == "__main__":
    RAW_BASE = "raw"
    GENERATED_BASE = "generated"

    for line in os.listdir(RAW_BASE):
        line_dir = os.path.join(RAW_BASE, line)
        if not os.path.isdir(line_dir):
            continue

        for filename in os.listdir(line_dir):
            if not filename.endswith(".csv"):
                continue

            suffix = filename.replace(".csv", "")
            path = os.path.join(line_dir, filename)

            buckets = parse_raw_csv(path)
            write_station_csvs(line, suffix, buckets, GENERATED_BASE)