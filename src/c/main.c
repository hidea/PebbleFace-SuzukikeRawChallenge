#include <pebble.h>

static Window *s_main_window;

static BitmapLayer *s_pdt_background_layer;
static GBitmap *s_pdt_background_bitmap;
static BitmapLayer *s_mdt_background_layer;
static GBitmap *s_mdt_background_bitmap;

static TextLayer *s_timezone_layer;
static TextLayer *s_time_layer;
static TextLayer *s_jst_time_layer;
static TextLayer *s_pdt_time_layer;
static TextLayer *s_mdt_time_layer;
static TextLayer *s_edt_time_layer;


TextLayer *create_textlayer(Window* window, GRect grect, GColor color, char* text, const char* font, GTextAlignment align) {
  // Create time TextLayer
  TextLayer *text_layer = text_layer_create(grect);
  text_layer_set_background_color (text_layer, GColorClear);
  text_layer_set_text_color(text_layer, color);
  text_layer_set_text(text_layer, text);
  // Improve the layout to be more like a watchface
  text_layer_set_font(text_layer, fonts_get_system_font(font));
  text_layer_set_text_alignment(text_layer, align);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));  
  
  return text_layer;
}


static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_pdt_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUZUKIKE_PDT);
  s_pdt_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_pdt_background_layer, s_pdt_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_pdt_background_layer));

  s_mdt_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUZUKIKE_MDT);
  s_mdt_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_mdt_background_layer, s_mdt_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_mdt_background_layer));
  
  // Create time TextLayer
  s_jst_time_layer = create_textlayer(window, GRect(1,15,143,50), GColorImperialPurple, "00:00", FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM, GTextAlignmentLeft);
  s_pdt_time_layer = create_textlayer(window, GRect(1,81,143,50), GColorImperialPurple, "00:00", FONT_KEY_LECO_32_BOLD_NUMBERS, GTextAlignmentLeft);
  s_mdt_time_layer = create_textlayer(window, GRect(0,81,144,50), GColorImperialPurple, "00:00", FONT_KEY_LECO_32_BOLD_NUMBERS, GTextAlignmentCenter);
  s_edt_time_layer = create_textlayer(window, GRect(0,112,143,50), GColorImperialPurple, "00:00", FONT_KEY_LECO_32_BOLD_NUMBERS, GTextAlignmentRight);

  // Create time TextLayer
  s_time_layer = create_textlayer(window, GRect(0,43,144,50), GColorOxfordBlue, "00:00", FONT_KEY_LECO_36_BOLD_NUMBERS, GTextAlignmentRight);
  // Create timezone TextLayer
  s_timezone_layer = create_textlayer(window, GRect(0,75,140,16), GColorImperialPurple, "-", FONT_KEY_GOTHIC_14, GTextAlignmentRight);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_timezone_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_pdt_time_layer);
  text_layer_destroy(s_mdt_time_layer);
  text_layer_destroy(s_edt_time_layer);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  
  // Write the current hours and minutes into the buffer
  // Use 24 hour format

  static char local[] = "00:00";
  struct tm *local_tm = localtime(&temp);
  int local_hour = local_tm->tm_hour;
  strftime(local, sizeof(local), "%H:%M", local_tm);

  static char jst[] = "00:00";
  struct tm *jst_tm = gmtime(&temp);
  jst_tm->tm_hour += 9;
  if (jst_tm->tm_hour >= 24) jst_tm->tm_hour -= 24;
  strftime(jst, sizeof(jst), "%H:%M", jst_tm);

  static char pdt[] = "00:00";
  struct tm *pdt_tm = gmtime(&temp);
  pdt_tm->tm_hour -= 7;
  if (pdt_tm->tm_hour < 0) pdt_tm->tm_hour += 24;
  int pdt_hour = pdt_tm->tm_hour;
  strftime(pdt, sizeof(pdt), "%H:%M", pdt_tm);

  static char mdt[] = "00:00";
  struct tm *mdt_tm = gmtime(&temp);
  mdt_tm->tm_hour -= 6;
  if (mdt_tm->tm_hour < 0) mdt_tm->tm_hour += 24;
  strftime(mdt, sizeof(mdt), "%H:%M", mdt_tm);
  
  static char edt[] = "00:00          ";
  struct tm *edt_tm = gmtime(&temp);
  edt_tm->tm_hour -= 4;
  if (edt_tm->tm_hour < 0) edt_tm->tm_hour += 24;
  strftime(edt, sizeof(edt), "%H:%M", edt_tm);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, local);
  text_layer_set_text(s_jst_time_layer, jst);
  text_layer_set_text(s_pdt_time_layer, pdt);
  text_layer_set_text(s_mdt_time_layer, mdt);
  text_layer_set_text(s_edt_time_layer, edt);
    
  if (local_hour == pdt_hour) {
    layer_set_hidden((Layer *)s_pdt_time_layer, true);
    layer_set_hidden((Layer *)s_mdt_time_layer, false);
    layer_set_hidden((Layer *)s_pdt_background_layer, true);
    layer_set_hidden((Layer *)s_mdt_background_layer, false);
  }
  else {
    layer_set_hidden((Layer *)s_pdt_time_layer, false);
    layer_set_hidden((Layer *)s_mdt_time_layer, true);
    layer_set_hidden((Layer *)s_pdt_background_layer, false);
    layer_set_hidden((Layer *)s_mdt_background_layer, true);
  }

  static char localtz[TIMEZONE_NAME_LENGTH];
  clock_get_timezone(localtz, sizeof(localtz));
  text_layer_set_text(s_timezone_layer, localtz);
}

static void tick_handler(struct tm*tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
    
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}


