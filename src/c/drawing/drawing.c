#include <pebble.h>
#include "../main.h"
#include "../messaging/msg.h"
extern Window *main_window;
extern ClaySettings settings;
#include <math.h>

int hour, min, sec, actual_hour;
static char date_char[] = "DD";

extern int num_stripes[];

int if_quickview_else(int if_no, int if_yes, GRect quick_view_bounds) {
  if(quick_view_bounds.size.h == layer_get_bounds(window_get_root_layer(main_window)).size.h) {
    return if_no;
  } else {
    return if_yes;
  }
}

void update_time() {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);

  strftime(date_char, sizeof(date_char), "%d", t);

  actual_hour = t->tm_hour;

  min = 360 * t->tm_min / 60;
  hour = 360 * (t->tm_hour % 12 * 6 + t->tm_min / 10) / (12 * 6);
  sec = 360 * t->tm_sec / 60;
}

static void draw_hand(int length, int width, int rot, GColor color, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));
  GPoint center = grect_center_point(&bounds);

  GPoint full_length = {
    .x = center.x + (sin_lookup(DEG_TO_TRIGANGLE(rot)) * length / TRIG_MAX_RATIO),
    .y = center.y - (cos_lookup(DEG_TO_TRIGANGLE(rot)) * length / TRIG_MAX_RATIO)
  };

  GPoint center_offset = {
    .x = center.x + (sin_lookup(DEG_TO_TRIGANGLE(rot)) * 5 / TRIG_MAX_RATIO),
    .y = center.y - (cos_lookup(DEG_TO_TRIGANGLE(rot)) * 5 / TRIG_MAX_RATIO)
  };

  GPoint opposite_length = {
    .x = center.x - (sin_lookup(DEG_TO_TRIGANGLE(rot)) * 12 / TRIG_MAX_RATIO),
    .y = center.y + (cos_lookup(DEG_TO_TRIGANGLE(rot)) * 12 / TRIG_MAX_RATIO)
  };

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, width+2);
  graphics_draw_line(ctx, center, opposite_length);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, width+2);
  graphics_draw_line(ctx, center, full_length);

  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, width-2);
  graphics_draw_line(ctx, center_offset, full_length);
}

static void draw_line_bg (GContext *ctx, int dist_from_center, int dist_from_outside) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));
  GPoint center = grect_center_point(&bounds);

  for(int i = 0; i < settings.num_of_dots; i++) {

    graphics_context_set_stroke_width(ctx, settings.hour_tick_size * 2);

    int angle = TRIG_MAX_ANGLE * i / settings.num_of_dots;
    int degree_angle = 360 * i / settings.num_of_dots;


    if (degree_angle >= 360 / 12 * (settings.redline_hour/1000)) {
      graphics_context_set_stroke_color(ctx, settings.redline_color);
    }
    else {
      graphics_context_set_stroke_color(ctx, settings.hour_tick_color);
    }

    GPoint point1 = {
      .x = center.x + sin_lookup(angle) * dist_from_center / TRIG_MAX_RATIO,
      .y = center.y - cos_lookup(angle) * dist_from_center / TRIG_MAX_RATIO
    };

    GPoint point2 = {
      .x = center.x + sin_lookup(angle) * dist_from_outside / TRIG_MAX_RATIO,
      .y = center.y - cos_lookup(angle) * dist_from_outside / TRIG_MAX_RATIO
    };

    graphics_draw_line(ctx, point1, point2);


    graphics_context_set_stroke_width(ctx, settings.sub_hour_tick_size * 2);

    for(int i = 0; i < settings.num_of_sub_dots; i++) {

      int sub_angle = angle + (TRIG_MAX_ANGLE / settings.num_of_dots /settings.num_of_sub_dots * i);
      int degree_sub_angle = degree_angle + (360 / settings.num_of_dots /settings.num_of_sub_dots * i);

      if ( degree_sub_angle >= 360 / 12 * (settings.redline_hour/1000)) {
        graphics_context_set_stroke_color(ctx, settings.redline_color);
      }
      else {
        graphics_context_set_stroke_color(ctx, settings.sub_hour_tick_color);
      }

      GPoint sub_point1 = {
        .x = center.x + sin_lookup(sub_angle) * (dist_from_center + settings.sub_tick_indentation) / TRIG_MAX_RATIO,
        .y = center.y - cos_lookup(sub_angle) * (dist_from_center + settings.sub_tick_indentation) / TRIG_MAX_RATIO
      };

      GPoint sub_point2 = {
        .x = center.x + sin_lookup(sub_angle) * dist_from_outside / TRIG_MAX_RATIO,
        .y = center.y - cos_lookup(sub_angle) * dist_from_outside / TRIG_MAX_RATIO
      };

      graphics_draw_line(ctx, sub_point1, sub_point2);

    }
  }
}

void hands_draw_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));

  int m_length = if_quickview_else(68, 53, bounds);
  int h_length = if_quickview_else(45, 35, bounds);

  graphics_context_set_fill_color(ctx, settings.dot_color);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 10);

  draw_hand(m_length, settings.hand_width, min, settings.min_color, ctx);
  draw_hand(h_length, settings.hand_width, hour, settings.hour_color, ctx);

}

void draw_sec_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));
  
  int s_length = if_quickview_else(55, 40, bounds);

  draw_hand(s_length, settings.second_width, sec, settings.sec_color, ctx);
}

void draw_hour_marks_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));
  GPoint center = grect_center_point(&bounds);

  double x_scale = .9;
  double y_scale = .9;

  int cent_dist = if_quickview_else(70, 54, bounds);
  draw_line_bg(ctx, PBL_IF_ROUND_ELSE(78, cent_dist), PBL_IF_ROUND_ELSE(90,80));
}

void pebble_text_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));

  int pebb_y_offset = if_quickview_else(40, 25, bounds);

  //graphics_context_set_text_color(ctx, settings.pebble_color);
  //graphics_draw_text(ctx, "turbo", fonts_get_system_font(FONT_KEY_GOTHIC_24), GRect(0, pebb_y_offset, bounds.size.w, 50), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);

  
}

void date_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(main_window));

  int square_quickview_offset = if_quickview_else(55, 70, bounds);

  graphics_context_set_fill_color(ctx,settings.hour_tick_color);
  int box_width = 35;
  int box_height = 24;
  //graphics_draw_round_rect(ctx,GRect((bounds.size.w/2) - (box_width/2), (bounds.size.h/2) + (box_height/2) + 9, box_width, box_height),5);
  graphics_fill_rect(ctx,GRect((bounds.size.w/2) - (box_width/2), (bounds.size.h/2) + (box_height/2) + 9, box_width, box_height),6,GCornersAll);

  graphics_context_set_text_color(ctx, settings.date_color);
  graphics_draw_text(ctx, date_char, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS), GRect(bounds.size.w /2 - 24, bounds.size.h / 2 + 20, 50, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
}