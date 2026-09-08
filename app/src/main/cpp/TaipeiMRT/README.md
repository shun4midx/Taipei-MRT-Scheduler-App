# Taipei-MRT-Scheduler
The current Taipei MRT app but with route times in real-time (to the nearest minute without needing Wi-Fi) by accommodating train waiting time, and also providing more variants of MRT routes rather than just the fastest or lowest transfer count ones (e.g. not passing certain stations or lines), alongside more requirements to satisfy my "MRT nerd" (捷運迷) self.

# Supported Lines
 - 🟥 Red Line/Tamsui-Xinyi Line (!)
 - 🟧 Orange Line/Zhonghe-Xinlu Line
 - 🟨 Yellow Line/Circular Line
 - 🟩 Green Line/Songshan-Xindian Line
 - 🟦 Blue Line/Bannan Line
 - [IN PROGRESS] 💠 Light Blue Line/Sanying line (#)
 - 🟫 Brown Line/Wenhu Line (*)

(*) Since the brown line isn't consistent with arrival times, I operate this code based on a rough estimate for the worst-case scenario of waiting time given the current timeframe (e.g. late night vs right after work).

(!) The Guangci/Fengtian Temple station was added to the Red Line officially on August 31st, 2026, and for the first four weeks, all rides between Xiangshan and Guangci/Fengtian Temple are counted for free. In other words, no official fare table has been released yet for travel to or from Guangci/Fengtian Temple after these four weeks. Thus, this code operates under the assumption that these rides are still counted for free.

(#) The Sanying Line was added in June, but there is limited official data on its arrival times. Thus, I estimate waiting times using a worst-case scenario given the current timeframe like the brown line.

# Credits
I used the [the Taipei MRT publicly available station layouts and network map](https://www.metro.taipei/cp.aspx?n=73B51F32ED23C5E1), and also [the New Taipei Metro publicly provided timetables for the Circular Line](https://www.ntmetro.com.tw/basic/?mode=detail&node=796). I also used the [CSVs provided by the Ministry of Digital Affairs](https://data.gov.tw/en/datasets/131737) for the most recent data on train arrival times for the Taipei MRT.

I am aware there is an MRT API, but I think that costs money if we want unlimited usage, also I doubt it can give me information about the Circular Line, so...