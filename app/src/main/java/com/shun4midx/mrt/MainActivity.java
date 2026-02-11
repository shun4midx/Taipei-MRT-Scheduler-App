package com.shun4midx.mrt;
import static com.shun4midx.mrt.AgeGroup.CHILD;
import static com.shun4midx.mrt.AgeGroup.ADULT;
import static com.shun4midx.mrt.AgeGroup.ELDERLY;

import com.shun4midx.mrt.Mode;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    Spinner fromLine;
    Spinner fromStation;

    AgeGroup user_age = ADULT; // Assume to be adult

    static {
        System.loadLibrary("mrt");
    }

    Map<String, String[]> stationsByLine = new HashMap<>();

    private final Handler minuteHandler =
            new Handler(Looper.getMainLooper());

    private Runnable minuteRunnable;

    long millisUntilNextMinute() {
        long now = System.currentTimeMillis();
        return 60_000 - (now % 60_000);
    }

    void startMinuteUpdates() {
        if (minuteRunnable != null) {
            return; // prevent duplicates
        }

        stopMinuteUpdates(); // prevent duplicates

        minuteRunnable = new Runnable() {
            @Override
            public void run() {

                // Only refresh if still in NEXT_TRAIN mode
                if (currentMode == Mode.NEXT_TRAIN) {
                    updateNextTrainUI();
                }

                // After first alignment, run every exact minute
                minuteHandler.postDelayed(this, 60_000);
            }
        };

        long delay = millisUntilNextMinute();
        minuteHandler.postDelayed(minuteRunnable, delay);
    }

    void stopMinuteUpdates() {
        if (minuteRunnable != null) {
            minuteHandler.removeCallbacks(minuteRunnable);
            minuteRunnable = null;
        }
    }

    private void copyAssetFolder(String assetDir, File outDir) throws IOException {
        String[] files = getAssets().list(assetDir);
        if (files == null) return;

        if (!outDir.exists()) outDir.mkdirs();

        for (String file : files) {
            String assetPath = assetDir + "/" + file;
            File outFile = new File(outDir, file);

            String[] subFiles = getAssets().list(assetPath);
            if (subFiles != null && subFiles.length > 0) {
                // directory
                copyAssetFolder(assetPath, outFile);
            } else {
                // file
                try (InputStream in = getAssets().open(assetPath);
                     OutputStream out = new FileOutputStream(outFile)) {

                    byte[] buf = new byte[4096];
                    int len;
                    while ((len = in.read(buf)) > 0) {
                        out.write(buf, 0, len);
                    }
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        updateFooterStatement();

        ImageView mrtMap = findViewById(R.id.mrtMap);

        mrtMap.setOnClickListener(v -> {
            Intent intent = new Intent(this, MapFullscreenActivity.class);

            int mapRes;
            switch (getLanguage()) {
                case "jp": mapRes = R.drawable.taipei_mrt_map_jp; break;
                case "kr": mapRes = R.drawable.taipei_mrt_map_kr; break;
                default:   mapRes = R.drawable.taipei_mrt_map_zh_en;
            }

            intent.putExtra("mapRes", mapRes);
            startActivity(intent);
        });

        try {
            copyAssetFolder(
                    "arrival_times/generated",
                    new File(getFilesDir(), "arrival_times/generated")
            );
            setDataDir(getFilesDir().getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Clickable links
        TextView repoLink = findViewById(R.id.repoLink);
        repoLink.setText(Html.fromHtml(getString(R.string.repo_link)));
        repoLink.setMovementMethod(LinkMovementMethod.getInstance());

        // ======== SPINNER ======== //
        fromLine = findViewById(R.id.fromLine);
        fromStation = findViewById(R.id.fromStation);

        ArrayAdapter<LineItem> adapter =
                new ArrayAdapter<>(
                        this,
                        android.R.layout.simple_spinner_item,
                        getLines()
                );

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        fromLine.setAdapter(adapter);

        updateMapImage();
        setupModeButtons();

        refreshStationSpinner();

        if (currentMode == Mode.NEXT_TRAIN) {
            fromStation.post(() -> updateNextTrainUI());
        }

        // ======== LISTENER ======== //
        fromLine.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner();
            }

            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });

        fromStation.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                if (currentMode == Mode.NEXT_TRAIN) {
                    fromStation.post(() -> updateNextTrainUI());
                }
            }

            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.menu_language) {
            showLanguageDialog();
            return true;
        } else if (item.getItemId() == R.id.menu_age_group) {
            showAgeGroupDialog();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    void showLanguageDialog() {
        final String[] langs = {"正體中文", "English", "日本語", "한국어"};

        new AlertDialog.Builder(this)
                .setItems(langs, (dialog, which) -> {
                    switch (which) {
                        case 0: setLanguage("zh"); break;
                        case 1: setLanguage("en"); break;
                        case 2: setLanguage("jp"); break;
                        case 3: setLanguage("kr"); break;
                    }
                })
                .show();
    }

    String[] getAgeGroupLabels() {
        switch (getLanguage()) {
            case "zh":
                return new String[]{"兒童", "成人", "敬老"};
            case "en":
                return new String[]{"Child", "Adult", "Elderly"};
            case "jp":
                return new String[]{"子供", "大人", "高齢者"};
            case "kr":
                return new String[]{"어린이", "성인", "노인"};
            default:
                return new String[]{"兒童", "成人", "敬老"};
        }
    }

    void showAgeGroupDialog(){
        final String[] ages = getAgeGroupLabels();

        new AlertDialog.Builder(this)
                .setItems(ages, (dialog, which) -> {
                    switch (which) {
                        case 0: setAge("CHILD"); break;
                        case 1: setAge("ADULT"); break;
                        case 2: setAge("ELDERLY"); break;
                    }
                })
                .show();
    }

    String getLanguage() {
        return getSharedPreferences("settings", MODE_PRIVATE)
                .getString("lang", "zh");
    }

    int getLanguageInt() {
        switch (getLanguage()) {
            case "en": return 1;
            case "jp": return 2;
            case "kr": return 3;
            case "zh":
            default:   return 0;
        }
    }

    void refreshStationSpinner() {
        if (fromLine == null || fromStation == null) {
            return;
        }
        if (fromLine.getSelectedItem() == null) {
            return;
        }

        LineItem item = (LineItem) fromLine.getSelectedItem();

        String[] stations =
                getStationsDisplayList(item.code, getLanguageInt());

        ArrayAdapter<String> stationAdapter =
                new ArrayAdapter<>(
                        this,
                        R.layout.spinner_item_station,
                        stations
                );

        stationAdapter.setDropDownViewResource(
                R.layout.spinner_item_station
        );

        fromStation.setAdapter(stationAdapter);
    }

    void updateMapImage() {
        ImageView map = findViewById(R.id.mrtMap);

        switch (getLanguage()) {
            case "zh":
                map.setImageResource(R.drawable.taipei_mrt_map_zh_en);
                break;
            case "en":
                map.setImageResource(R.drawable.taipei_mrt_map_zh_en);
                break;
            case "jp":
                map.setImageResource(R.drawable.taipei_mrt_map_jp);
                break;
            case "kr":
                map.setImageResource(R.drawable.taipei_mrt_map_kr);
                break;
        }
    }

    void recomputeCurrentResult() {
        switch (currentMode) {
            case ROUTE_PLANNER:
//                recomputeRoutePlanner();
                break;
            case CUSTOM_PATH:
//                recomputeCustomPath();
                break;
            case TRAIN_COST:
//                recomputeFareOnly();
                break;
            case NEXT_TRAIN:
                // age probably irrelevant here
                break;
        }
    }

    void refreshCostUI() {
        if (currentMode == Mode.NEXT_TRAIN) {
            return;
        }

        if (fromLine.getSelectedItem() == null || fromStation.getSelectedItem() == null) {
            return;
        }

        LineItem line = (LineItem) fromLine.getSelectedItem();
        int stationIndex = fromStation.getSelectedItemPosition();

        recomputeCurrentResult();
    }

    void applyLocale(String lang) {
        Locale locale = new Locale(lang);
        Locale.setDefault(locale);

        Configuration config = new Configuration(getResources().getConfiguration());
        config.setLocale(locale);

        getResources().updateConfiguration(
                config,
                getResources().getDisplayMetrics()
        );
    }

    void setLanguage(String lang) {
        getSharedPreferences("settings", MODE_PRIVATE).edit().putString("lang", lang).apply();

        String language_set = "";

        switch (getLanguage()) {
            case "zh":
                language_set = "語言成功設為正體中文";
                break;
            case "en":
                language_set = "Language successfully set to English";
                break;
            case "jp":
                language_set = "言語が日本語に設定されました";
                break;
            case "kr":
                language_set = "언어가 한국어로 성공적으로 설정되었습니다";
                break;
        }

        android.widget.Toast.makeText(this, language_set, android.widget.Toast.LENGTH_SHORT).show();

        updateMapImage();
        refreshStationSpinner();
        setupModeButtons();

        getSharedPreferences("settings", MODE_PRIVATE).edit().putString("lang", lang).apply();
        applyLocale(lang);
        updateFooterStatement();
        updateMapImage();
        refreshStationSpinner();

        if (currentMode == Mode.NEXT_TRAIN) {
            updateNextTrainUI();
        }
    }

    void setAge(String age) {
        getSharedPreferences("settings", MODE_PRIVATE).edit().putString("age_group", age).apply();
        refreshCostUI();
    }

    String getModeLabel(Mode mode) {
        switch (getLanguage()) {
            case "zh":
                switch (mode) {
                    case NEXT_TRAIN:   return "下一班車";
                    case ROUTE_PLANNER:return "路線規劃";
                    case CUSTOM_PATH:  return "自訂路線";
                    case TRAIN_COST:   return "票價";
                }
            case "en":
                switch (mode) {
                    case NEXT_TRAIN:   return "Next Train";
                    case ROUTE_PLANNER:return "Route Planner";
                    case CUSTOM_PATH:  return "Custom Path";
                    case TRAIN_COST:   return "Train Fares";
                }
            case "jp":
                switch (mode) {
                    case NEXT_TRAIN:   return "次の電車";
                    case ROUTE_PLANNER:return "経路検索";
                    case CUSTOM_PATH:  return "カスタム経路";
                    case TRAIN_COST:   return "運賃";
                }
            case "kr":
                switch (mode) {
                    case NEXT_TRAIN:   return "다음 열차";
                    case ROUTE_PLANNER:return "경로 찾기";
                    case CUSTOM_PATH:  return "사용자 경로";
                    case TRAIN_COST:   return "요금";
                }
        }
        return "";
    }

    void updateFooterStatement() {
        TextView footer = findViewById(R.id.footerStatement);
        if (footer == null) return;

        switch (getLanguage()) {
            case "kr":
                footer.setText(
                        "⚠️위에 표시된 원후선(갈색선) 열차 도착 시간은 최악의 상황을 가정하여 계산된 것이며 실제 운행 상황을 반영하지 않습니다."
                );
                break;
            case "jp":
                footer.setText(
                        "⚠️上記の文湖線（茶色の線）の列車の到着時間は最悪の状況下で計算されており、実際の路線状況を反映するものではありません。"
                );
                break;
            case "en":
                footer.setText(
                        "⚠️ The train arrival times for the brown line stations are worst-case estimates and do not reflect real conditions."
                );
                break;
            default: // zh
                footer.setText(
                        "⚠️以上顯示文湖線的列車到達時間，皆為最壞情況估計，並非即時路況。"
                );
        }
    }

    void updateModeUI() {
        switch (currentMode) {
            case NEXT_TRAIN:
                // show arrival times
                break;
            case ROUTE_PLANNER:
                // show spinners + route results
                break;
            case CUSTOM_PATH:
                // show path builder UI
                break;
            case TRAIN_COST:
                // show spinners and cost results
                break;
        }
    }

    LinearLayout modeContainer;
    Mode currentMode = Mode.NEXT_TRAIN;

    void setupModeButtons() {
        modeContainer = findViewById(R.id.modeContainer);
        modeContainer.removeAllViews();

        for (Mode mode : Mode.values()) {
            android.widget.Button btn = new Button(this, null, 0, R.style.ModeButton);

            btn.setText(getModeLabel(mode));
            btn.setTypeface(btn.getTypeface(), android.graphics.Typeface.BOLD);
            btn.setGravity(Gravity.CENTER);
            btn.setMinHeight(0);
            btn.setIncludeFontPadding(false);
            btn.setBackgroundResource(R.drawable.mode_button_selector);
            btn.setPadding(25, 20, 25, 20);

            btn.setOnClickListener(v -> {
                currentMode = mode;
                updateModeUI();
                if (mode == Mode.NEXT_TRAIN) {
                    updateNextTrainUI();     // immediate refresh
                    startMinuteUpdates();    // then aligned refresh
                } else {
                    stopMinuteUpdates();
                }
            });

            LinearLayout.LayoutParams lp =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                    );

            // spacing BETWEEN buttons
            lp.setMargins(10, 0, 10, 0);

            btn.setLayoutParams(lp);

            modeContainer.addView(btn);
        }
    }

    void updateNextTrainUI() {
//        android.widget.Toast.makeText(this, "Updating table", Toast.LENGTH_SHORT).show();
        LinearLayout table = findViewById(R.id.nextTrainTable);
        table.removeAllViews();

        // immediate placeholder
        TextView loading = new TextView(this);
        loading.setText(
                getLanguage().equals("en") ? "Updating..." :
                        getLanguage().equals("jp") ? "更新中..." :
                                getLanguage().equals("kr") ? "업데이트 중..." :
                                        "更新中..."
        );

        if (fromLine.getSelectedItem() == null ||
                fromStation.getSelectedItem() == null) {
            return;
        }

        LineItem line = (LineItem) fromLine.getSelectedItem();
        String item = (String) fromStation.getSelectedItem();
        if (item.equals("––")) return;

        String code = item.substring(line.code.length(), line.code.length() + 2); // stn num
        int station = Integer.parseInt(code);

        String[][] grid = getNextTrainTable(line.code, station, 4, 5, getLanguageInt());

        for (String[] row : grid) {
            LinearLayout rowView = new LinearLayout(this);
            rowView.setOrientation(LinearLayout.HORIZONTAL);

            for (int j = 0; j < row.length; j++) {
                TextView tv = new TextView(this);
                tv.setText(row[j]);
                tv.setTextSize(16);
                tv.setTextColor(getColor(R.color.custom_pink));
                tv.setPadding(12, 10, 12, 10);

                if (j == 0) {
                    // first column: left aligned
                    tv.setGravity(Gravity.START);
                } else {
                    // other columns: right aligned
                    tv.setGravity(Gravity.END);
                }

                LinearLayout.LayoutParams lp =
                        new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT);

                lp.weight = (j == 0) ? (float)1.25 : 1;
                tv.setLayoutParams(lp);

                rowView.addView(tv);
            }

            table.addView(rowView);
        }
    }


    View makeRow(String[] times) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_HORIZONTAL);

        for (String t : times) {
            TextView tv = new TextView(this);
            tv.setText(t);
            tv.setPadding(16, 8, 16, 8);
            tv.setTextSize(14);
            row.addView(tv);
        }

        return row;
    }

    @Override
    protected void onPause() {
        super.onPause();
        stopMinuteUpdates();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (currentMode == Mode.NEXT_TRAIN) {
            startMinuteUpdates();
        }
    }

    public native void setDataDir(String path);
    public native String[] getStationsDisplayList(String line_code, int lang);

    public native LineItem[] getLines();

    public native String[][] getNextTrainTable(String line_code, int station, int maxRows, int maxCols, int lang);
}