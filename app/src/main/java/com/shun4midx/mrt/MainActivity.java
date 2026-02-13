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
import android.graphics.Typeface;
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
import android.widget.CheckBox;
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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

enum RouteStrategy {
    FASTEST,
    LEAST_INTERCHANGE,
    CUSTOM
}

public class MainActivity extends AppCompatActivity {
    Spinner fromLine;
    Spinner fromStation;

    AgeGroup user_age = ADULT; // Assume to be adult

    LinearLayout nextTrainControls;
    LinearLayout trainCostControls;

    static {
        System.loadLibrary("mrt");
    }

    Map<String, String[]> stationsByLine = new HashMap<>();

    // ===== ROUTE PLANNER UI =====
    LinearLayout routePlannerControls;
    TextView routeStartLabel, routeEndLabel;
    Spinner routeFromLine, routeFromStation;
    Spinner routeToLine, routeToStation;
    LinearLayout customControls;
    RouteStrategy currentStrategy = null;
    LinearLayout routeStrategyContainer;

    TextView customConstraintsLabel;
    TextView mustStationsTitle, avoidStationsTitle, avoidLinesTitle, mustLinesTitle;
    Button addMustStationBtn, addAvoidStationBtn, applyCustomBtn;
    TextView rankingPreferenceTitle;
    CheckBox minTimeCheck, minTransferCheck;

    static class StationRow {
        Spinner lineSpinner;
        Spinner stationSpinner;
        View rootView;  // the whole row layout
    }

    List<StationRow> mustStationRows = new ArrayList<>();
    LinearLayout mustLinesContainer;
    List<CheckBox> mustLineChecks = new ArrayList<>();
    List<StationRow> avoidStationRows = new ArrayList<>();
    LinearLayout avoidLinesContainer;
    List<CheckBox> avoidLineChecks = new ArrayList<>();

    // ===== CUSTOM PATH UI =====
    LinearLayout manualPathControls;
    List<StationRow> customPathRows = new ArrayList<>();

    // ===== TRAIN_COST UI =====
    TextView costStartLabel, costEndLabel;
    Spinner costFromLine, costFromStation;
    Spinner costToLine, costToStation;
    LinearLayout costTable;
    TextView costIdentityHint;

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

        // ======== CLEAR SCREEN ======== //
        nextTrainControls = findViewById(R.id.nextTrainControls);
        nextTrainControls.setVisibility(View.GONE);
        manualPathControls = findViewById(R.id.manualPathControls);
        manualPathControls.setVisibility(View.GONE);
        trainCostControls = findViewById(R.id.trainCostControls);
        trainCostControls.setVisibility(View.GONE);

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

        setupTrainCostUI(adapter);
        setupRoutePlannerUI(adapter);
        setupManualPathUI();
        updateMapImage();
        setupModeButtons();

        refreshStationSpinner();

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

        String savedAge = getSharedPreferences("settings", MODE_PRIVATE)
                .getString("age_group", "ADULT");

        switch (savedAge) {
            case "CHILD": user_age = CHILD; break;
            case "ELDERLY": user_age = ELDERLY; break;
            default: user_age = ADULT;
        }

        updateCostLabels();
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

        if (currentMode == Mode.NEXT_TRAIN) {
            updateNextTrainUI();
        }

        updateRoutePlannerLabels();
        updateCustomLabels();
        setupRouteStrategyButtons();
        updateRouteStrategyUI();
        updateCostLabels();
        updateManualLabels();
        refreshStationSpinner(costFromLine, costFromStation);
        refreshStationSpinner(costToLine, costToStation);
        for (StationRow row : mustStationRows) {
            refreshStationSpinner(row.lineSpinner, row.stationSpinner);
        }
        for (StationRow row : avoidStationRows) {
            refreshStationSpinner(row.lineSpinner, row.stationSpinner);
        }

