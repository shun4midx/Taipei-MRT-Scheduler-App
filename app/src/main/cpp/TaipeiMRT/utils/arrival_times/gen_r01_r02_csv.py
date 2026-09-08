############################################
# Copyright (c) 2026 Shun/修海 (@shun4midx) #
# Project: Taipei-MRT-Scheduler            #
# File Type: Python file                   #
# File: gen_r01_r02_csv.py                 #
############################################

from pathlib import Path
import csv

# Run this script from arrival_times/
RAW_DIR = Path("raw/R")
GENERATED_DIR = Path("generated/R")

DAYS = ["12345", "6", "7"]

def parse_hour_line(line, path):
    hour_str, mins_str = line.split(":", 1)

    try:
        hour = int(hour_str)
    except ValueError:
        raise ValueError(f"{path}: invalid hour in line: {line}")

    # Allow empty hour row
    if not mins_str:
        return []

    try:
        mins = [int(x) for x in mins_str.split(",")]
    except ValueError:
        raise ValueError(f"{path}: invalid minute in line: {line}")

    # Validate minute range
    for minute in mins:
        if minute < 0 or minute > 59:
            raise ValueError(f"{path}: invalid minute {minute:02d} in line: {line}")

    # Don't silently sort typoed hand-entered data
    if mins != sorted(mins):
        raise ValueError(f"{path}: minutes aren't sorted in line: {line}")

    # Catch accidental duplicate times in one hour
    if len(mins) != len(set(mins)):
        raise ValueError(f"{path}: duplicate minute in line: {line}")

    return [hour * 60 + minute for minute in mins]


def parse_raw(path):
    sections = {}
    current_section = None

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()

        if not line:
            continue

        # Anything without ":" is a section header
        if ":" not in line:
            if line in sections:
                raise ValueError(f"{path}: duplicate section header: {line}")

            current_section = line
            sections[current_section] = []
            continue

        if current_section is None:
            raise ValueError(f"{path}: timetable row appears before a section header: {line}")

        sections[current_section].extend(parse_hour_line(line, path))

    return sections

def write_csv(path, rows):
    # Final sanity check
    rows.sort(key=lambda row: row[3])

    path.parent.mkdir(parents=True, exist_ok=True)

    # Write to temporary file first so an error doesn't destroy an existing generated timetable
    temp_path = path.with_suffix(".csv.tmp")

    with temp_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["line", "destination", "direction", "time"])
        writer.writerows(rows)

    temp_path.replace(path)

def generate_r01(day):
    raw_path = RAW_DIR / f"R01_{day}.txt"
    out_path = GENERATED_DIR / f"R01_{day}.csv"

    sections = parse_raw(raw_path)
    expected = {"R28,R-1", "R22,R-1", "R-2"}

    unknown = set(sections) - expected
    if unknown:
        raise ValueError(f"{raw_path}: unknown section(s): {sorted(unknown)}")

    # R-2 should exist but be empty for R01
    if sections.get("R-2", []):
        raise ValueError(f"{raw_path}: R01 is the southern terminus; R-2 section should be empty")

    rows = []

    # Tamsui trains
    for t in sections.get("R28,R-1", []):
        rows.append(["R-1", "R28", 0, t])

    # Beitou shuttle trains
    for t in sections.get("R22,R-1", []):
        rows.append(["R-1", "R22", 0, t])

    write_csv(out_path, rows)

    print(
        f"{out_path}: generated {len(rows)} trains ({len(sections.get('R28,R-1', []))} R28, {len(sections.get('R22,R-1', []))} R22)")

def rebuild_r02(day):
    raw_path = RAW_DIR / f"R02_{day}.txt"
    csv_path = GENERATED_DIR / f"R02_{day}.csv"

    sections = parse_raw(raw_path)

    expected = {"R-1", "R-2"}

    unknown = set(sections) - expected
    if unknown:
        raise ValueError(f"{raw_path}: unknown section(s): {sorted(unknown)}")

    # Stop duplicating existing R-1 data
    if sections.get("R-1", []):
        raise ValueError(f"{raw_path}: R02 R-1 section should be empty; northbound trains are preserved from the generated CSV")

    if not csv_path.exists():
        raise FileNotFoundError(f"{csv_path} doesn't exist; R02 needs its old generated CSV so R-1 trains can be preserved")

    kept_northbound = []

    # Preserve existing direction 0 data
    with csv_path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)

        required_columns = {"line", "destination", "direction", "time"}

        if reader.fieldnames is None or not required_columns.issubset(reader.fieldnames):
            raise ValueError(f"{csv_path}: unexpected CSV header: {reader.fieldnames}")

        for row in reader:
            if row["direction"] == "0":
                kept_northbound.append([row["line"], row["destination"], int(row["direction"]), int(row["time"])])

    # Generate brand-new R01-bound trains
    new_southbound = []

    for t in sections.get("R-2", []):
        new_southbound.append(["R-2", "R01", 1, t])

    rows = kept_northbound + new_southbound
    write_csv(csv_path, rows)

    print(f"{csv_path}: kept {len(kept_northbound)} northbound trains, generated {len(new_southbound)} R01-bound trains")


def main():
    print("======== GENERATING R01 ========")

    for day in DAYS:
        generate_r01(day)

    print()
    print("======== REBUILDING R02 ========")

    for day in DAYS:
        rebuild_r02(day)

    print()
    print("Done.")


if __name__ == "__main__":
    main()