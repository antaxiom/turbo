#include <pebble.h>
#include "cfg.h"
#include "../main.h"

extern ClaySettings settings;

static void default_settings() {
    settings.hand_width = 6;
    settings.second_width = 3;
    settings.hour_color = GColorOrange;
    settings.min_color = GColorOrange;
    settings.dot_color = GColorBlack;
    settings.sec_color = GColorBlack;
    settings.hour_tick_color = GColorBlack;
    settings.sub_hour_tick_color = GColorBlack;
    settings.redline_color = GColorRed;
    settings.bg_color = GColorYellow;
    settings.enable_bg = true;
    settings.num_of_dots = 12;
    settings.num_of_sub_dots = 5;
    settings.sub_tick_indentation = 6;
    settings.hour_tick_size = 2;
    settings.sub_hour_tick_size = 1;
    settings.redline_hour = 9000;
    settings.pebble_color = GColorBlack;
    settings.enable_pebble = true;
    settings.date_color = GColorWhite;
    settings.enable_date = true;
    settings.do_bt_buzz = true;
    settings.sec_start = 0;
    settings.sec_end = 0;
};

void load_settings() {
    default_settings();
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void save_settings() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}