        if (currentMode == Mode.TRAIN_COST) {
            updateCostUI();
        }
        resetRoutePlannerState();
        resetManualPathState();
    }

    void clearRouteResult() {
        LinearLayout container = findViewById(R.id.routeResultContainer);
        if (container != null) {
            container.removeAllViews();
        }
    }

    void resetRoutePlannerState() {

        // Clear selected strategy
        currentStrategy = null;
        updateRouteStrategyUI();

        // Reset spinners to first item
        if (routeFromLine != null) {
            routeFromLine.setSelection(0);
        }
        if (routeToLine != null) {
            routeToLine.setSelection(0);
        }

        refreshStationSpinner(routeFromLine, routeFromStation);
        refreshStationSpinner(routeToLine, routeToStation);

        if (currentMode == Mode.ROUTE_PLANNER) {
            recomputeRoutePlanner();
        }

        if (routeFromStation != null) {
            routeFromStation.setSelection(0);
        }
        if (routeToStation != null) {
            routeToStation.setSelection(0);
        }

        // Clear result container
        clearRouteResult();
    }

    void resetManualPathState() {

        for (StationRow row : customPathRows) {

            if (row.lineSpinner != null) {
                row.lineSpinner.setSelection(0);
            }

            if (row.stationSpinner != null) {
                refreshStationSpinner(row.lineSpinner, row.stationSpinner);
                row.stationSpinner.setSelection(0);
            }
        }

        displayManualResult("");
    }

    void setAge(String age) {
        getSharedPreferences("settings", MODE_PRIVATE).edit().putString("age_group", age).apply();

        // Update in-memory enum
        switch (age) {
            case "CHILD": user_age = CHILD; break;
            case "ADULT": user_age = ADULT; break;
            case "ELDERLY": user_age = ELDERLY; break;
        }

        if (currentMode == Mode.ROUTE_PLANNER) {
            recomputeRoutePlanner();
        } else if (currentMode == Mode.TRAIN_COST) {
            updateCostUI();
        } else if (currentMode == Mode.CUSTOM_PATH) {
            recomputeManualPath();
        }

        updateCostLabels();
    }

    void recomputeRoutePlanner() {

        if (currentStrategy == null) {
            clearRouteResult();
            return;
        }

        if (currentMode != Mode.ROUTE_PLANNER) {
            return;
        }

        LineItem fromL = (LineItem) routeFromLine.getSelectedItem();
        LineItem toL   = (LineItem) routeToLine.getSelectedItem();

        int fromSt = parseStationNo(fromL, routeFromStation);
        int toSt   = parseStationNo(toL, routeToStation);

        if (fromSt < 0 || toSt < 0) {
            clearRouteResult();
            return;
        }

        String lang = getLanguage();
        if (lang.equals("en")) {
            displayRouteResult("Computing...");
        } else if (lang.equals("zh")) {
            displayRouteResult("計算中...");
        } else if (lang.equals("jp")) {
            displayRouteResult("計算中...");
        } else if (lang.equals("kr")) {
            displayRouteResult("계산 중...");
        }

        new Thread(() -> {

            String result;

            if (currentStrategy == RouteStrategy.FASTEST) {
                result = computeFastestRoute(fromL.code, fromSt, toL.code, toSt, getLanguageInt(), user_age.ordinal());
            } else if (currentStrategy == RouteStrategy.LEAST_INTERCHANGE) {
                result = computeLeastInterchangeRoute(fromL.code, fromSt, toL.code, toSt, getLanguageInt(), user_age.ordinal());
            } else {
                clearRouteResult();
                return;
            }

            runOnUiThread(() -> {
                displayRouteResult(result);
            });

        }).start();
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
        TextView footer = findViewById(R.id.footerStatement);

        switch (currentMode) {
            case NEXT_TRAIN:
                nextTrainControls.setVisibility(View.VISIBLE);
                routePlannerControls.setVisibility(View.GONE);
                manualPathControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.GONE);
                if (footer != null) {
                    footer.setVisibility(View.VISIBLE);
                }
                break;

            case ROUTE_PLANNER:
                nextTrainControls.setVisibility(View.GONE);
                routePlannerControls.setVisibility(View.VISIBLE);
                manualPathControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.GONE);
                if (footer != null) {
                    footer.setVisibility(View.GONE);
                }
                break;

            case CUSTOM_PATH:
                nextTrainControls.setVisibility(View.GONE);
                routePlannerControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.GONE);
                manualPathControls.setVisibility(View.VISIBLE);
                if (footer != null) {
                    footer.setVisibility(View.GONE);
                }
                break;

            case TRAIN_COST:
                nextTrainControls.setVisibility(View.GONE);
                routePlannerControls.setVisibility(View.GONE);
                manualPathControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.VISIBLE);
                if (footer != null) {
                    footer.setVisibility(View.GONE);
                }
                break;

            default: // normal case should be to show
                nextTrainControls.setVisibility(View.GONE);
                routePlannerControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.GONE);
                if (footer != null) {
                    footer.setVisibility(View.VISIBLE);
                }
        }
    }


    LinearLayout modeContainer;
    Mode currentMode = null;

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
                if (currentMode == mode) {
                    // Clicking same mode again → deselect
                    currentMode = null;
                    stopMinuteUpdates();
                    nextTrainControls.setVisibility(View.GONE);
                    routePlannerControls.setVisibility(View.GONE);
                    manualPathControls.setVisibility(View.GONE);
                    trainCostControls.setVisibility(View.GONE);
                    TextView footer = findViewById(R.id.footerStatement);
                    footer.setVisibility(View.VISIBLE);
                    return;
                }

                currentMode = mode;
                updateModeUI();
                nextTrainControls.setVisibility(View.GONE);
                trainCostControls.setVisibility(View.GONE);
                if (mode == Mode.NEXT_TRAIN) {
                    nextTrainControls.setVisibility(View.VISIBLE);
                    updateNextTrainUI();     // immediate refresh
                    startMinuteUpdates();    // then aligned refresh
                } else if (mode == Mode.TRAIN_COST) {
                    trainCostControls.setVisibility(View.VISIBLE);
                    updateCostLabels();
                    updateCostUI();
                    stopMinuteUpdates();
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

    void setupManualPathUI() {

        Button addBtn = findViewById(R.id.addCustomStationBtn);
        Button applyBtn = findViewById(R.id.applyManualPathBtn);

        addBtn.setText(getAddStationLabel());
        applyBtn.setText(getApplyCustomLabel());

        addBtn.setOnClickListener(v -> addCustomPathRow());
        applyBtn.setOnClickListener(v -> applyManualPath());

        // Auto add two rows
        addCustomPathRow();
        addCustomPathRow();
    }

    void applyManualPath() {

        List<String> stations = getCustomPathStations();

        if (stations.size() < 2) {
            return;
        }

        String lang = getLanguage();
        if (lang.equals("en")) {
            displayManualResult("Computing...");
        } else if (lang.equals("zh")) {
            displayManualResult("計算中...");
        } else if (lang.equals("jp")) {
            displayManualResult("計算中...");
        } else if (lang.equals("kr")) {
            displayManualResult("계산 중...");
        }

        new Thread(() -> {

            String result = computeManualPath(
                    stations.toArray(new String[0]),
                    getLanguageInt(),
                    user_age.ordinal()
            );

            runOnUiThread(() -> {
                displayManualResult(result);
            });

        }).start();
    }

    void recomputeManualPath() {

        if (currentMode != Mode.CUSTOM_PATH) return;

        List<String> stations = getCustomPathStations();

        if (stations.size() < 2) {
            displayManualResult("");
            return;
        }

        String lang = getLanguage();
        if (lang.equals("en")) {
            displayManualResult("Computing...");
        } else if (lang.equals("zh")) {
            displayManualResult("計算中...");
        } else if (lang.equals("jp")) {
            displayManualResult("計算中...");
        } else if (lang.equals("kr")) {
            displayManualResult("계산 중...");
        }

        new Thread(() -> {

            String result = computeManualPath(
                    stations.toArray(new String[0]),
                    getLanguageInt(),
                    user_age.ordinal()
            );

            runOnUiThread(() -> displayManualResult(result));

        }).start();
    }

    List<String> getCustomPathStations() {

        List<String> result = new ArrayList<>();

        for (StationRow row : customPathRows) {

            LineItem lineItem =
                    (LineItem) row.lineSpinner.getSelectedItem();

            int stationNo =
                    parseStationNo(lineItem, row.stationSpinner);

            if (lineItem != null && stationNo >= 0) {
                result.add(lineItem.code + stationNo);
            }
        }

        return result;
    }

    void addCustomPathRow() {

        LinearLayout container = findViewById(R.id.customPathContainer);

        LinearLayout rowLayout = new LinearLayout(this);
        rowLayout.setOrientation(LinearLayout.HORIZONTAL);
        rowLayout.setPadding(0, 0, 0, 0);

        LinearLayout.LayoutParams rowParams =
                new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                );

        rowParams.setMargins(0, 0, 0, 0);
        rowLayout.setLayoutParams(rowParams);

        Spinner lineSpinner = new Spinner(this);
        Spinner stationSpinner = new Spinner(this);
        Button removeBtn = new Button(this);

        removeBtn.setText("🗑");
        removeBtn.setMinHeight(10);
        removeBtn.setMinimumHeight(10);
        removeBtn.setMinWidth(10);
        removeBtn.setMinimumWidth(10);

        // Layout weights
        lineSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        stationSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 3));
        removeBtn.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        rowLayout.addView(lineSpinner);
        rowLayout.addView(stationSpinner);
        rowLayout.addView(removeBtn);

        container.addView(rowLayout);

        ArrayAdapter<LineItem> adapter =
                new ArrayAdapter<>(this,
                        android.R.layout.simple_spinner_item,
                        getLines());

        adapter.setDropDownViewResource(
                android.R.layout.simple_spinner_dropdown_item);

        lineSpinner.setAdapter(adapter);

        refreshStationSpinner(lineSpinner, stationSpinner);

        lineSpinner.setOnItemSelectedListener(
                new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(
                            AdapterView<?> parent, View view, int pos, long id) {
                        refreshStationSpinner(lineSpinner, stationSpinner);
                    }

                    @Override public void onNothingSelected(AdapterView<?> parent) {}
                });

        StationRow row = new StationRow();
        row.lineSpinner = lineSpinner;
        row.stationSpinner = stationSpinner;
        row.rootView = rowLayout;

        customPathRows.add(row);

        removeBtn.setOnClickListener(v -> {
            container.removeView(rowLayout);
            customPathRows.remove(row);
        });
    }


    String getStartLabel() {
        switch (getLanguage()) {
            case "zh": return "起點";
            case "en": return "Start";
            case "jp": return "出発";
            case "kr": return "출발";
            default:   return "起點";
        }
    }

    String getEndLabel() {
        switch (getLanguage()) {
            case "zh": return "終點";
            case "en": return "Destination";
            case "jp": return "到着";
            case "kr": return "도착";
            default:   return "終點";
        }
    }

    String[] getFareRowLabels() {
        switch (getLanguage()) {
            case "zh": return new String[]{"成人", "兒童", "敬老"};
            case "en": return new String[]{"Adult", "Child", "Elderly"};
            case "jp": return new String[]{"大人", "子供", "高齢者"};
            case "kr": return new String[]{"성인", "어린이", "노인"};
            default:   return new String[]{"成人", "兒童", "敬老"};
        }
    }

    String getIdentityHint() {
        String ageLabel;
        switch (getLanguage()) {
            case "zh":
                ageLabel = (user_age == ADULT) ? "成人" : (user_age == CHILD) ? "兒童" : "敬老";
                return "＊您目前是「" + ageLabel + "」，請點擊右上角地球旁的圖示以更改身份。";
            case "en":
                ageLabel = (user_age == ADULT) ? "Adult" : (user_age == CHILD) ? "Child" : "Elderly";
                return "*Right now you are set as " + ageLabel + ". Click the icon next to the globe to change your identity.";
            case "jp":
                ageLabel = (user_age == ADULT) ? "大人" : (user_age == CHILD) ? "子供" : "高齢者";
                return "＊現在は「" + ageLabel + "」です。右上の地球の隣のアイコンから変更できます。";
            case "kr":
                ageLabel = (user_age == ADULT) ? "성인" : (user_age == CHILD) ? "어린이" : "노인";
                return "*현재는 " + ageLabel + "입니다. 오른쪽 위 지구 옆 아이콘에서 변경하세요.";
            default:
                ageLabel = (user_age == ADULT) ? "成人" : (user_age == CHILD) ? "兒童" : "敬老";
                return "＊您目前是「" + ageLabel + "」，請點擊右上角地球旁的圖示以更改身份。";
        }
    }

    void updateCostLabels() {
        if (costStartLabel != null) {
            costStartLabel.setText(getStartLabel());
        }

        if (costEndLabel != null) {
            costEndLabel.setText(getEndLabel());
        }

        if (costIdentityHint != null) {
            costIdentityHint.setText(getIdentityHint());
        }
    }

    void updateManualLabels() {

        Button addBtn = findViewById(R.id.addCustomStationBtn);
        Button applyBtn = findViewById(R.id.applyManualPathBtn);

        if (addBtn != null) {
            addBtn.setText(getAddStationLabel());
        }

        if (applyBtn != null) {
            applyBtn.setText(getApplyCustomLabel());
        }
    }

    void refreshStationSpinner(Spinner lineSpinner, Spinner stationSpinner) {
        if (lineSpinner == null || stationSpinner == null) return;
        if (lineSpinner.getSelectedItem() == null) return;

        LineItem item = (LineItem) lineSpinner.getSelectedItem();

        String[] stations = getStationsDisplayList(item.code, getLanguageInt());

        ArrayAdapter<String> stationAdapter =
                new ArrayAdapter<>(this, R.layout.spinner_item_station, stations);

        stationAdapter.setDropDownViewResource(R.layout.spinner_item_station);
        stationSpinner.setAdapter(stationAdapter);
    }

    int parseStationNo(LineItem lineItem, Spinner stationSpinner) {
        if (lineItem == null || stationSpinner == null) return -1;
        Object obj = stationSpinner.getSelectedItem();
        if (obj == null) return -1;

        String item = (String) obj;
        if (item.equals("––")) return -1;

        // same logic you already use in NEXT_TRAIN
        String code = item.substring(lineItem.code.length(), lineItem.code.length() + 2);
        return Integer.parseInt(code);
    }

    void setupTrainCostUI(ArrayAdapter<LineItem> adapter) {
        costStartLabel = findViewById(R.id.costStartLabel);
        costEndLabel = findViewById(R.id.costEndLabel);

        costFromLine = findViewById(R.id.costFromLine);
        costFromStation = findViewById(R.id.costFromStation);
        costToLine = findViewById(R.id.costToLine);
        costToStation = findViewById(R.id.costToStation);

        costTable = findViewById(R.id.costTable);
        costIdentityHint = findViewById(R.id.costIdentityHint);

        costFromLine.setAdapter(adapter);
        costToLine.setAdapter(adapter);

        updateCostLabels();

        // init station lists
        refreshStationSpinner(costFromLine, costFromStation);
        refreshStationSpinner(costToLine, costToStation);

        costFromLine.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(costFromLine, costFromStation);
                if (currentMode == Mode.TRAIN_COST) {
                    updateCostUI();
                }
            }
            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });

        costToLine.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(costToLine, costToStation);
                if (currentMode == Mode.TRAIN_COST) {
                    updateCostUI();
                }
            }
            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });

        AdapterView.OnItemSelectedListener stationListener = new AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                if (currentMode == Mode.TRAIN_COST) {
                    updateCostUI();
                }
            }
            @Override public void onNothingSelected(AdapterView<?> parent) {}
        };

        costFromStation.setOnItemSelectedListener(stationListener);
        costToStation.setOnItemSelectedListener(stationListener);
    }

    void setupRoutePlannerUI(ArrayAdapter<LineItem> adapter) {

        routePlannerControls = findViewById(R.id.routePlannerControls);

        routeStartLabel = findViewById(R.id.routeStartLabel);
        routeEndLabel   = findViewById(R.id.routeEndLabel);

        routeFromLine = findViewById(R.id.routeFromLine);
        routeFromStation = findViewById(R.id.routeFromStation);

        routeToLine = findViewById(R.id.routeToLine);
        routeToStation = findViewById(R.id.routeToStation);

        customControls = findViewById(R.id.customControls);

        customConstraintsLabel = findViewById(R.id.customConstraintsLabel);

        routeStrategyContainer = findViewById(R.id.routeStrategyContainer);

        routeFromLine.setAdapter(adapter);
        routeToLine.setAdapter(adapter);

        refreshStationSpinner(routeFromLine, routeFromStation);
        refreshStationSpinner(routeToLine, routeToStation);

        mustStationsTitle = findViewById(R.id.mustStationsTitle);
        avoidStationsTitle = findViewById(R.id.avoidStationsTitle);
        avoidLinesTitle = findViewById(R.id.avoidLinesTitle);
        mustLinesTitle = findViewById(R.id.mustLinesTitle);

        addMustStationBtn = findViewById(R.id.addMustStationBtn);
        addMustStationBtn.setOnClickListener(v -> addMustStationRow());

        mustLinesContainer = findViewById(R.id.mustLinesContainer);
        buildMustLineCheckboxes();

        addAvoidStationBtn = findViewById(R.id.addAvoidStationBtn);
        addAvoidStationBtn.setOnClickListener(v -> addAvoidStationRow());

        avoidLinesContainer = findViewById(R.id.avoidLinesContainer);
        buildAvoidLineCheckboxes();

        rankingPreferenceTitle = findViewById(R.id.rankingPreferenceTitle);

        applyCustomBtn = findViewById(R.id.applyCustomBtn);

        minTimeCheck = findViewById(R.id.minTimeCheck);
        minTransferCheck = findViewById(R.id.minTransferCheck);

        updateCustomLabels();

        applyCustomBtn.setOnClickListener(v -> applyCustomRoute());

        setupRouteStrategyButtons();
        updateRoutePlannerLabels();

        routeFromLine.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(routeFromLine, routeFromStation);

                // Immediately clear result
                clearRouteResult();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        routeToLine.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(routeToLine, routeToStation);

                // Immediately clear result
                clearRouteResult();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        AdapterView.OnItemSelectedListener routeStationListener =
                new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                        if (currentMode == Mode.ROUTE_PLANNER) {
                            recomputeRoutePlanner();
                        }
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> parent) {}
                };

        routeFromStation.setOnItemSelectedListener(routeStationListener);
        routeToStation.setOnItemSelectedListener(routeStationListener);
    }

    void setupRouteStrategyButtons() {

        routeStrategyContainer.removeAllViews();

        for (RouteStrategy strategy : RouteStrategy.values()) {

            Button btn = new Button(this, null, 0, R.style.ModeButton);

            btn.setText(getStrategyLabel(strategy));
            btn.setTypeface(btn.getTypeface(), android.graphics.Typeface.BOLD);
            btn.setGravity(Gravity.CENTER);
            btn.setMinHeight(0);
            btn.setIncludeFontPadding(false);
            btn.setBackgroundResource(R.drawable.mode_button_selector);
            btn.setPadding(25, 20, 25, 20);

            btn.setSelected(strategy == currentStrategy);

            btn.setOnClickListener(v -> {
                currentStrategy = strategy;
                updateRouteStrategyUI();
                recomputeRoutePlanner();
            });

            LinearLayout.LayoutParams lp =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                    );

            lp.setMargins(10, 0, 10, 0);
            btn.setLayoutParams(lp);

            routeStrategyContainer.addView(btn);
        }
    }

    void updateRouteStrategyUI() {

        for (int i = 0; i < routeStrategyContainer.getChildCount(); ++i) {
            View v = routeStrategyContainer.getChildAt(i);
            if (v instanceof Button) {
                Button b = (Button) v;
                RouteStrategy strategy = RouteStrategy.values()[i];
                b.setSelected(strategy == currentStrategy);
            }
        }

        if (currentStrategy == RouteStrategy.CUSTOM) {
            customControls.setVisibility(View.VISIBLE);
        } else {
            customControls.setVisibility(View.GONE);
        }
    }

    void updateRoutePlannerLabels() {

        if (routeStartLabel != null) {
            routeStartLabel.setText(getStartLabel());
        }

        if (routeEndLabel != null) {
            routeEndLabel.setText(getEndLabel());
        }

        if (customConstraintsLabel != null) {
            customConstraintsLabel.setText(getCustomConstraintsLabel());
        }
    }

    String getStrategyLabel(RouteStrategy strategy) {
        switch (getLanguage()) {

            case "zh":
                switch (strategy) {
                    case FASTEST: return "最快";
                    case LEAST_INTERCHANGE: return "最少轉乘";
                    case CUSTOM: return "自訂";
                }

            case "en":
                switch (strategy) {
                    case FASTEST: return "Fastest";
                    case LEAST_INTERCHANGE: return "Least Transfers";
                    case CUSTOM: return "Custom";
                }

            case "jp":
                switch (strategy) {
                    case FASTEST: return "最速";
                    case LEAST_INTERCHANGE: return "最少乗換";
                    case CUSTOM: return "カスタム";
                }

            case "kr":
                switch (strategy) {
                    case FASTEST: return "최단 시간";
                    case LEAST_INTERCHANGE: return "최소 환승";
                    case CUSTOM: return "사용자 지정";
                }
        }

        return "";
    }

    String getCustomConstraintsLabel() {
        switch (getLanguage()) {
            case "zh": return "自訂條件";
            case "en": return "Custom Constraints";
            case "jp": return "カスタム条件";
            case "kr": return "사용자 지정 조건";
            default: return "自訂條件";
        }
    }

    void updateCustomLabels() {
        mustStationsTitle.setText(getMustStationsLabel());
        mustLinesTitle.setText(getMustLinesLabel());
        avoidStationsTitle.setText(getAvoidStationsLabel());
        avoidLinesTitle.setText(getAvoidLinesLabel());

        addMustStationBtn.setText(getAddStationLabel());
        addAvoidStationBtn.setText(getAddStationLabel());
        applyCustomBtn.setText(getApplyCustomLabel());

        rankingPreferenceTitle.setText(getRankingPreferenceLabel());
        minTimeCheck.setText(getMinTimeLabel());
        minTransferCheck.setText(getMinTransferLabel());
    }

    String getMustStationsLabel() {
        switch (getLanguage()) {
            case "en": return "Must Pass Stations (Max 4)";
            case "jp": return "必経駅（最大4）";
            case "kr": return "반드시 지나야 할 역 (최대 4개)";
            default:   return "必經車站（最多 4 個）";
        }
    }

    String getMustLinesLabel() {
        switch (getLanguage()) {
            case "en": return "Must Use Lines";
            case "jp": return "必ず通る路線";
            case "kr": return "반드시 사용하는 노선";
            default:   return "必須經過路線";
        }
    }

    String getAvoidStationsLabel() {
        switch (getLanguage()) {
            case "en": return "Avoid Stations";
            case "jp": return "避ける駅";
            case "kr": return "피할 역";
            default:   return "避開車站";
        }
    }

    String getAvoidLinesLabel() {
        switch (getLanguage()) {
            case "en": return "Avoid Lines";
            case "jp": return "避ける路線";
            case "kr": return "피할 노선";
            default:   return "避開路線";
        }
    }

    String getRankingPreferenceLabel() {
        switch (getLanguage()) {
            case "en": return "Ranking Preference";
            case "jp": return "優先順位";
            case "kr": return "우선순위";
            default:   return "排序偏好";
        }
    }

    String getMinTimeLabel() {
        switch (getLanguage()) {
            case "en": return "Minimize Time";
            case "jp": return "最短時間";
            case "kr": return "최단 시간";
            default:   return "最短時間";
        }
    }

    String getMinTransferLabel() {
        switch (getLanguage()) {
            case "en": return "Minimize Transfers";
            case "jp": return "最少乗換";
            case "kr": return "최소 환승";
            default:   return "最少轉乘";
        }
    }

    String getAddStationLabel() {
        switch (getLanguage()) {
            case "en": return "+ Add Station";
            case "jp": return "+ 駅を追加";
            case "kr": return "+ 역 추가";
            default:   return "+ 新增車站";
        }
    }

    String getApplyCustomLabel() {
        switch (getLanguage()) {
            case "en": return "Apply Custom Route";
            case "jp": return "カスタム経路を適用";
            case "kr": return "사용자 경로 적용";
            default:   return "套用自訂路線";
        }
    }

    void updateCostUI() {
        if (costTable == null) {
            return;
        }
        costTable.removeAllViews();

        if (costFromLine.getSelectedItem() == null || costToLine.getSelectedItem() == null) {
            return;
        }

        LineItem fromL = (LineItem) costFromLine.getSelectedItem();
        LineItem toL   = (LineItem) costToLine.getSelectedItem();

        int fromSt = parseStationNo(fromL, costFromStation);
        int toSt   = parseStationNo(toL, costToStation);
        if (fromSt < 0 || toSt < 0) return;

        // Call JNI fare function(s)
        int adult  = getFare(fromL.code, fromSt, toL.code, toSt, ADULT.ordinal());
        int child  = getFare(fromL.code, fromSt, toL.code, toSt, CHILD.ordinal());
        int elderly= getFare(fromL.code, fromSt, toL.code, toSt, ELDERLY.ordinal());

        String[] labels = getFareRowLabels();
        addCostRow(labels[0], adult);
        addCostRow(labels[1], child);
        addCostRow(labels[2], elderly);

        updateCostLabels();
    }

    void addCostRow(String label, int value) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);

        TextView left = new TextView(this);
        left.setText(label);
        left.setTextSize(16);
        left.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 2));
        left.setTextColor(getColor(R.color.custom_pink));
        left.setPadding(12, 10, 12, 10);

        TextView right = new TextView(this);
        right.setText("$" + value);
        right.setTextSize(16);
        right.setGravity(Gravity.END);
        right.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        right.setTextColor(getColor(R.color.custom_pink));
        right.setPadding(12, 10, 12, 10);

        row.addView(left);
        row.addView(right);
        costTable.addView(row);
    }

    void displayRouteResult(String result) {

        LinearLayout container = findViewById(R.id.routeResultContainer);
        container.removeAllViews();

        TextView tv = new TextView(this);
        tv.setText(result);
        tv.setTextColor(getColor(R.color.custom_pink));
        tv.setTextSize(16);
        tv.setPadding(12, 12, 12, 12);

        container.addView(tv);
    }

    void displayManualResult(String result) {

        LinearLayout container = findViewById(R.id.manualResultContainer);
        container.removeAllViews();

        TextView tv = new TextView(this);
        tv.setText(result);
        tv.setTextColor(getColor(R.color.custom_pink));
        tv.setTextSize(16);
        tv.setPadding(12, 12, 12, 12);

        container.addView(tv);
    }

    void addMustStationRow() {

        if (mustStationRows.size() >= 4) {
            return;
        }

        LinearLayout container = findViewById(R.id.mustStationsContainer);

        LinearLayout rowLayout = new LinearLayout(this);
        rowLayout.setOrientation(LinearLayout.HORIZONTAL);
        rowLayout.setPadding(0, 0, 0, 0);

        LinearLayout.LayoutParams rowParams =
                new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                );

        rowParams.setMargins(0, 0, 0, 0);
        rowLayout.setLayoutParams(rowParams);

        Spinner lineSpinner = new Spinner(this);
        Spinner stationSpinner = new Spinner(this);
        Button removeBtn = new Button(this);

        removeBtn.setText("🗑");
        removeBtn.setMinHeight(10);
        removeBtn.setMinimumHeight(10);
        removeBtn.setMinWidth(10);
        removeBtn.setMinimumWidth(10);

        // Layout weights
        lineSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        stationSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 3));
        removeBtn.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        rowLayout.addView(lineSpinner);
        rowLayout.addView(stationSpinner);
        rowLayout.addView(removeBtn);

        container.addView(rowLayout);

        // Set adapter
        ArrayAdapter<LineItem> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, getLines());
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        lineSpinner.setAdapter(adapter);

        refreshStationSpinner(lineSpinner, stationSpinner);

        lineSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(lineSpinner, stationSpinner);
            }
            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });

        StationRow stationRow = new StationRow();
        stationRow.lineSpinner = lineSpinner;
        stationRow.stationSpinner = stationSpinner;
        stationRow.rootView = rowLayout;

        mustStationRows.add(stationRow);

        removeBtn.setOnClickListener(v -> {
            container.removeView(rowLayout);
            mustStationRows.remove(stationRow);
        });
    }

    void addAvoidStationRow() {

        LinearLayout container = findViewById(R.id.avoidStationsContainer);

        LinearLayout rowLayout = new LinearLayout(this);
        rowLayout.setOrientation(LinearLayout.HORIZONTAL);
        rowLayout.setPadding(0, 0, 0, 0);

        LinearLayout.LayoutParams rowParams =
                new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                );

        rowParams.setMargins(0, 0, 0, 0);
        rowLayout.setLayoutParams(rowParams);

        Spinner lineSpinner = new Spinner(this);
        Spinner stationSpinner = new Spinner(this);
        Button removeBtn = new Button(this);

        removeBtn.setText("🗑");
        removeBtn.setMinHeight(10);
        removeBtn.setMinimumHeight(10);
        removeBtn.setMinWidth(10);
        removeBtn.setMinimumWidth(10);

        // Layout weights
        lineSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        stationSpinner.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 3));
        removeBtn.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        rowLayout.addView(lineSpinner);
        rowLayout.addView(stationSpinner);
        rowLayout.addView(removeBtn);

        container.addView(rowLayout);

        ArrayAdapter<LineItem> adapter =
                new ArrayAdapter<>(this,
                        android.R.layout.simple_spinner_item,
                        getLines());

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        lineSpinner.setAdapter(adapter);

        refreshStationSpinner(lineSpinner, stationSpinner);

        lineSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                refreshStationSpinner(lineSpinner, stationSpinner);
            }

            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });

        StationRow row = new StationRow();
        row.lineSpinner = lineSpinner;
        row.stationSpinner = stationSpinner;
        row.rootView = rowLayout;

        avoidStationRows.add(row);

        removeBtn.setOnClickListener(v -> {
            container.removeView(rowLayout);
            avoidStationRows.remove(row);
        });
    }

    void buildMustLineCheckboxes() {

        mustLinesContainer.removeAllViews();
        mustLineChecks.clear();

        LineItem[] lines = getLines();

        for (LineItem line : lines) {

            CheckBox cb = new CheckBox(this);

            cb.setText("");  // no label text

            int color = getLineColor(line.code);

            // Set tint
            cb.setButtonTintList(
                    android.content.res.ColorStateList.valueOf(color)
            );

            LinearLayout.LayoutParams lp =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                    );

            lp.setMargins(12, 0, 12, 0);
            cb.setLayoutParams(lp);

            mustLinesContainer.addView(cb);
            mustLineChecks.add(cb);
        }
    }

    void buildAvoidLineCheckboxes() {

        avoidLinesContainer.removeAllViews();
        avoidLineChecks.clear();

        LineItem[] lines = getLines();

        for (LineItem line : lines) {

            CheckBox cb = new CheckBox(this);

            cb.setText("");  // no label
            cb.setButtonTintList(
                    android.content.res.ColorStateList.valueOf(
                            getLineColor(line.code)
                    )
            );

            LinearLayout.LayoutParams lp =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                    );

            lp.setMargins(12, 0, 12, 0);
            cb.setLayoutParams(lp);

            avoidLinesContainer.addView(cb);
            avoidLineChecks.add(cb);
        }
    }

    List<String> getSelectedMustLines() {

        List<String> result = new ArrayList<>();
        LineItem[] lines = getLines();

        for (int i = 0; i < mustLineChecks.size(); i++) {
            if (mustLineChecks.get(i).isChecked()) {
                result.add(lines[i].code);
            }
        }

        return result;
    }

    List<String> getSelectedAvoidLines() {

        List<String> result = new ArrayList<>();
        LineItem[] lines = getLines();

        for (int i = 0; i < avoidLineChecks.size(); i++) {
            if (avoidLineChecks.get(i).isChecked()) {
                result.add(lines[i].code);
            }
        }

        return result;
    }

    List<String> getMustStations() {

        List<String> result = new ArrayList<>();

        for (StationRow row : mustStationRows) {

            LineItem lineItem = (LineItem) row.lineSpinner.getSelectedItem();
            int stationNo = parseStationNo(lineItem, row.stationSpinner);

            if (lineItem != null && stationNo >= 0) {
                result.add(lineItem.code + stationNo);
            }
        }

        return result;
    }


    List<String> getAvoidStations() {

        List<String> result = new ArrayList<>();

        for (StationRow row : avoidStationRows) {

            LineItem lineItem = (LineItem) row.lineSpinner.getSelectedItem();
            int stationNo = parseStationNo(lineItem, row.stationSpinner);

            if (lineItem != null && stationNo >= 0) {
                result.add(lineItem.code + stationNo);
            }
        }

        return result;
    }

    int getLineColor(String code) {

        switch (code) {
            case "R":  return getColor(R.color.line_red);
            case "O":  return getColor(R.color.line_orange);
            case "Y":  return getColor(R.color.line_yellow);
            case "G":  return getColor(R.color.line_green);
            case "BL": return getColor(R.color.line_blue);
            case "BR": return getColor(R.color.line_brown);
            default:   return Color.GRAY;
        }
    }

    void applyCustomRoute() {

        LineItem fromL = (LineItem) routeFromLine.getSelectedItem();
        LineItem toL   = (LineItem) routeToLine.getSelectedItem();

        int fromSt = parseStationNo(fromL, routeFromStation);
        int toSt   = parseStationNo(toL, routeToStation);

        if (fromSt < 0 || toSt < 0) {
            return;
        }

        List<String> mustStations  = getMustStations();
        List<String> avoidStations = getAvoidStations();
        List<String> mustLines     = getSelectedMustLines();
        List<String> avoidLines    = getSelectedAvoidLines();

        boolean minimizeTime = minTimeCheck.isChecked();
        boolean minimizeTransfers = minTransferCheck.isChecked();

        // Show loading text
        String lang = getLanguage();
        if (lang.equals("en")) {
            displayRouteResult("Computing...");
        } else if (lang.equals("zh")) {
            displayRouteResult("計算中...");
        } else if (lang.equals("jp")) {
            displayRouteResult("計算中...");
        } else if (lang.equals("kr")) {
            displayRouteResult("계산 중...");
        }

        new Thread(() -> {

            String result = computeCustomRoute(fromL.code, fromSt, toL.code, toSt, mustStations.toArray(new String[0]), avoidStations.toArray(new String[0]), mustLines.toArray(new String[0]), avoidLines.toArray(new String[0]), minimizeTime, minimizeTransfers, getLanguageInt(), user_age.ordinal());

            runOnUiThread(() -> {
                displayRouteResult(result);
            });

        }).start();
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

    public native int getFare(String line1, int st1, String line2, int st2, int ageGroup);

    public native String computeFastestRoute(String fromLine, int fromStation, String toLine, int toStation, int lang, int ticketType);
    public native String computeLeastInterchangeRoute(String fromLine, int fromStation, String toLine, int toStation, int lang, int ticketType);
    public native String computeCustomRoute(String fromLine, int fromStation, String toLine, int toStation, String[] mustStations, String[] avoidStations, String[] mustLines, String[] avoidLines, boolean minimizeTime, boolean minimizeTransfers, int lang_int, int ageGroup);

    public native String computeManualPath(String[] stations, int lang, int ageGroup);
}