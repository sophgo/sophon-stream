#pragma once

#include "awtk.h"

BEGIN_C_DECLS
extern ret_t assets_init(void);
END_C_DECLS

#ifndef APP_DEFAULT_FONT
#define APP_DEFAULT_FONT "default"
#endif /*APP_DEFAULT_FONT*/

#ifndef LCD_WIDTH
#define LCD_WIDTH 320
#endif /*LCD_WIDTH*/

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 480
#endif /*LCD_HEIGHT*/

#ifndef APP_TYPE
#define APP_TYPE APP_SIMULATOR
#endif /*APP_TYPE*/

#ifndef GLOBAL_INIT
#define GLOBAL_INIT()
#endif /*GLOBAL_INIT*/

#ifndef GLOBAL_EXIT
#define GLOBAL_EXIT()
#endif /*GLOBAL_EXIT*/

#ifndef FINAL_EXIT
#define FINAL_EXIT()
#endif /*FINAL_EXIT*/

#ifndef APP_NAME
#define APP_NAME "awtk"
#endif /*APP_NAME*/

#ifndef APP_RES_ROOT
#define APP_RES_ROOT NULL
#endif /*APP_RES_ROOT*/

#ifndef APP_ENABLE_CONSOLE
#define APP_ENABLE_CONSOLE TRUE
#endif /*APP_ENABLE_CONSOLE*/

#include "base/custom_keys.inc"
#include "base/asset_loader_zip.h"

class EdgeAwtk {
public:
    EdgeAwtk(int, int);
    int m_LcdWeight;
    int m_LcdHeight;
    void run();

private:
    int guiAppStartEx(int lcd_w, int lcd_h, const char* res_root);
    int guiAppStart(int lcd_w, int lcd_h);

    static ret_t setLocaleValue(widget_t* widget, int32_t value);

    static ret_t onLocaleChange(void* ctx, event_t* e);
    static ret_t changeLocale(void* ctx, event_t* e);

    ret_t applicationInit();
    ret_t applicationExit();
};