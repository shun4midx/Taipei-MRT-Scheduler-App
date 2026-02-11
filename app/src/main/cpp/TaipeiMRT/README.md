# Taipei-MRT-Scheduler
The current Taipei MRT app but with route times in real-time (to the nearest minute without needing Wi-Fi) by accomodating train waiting time, and also providing more variants of MRT routes rather than just the fastest or lowest interchange ones (e.g. not passing certain stations or lines), alongside more requirements to satisfy my "MRT nerd" (捷運迷) self.

# IMPORTANT NOTICE ABOUT R01 (GUANGCI/FENGTIAN TEMPLE)
Recently, the Taipei MRT has announced it will start to do final testing with the new R01 station called "Guangci/Fengtian Temple". However, since I do not possess the ability to mindread, I have no clue how the new timetables for the stations on the Red Line/Tamsui-Xinyi Line will be affected. I will update my used CSVs when I find new information about the arrival times, but for now, please note that the arrival times are based off of the Taipei MRT before R01 was added. Thank you for your understanding.

# Supported Lines (with accuracy up until Jan 2026)
 - 🟥 Red Line/Tamsui-Xinyi Line
 - 🟧 Orange Line/Zhonghe-Xinlu Line
 - 🟨 Yellow Line/Circular Line
 - 🟩 Green Line/Songshan-Xindian Line
 - 🟦 Blue Line/Bannan Line
 - 🟫 Brown Line/Wenhu Line (*)

(*) Since the brown line isn't consistent with arrival time, I operate this code based on a rough estimate for the worst case scenario of waiting time given the current timeframe (e.g. late night vs right after work).

Once there is enough information about the Light Blue Line/Sanying Line, I plan to add it to my code!

# Credits
I used the [the Taipei MRT publicly provided Taipei MRT per-station architecture, and station map](https://www.metro.taipei/cp.aspx?n=73B51F32ED23C5E1), and also [the New Taipei Metro publicly provided timetables for the Circular Line](https://www.ntmetro.com.tw/basic/?mode=detail&node=796). I also used the [CSVs provided by the Ministry of Digital Affairs](https://data.gov.tw/en/datasets/131737) for the most recent data on train arrival times for the Taipei MRT.

I am aware there is an MRT API, but I think that costs money if we want unlimited usage, also I doubt it can give me information about the Circular Line, so...