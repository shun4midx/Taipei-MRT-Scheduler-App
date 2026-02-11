package com.shun4midx.mrt;

public final class LineItem {
    public final String code;
    public final String emoji; // from LINE_EMOJIS

    public LineItem(String line, String emoji) {
        this.code = line;
        this.emoji = emoji;
    }

    @Override
    public String toString() {
        return emoji; // spinner shows emoji only
    }
}