############################################
# Copyright (c) 2026 Shun/翔海 (@shun4midx) #
# Project: Taipei-MRT-Scheduler            #
# File Type: Python file                   #
# File: parse_y_arrival_times.py           #
############################################

import os, csv, re

RAW_Y_DIR = "raw/Y"
GEN_Y_DIR = "generated/Y"

DIR_MAP = {
    "Y-1": 0,  # increasing station number
    "Y-2": 1,  # decreasing station number
}

DEST_MAP = {
    "Y-1": "Y20",
    "Y-2": "Y07",
}

def hhmm_to_abs_minutes(hhmm: str) -> int:
    h, m = map(int, hhmm.split(":"))
    return h * 60 + m   # 24:xx naturally becomes >= 1440

def parse_y_txt(path: str):
    trips = []
    current_route = None

    with open(path, "r", encoding="utf8") as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue

            # Section header
            if line in DIR_MAP:
                current_route = line
                continue

            if current_route is None:
                continue

            # Time line: HH:MM,mm,mm,...
            parts = [p.strip() for p in line.split(",") if p.strip()]
            if not parts:
                continue

            if not re.fullmatch(r"\d{2}:\d{2}", parts[0]):
                continue

            base_hhmm = parts[0]
            base_hour = int(base_hhmm[:2])
            direction = DIR_MAP[current_route]

            # Base time
            trips.append((
                current_route,
                DEST_MAP[current_route],
                direction,
                hhmm_to_abs_minutes(base_hhmm),
            ))

            # Extra minutes in same hour
            for mm in parts[1:]:
                if not mm.isdigit():
                    continue
                trips.append((
                    current_route,
                    DEST_MAP[current_route],
                    direction,
                    base_hour * 60 + int(mm),
                ))

    trips.sort(key=lambda x: x[3])
    return trips

def write_y_csv(station: str, suffix: str, trips):
    os.makedirs(GEN_Y_DIR, exist_ok=True)
    out_path = os.path.join(GEN_Y_DIR, f"{station}_{suffix}.csv")

    with open(out_path, "w", newline="", encoding="utf8") as f:
        w = csv.writer(f)
        w.writerow(["line", "destination", "direction", "abs_time"])
        for row in trips:
            w.writerow(row)

# Main
if __name__ == "__main__":
    for fn in os.listdir(RAW_Y_DIR):
        if not fn.endswith(".txt"):
            continue

        m = re.fullmatch(r"(Y\d{2})_(\d+)\.txt", fn)
        if not m:
            continue

        station, suffix = m.group(1), m.group(2)
        path = os.path.join(RAW_Y_DIR, fn)

        trips = parse_y_txt(path)
        write_y_csv(station, suffix, trips)