############################################
# Copyright (c) 2026 Shun/修海 (@shun4midx) #
# Project: Taipei-MRT-Scheduler            #
# File Type: Python file                   #
# File: change_r02_to_r01.py               #
############################################

from pathlib import Path

folder = Path("generated/R")

for path in folder.glob("R*.csv"):
    # R03_12345.csv -> station_code = "R03"
    station_code = path.stem.split("_")[0]

    if not station_code.startswith("R"):
        continue

    # Skip non-numeric station suffixes like R22A
    if not station_code[1:].isdigit():
        continue

    stn_num = int(station_code[1:])

    # Only modify R03 through R28
    if not (3 <= stn_num <= 28):
        continue

    text = path.read_text(encoding="utf-8")

    old_count = text.count(",R02,1,")
    if old_count == 0:
        continue

    text = text.replace(",R02,1,", ",R01,1,")

    path.write_text(text, encoding="utf-8")

    print(f"{path}: replaced {old_count} entries")