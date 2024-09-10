#include <EdgeAwtk.h>

// EdgeAwtk::EdgeAwtk() {

// }

EdgeAwtk::EdgeAwtk(int w, int h):
  m_LcdWeight(w), m_LcdHeight(h) {

}

void EdgeAwtk::run() {
    guiAppStart(m_LcdWeight, m_LcdHeight);
}

int EdgeAwtk::guiAppStart(int lcd_w, int lcd_h) {
  return guiAppStartEx(lcd_w, lcd_h, APP_RES_ROOT);
}

int EdgeAwtk::guiAppStartEx(int lcd_w, int lcd_h, const char* res_root) {
  tk_init(lcd_w, lcd_h, APP_MOBILE, APP_NAME, res_root);

#ifdef ASSETS_ZIP
  assets_manager_set_res_root(assets_manager(), "");
  log_debug("Load assets from zip: %s\n", ASSETS_ZIP);
  assets_manager_set_loader(assets_manager(), asset_loader_zip_create(ASSETS_ZIP));
#elif defined(ASSETS_CUSTOM_INIT)
  ASSETS_CUSTOM_INIT();
#endif /*ASSETS_ZIP*/

#if defined(WITH_LCD_PORTRAIT)
  if (lcd_w > lcd_h) {
    tk_set_lcd_orientation(LCD_ORIENTATION_90);
  }
#endif /*WITH_LCD_PORTRAIT*/

#ifdef WITH_LCD_LANDSCAPE
  if (lcd_w < lcd_h) {
    tk_set_lcd_orientation(LCD_ORIENTATION_90);
  }
#endif /*WITH_LCD_PORTRAIT*/

#ifndef TK_IS_PC
#ifdef APP_LCD_ORIENTATION
#if defined(APP_ENABLE_FAST_LCD_PORTRAIT)
  tk_enable_fast_lcd_portrait(TRUE);
#endif
  tk_set_lcd_orientation(APP_LCD_ORIENTATION);
#endif
#endif/*TK_IS_PC*/

  system_info_set_default_font(system_info(), APP_DEFAULT_FONT);
  assets_init();
#ifndef WITH_FS_RES
  locale_info_reload(locale_info());
#endif

#ifndef WITHOUT_EXT_WIDGETS
  tk_ext_widgets_init();
#endif /*WITHOUT_EXT_WIDGETS*/

#ifdef NDEBUG
  log_set_log_level(LOG_LEVEL_INFO);
#else
  log_set_log_level(LOG_LEVEL_DEBUG);
#endif /*NDEBUG*/
  log_info("Build at: %s %s\n", __DATE__, __TIME__);

#ifdef ENABLE_CURSOR
  window_manager_set_cursor(window_manager(), "cursor");
#endif /*ENABLE_CURSOR*/

#ifdef WIN32
  setvbuf(stdout, NULL, _IONBF, 0);
#elif defined(HAS_STDIO)
  setlinebuf(stdout);
#endif /*WIN32*/

  GLOBAL_INIT();
#if defined(APP_DEFAULT_LANGUAGE) && defined(APP_DEFAULT_COUNTRY)
  locale_info_change(locale_info(), APP_DEFAULT_LANGUAGE, APP_DEFAULT_COUNTRY);
#endif /*APP_DEFAULT_LANGUAGE and APP_DEFAULT_LANGUAGE*/
  custom_keys_init(TRUE);
  applicationInit();
  tk_run();
  applicationExit();
  custom_keys_deinit(TRUE);
  
  GLOBAL_EXIT();
  tk_exit();

  FINAL_EXIT();
#ifdef HAS_STDIO
  fflush(stdout);
#endif /*HAS_STDIO*/

#if defined(WIN32) && !defined(MINGW)
  str_reset(&str);
#endif /*WIN32*/

#if defined(IOS) || defined(ANDROID)
  exit(0);
#endif /*IOS | ANDROID*/

  return 0;
}


ret_t EdgeAwtk::changeLocale(void* ctx, event_t* e) {
  widget_t* radio_button = WIDGET(e->target);
  if (widget_get_value_int(radio_button)) {
    char country[3];
    char language[3];
    const char* str = (const char*)ctx;

    widget_t* widget = WIDGET(e->target);
    if (widget_get_value(widget)) {
      strncpy(language, str, 2);
      strncpy(country, str + 3, 2);
      locale_info_change(locale_info(), language, country);
    }
  }

  return RET_OK;
}

ret_t EdgeAwtk::setLocaleValue(widget_t* widget, int32_t value) {
  char str[64];
  const char* format = locale_info_tr(locale_info(), "value is %d");
  tk_snprintf(str, sizeof(str), format, value);
  widget_set_text_utf8(widget, str);

  return RET_OK;
}

ret_t EdgeAwtk::onLocaleChange(void* ctx, event_t* e) {
  (void)ctx;
  (void)e;
  widget_t* win = WIDGET(ctx);
  widget_t* value = widget_lookup(win, "value", TRUE);

  setLocaleValue(value, 100);
  log_debug("locale_infod change: %s_%s\n", locale_info()->language, locale_info()->country);

  return RET_OK;
}

ret_t EdgeAwtk::applicationInit() {
  widget_t* ok = NULL;
  widget_t* cancel = NULL;
  widget_t* value = NULL;
  widget_t* radio_button = NULL;
  widget_t* win = window_create(NULL, 0, 0, 0, 0);

  widget_on(win, EVT_LOCALE_CHANGED, &EdgeAwtk::onLocaleChange, win);

  ok = button_create(win, 10, 5, 80, 30);
  widget_set_tr_text(ok, "ok");

  cancel = button_create(win, 100, 5, 80, 30);
  widget_set_tr_text(cancel, "cancel");

  value = label_create(win, 200, 5, 80, 30);
  widget_set_name(value, "value");
  setLocaleValue(value, 100);

  radio_button = check_button_create_radio(win, 10, 200, 80, 30);
  widget_set_tr_text(radio_button, "English");
  widget_on(radio_button, EVT_VALUE_CHANGED, &EdgeAwtk::changeLocale, (void*)"en_US");
  widget_set_value(radio_button, 1);

  radio_button = check_button_create_radio(win, 100, 200, 80, 30);
  widget_set_tr_text(radio_button, "Chinese");
  widget_on(radio_button, EVT_VALUE_CHANGED, &EdgeAwtk::changeLocale, (void*)"zh_CN");

  return RET_OK;
}

ret_t EdgeAwtk::applicationExit() {
  log_debug("applicationExit\n");
  return RET_OK;
}