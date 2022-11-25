#include <pebble.h>
#include "main.h"
#include "drawing/drawing.h"
#include "config/cfg.h"
#include "messaging/msg.h"

Window *main_window;
static Layer *clock_hands, *sec_hand, *bg_layer, *date_layer;
static BitmapLayer *logo_layer;
static GBitmap *turbo_logo;

extern int actual_hour;

ClaySettings settings;

static void bluetooth_callback(bool connected) {
    if(settings.do_bt_buzz == true && !connected) {
        vibes_short_pulse();
    }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  layer_mark_dirty(clock_hands);
  layer_mark_dirty(sec_hand);
}

static void sub_to_time(bool sec_true) {
  if(!sec_true) {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  }
}

void update_stuff() {
  update_time();

  window_set_background_color(main_window, settings.bg_color);

  layer_mark_dirty(clock_hands);
  layer_mark_dirty(bg_layer);
  layer_mark_dirty(sec_hand);
  layer_mark_dirty(date_layer);

  if(settings.sec_end == settings.sec_start) {
    layer_set_hidden(sec_hand, true);
    sub_to_time(false);
  } else if(settings.sec_start <= actual_hour && actual_hour <= settings.sec_end) {
    layer_set_hidden(sec_hand, false);
    sub_to_time(true);
  }

  layer_set_hidden(bg_layer, !settings.enable_bg);
  //layer_set_hidden(logo_layer, !settings.enable_pebble);
  layer_set_hidden(date_layer, !settings.enable_date);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  turbo_logo = gbitmap_create_with_resource(RESOURCE_ID_TURBO_LOGO);

  window_set_background_color(main_window, settings.bg_color);
  
  update_time();

  bg_layer = layer_create(bounds);
  layer_set_update_proc(bg_layer, draw_hour_marks_update_proc);
  layer_add_child(window_layer, bg_layer);

  logo_layer = bitmap_layer_create(GRect(center.x-30,60,59,10));
  bitmap_layer_set_compositing_mode(logo_layer, GCompOpSet);
  bitmap_layer_set_bitmap(logo_layer, turbo_logo);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
  //layer_set_update_proc(logo_layer, pebble_text_update_proc);

  date_layer = layer_create(bounds);
  layer_set_update_proc(date_layer, date_update_proc);
  layer_add_child(window_layer, date_layer);

  clock_hands = layer_create(bounds);
  layer_set_update_proc(clock_hands, hands_draw_update_proc);
  layer_add_child(window_layer, clock_hands);

  sec_hand = layer_create(bounds);
  layer_set_update_proc(sec_hand, draw_sec_update_proc);
  layer_add_child(window_layer, sec_hand);
}

static void main_window_unload() {
  layer_destroy(clock_hands);
  gbitmap_destroy(turbo_logo);
  bitmap_layer_destroy(logo_layer);
}

static void init() {
  main_window = window_create();

  init_msg();
  load_settings();

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  sub_to_time(false);

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);

  bluetooth_callback(connection_service_peek_pebble_app_connection());

  update_stuff();
}

static void deinit() {
  tick_timer_service_unsubscribe();
  unobstructed_area_service_unsubscribe();
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
