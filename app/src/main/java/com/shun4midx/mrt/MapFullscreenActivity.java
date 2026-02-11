package com.shun4midx.mrt;

import android.os.Bundle;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;

import com.github.chrisbanes.photoview.PhotoView;
public class MapFullscreenActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map_fullscreen);

        ImageButton btnClose = findViewById(R.id.btnClose);
        btnClose.setOnClickListener(v -> finish());

        PhotoView map = findViewById(R.id.fullMap);

        int mapRes = getIntent().getIntExtra("mapRes", 0);
        map.setImageResource(mapRes);
    }
}
