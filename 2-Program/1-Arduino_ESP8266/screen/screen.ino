/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Breakout
  ----> http://www.adafruit.com/products/1770

 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include "TouchScreen.h"

// These are the four touchscreen analog pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 7   // can be any digital pin
#define XP 8   // can be any digital pin

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// The display uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Size of the color selection boxes and the paintbrush size
#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     240
#define MARGIN            10

#define BANNER_HEIGHT     40

#define BUTTON_WIDTH      (SCREEN_WIDTH/2)-(2*MARGIN)
#define BUTTON_HEIGHT     (SCREEN_HEIGHT)-(BANNER_HEIGHT)-(2*MARGIN)
#define BUTTON_RADIUS     20

#define BUTTON_TRASH_X    MARGIN
#define BUTTON_TRASH_Y    BANNER_HEIGHT+MARGIN
#define BUTTON_RECYCLE_X  (SCREEN_WIDTH/2)+MARGIN
#define BUTTON_RECYCLE_Y  BANNER_HEIGHT+MARGIN

#define LOGO_WIFI_X       8
#define LOGO_WIFI_Y       5

#define LOGO_RECYCLE_X    BUTTON_RECYCLE_X+MARGIN
#define LOGO_RECYCLE_Y    BUTTON_RECYCLE_Y+MARGIN
#define LOGO_RECYCLE_WIDTH        64
#define LOGO_RECYCLE_HEIGHT       57

#define TEXT_BANNER_X     60
#define TEXT_BANNER_Y     13

#define COLOR_GREY                0x5269
#define COLOR_COMMON_TRASH        0x3480
#define COLOR_COMMONN_TRASH_LINE  0x7FEC
#define COLOR_RECYCLE_TRASH       0xFD80
#define COLOR_RECYCLE_TRASH_LINE  0xFFCC

typedef enum status_e {
  DISABLED,
  ENABLED
}status_t;

typedef enum phase_e {
  WAIT_M,
  CONNECT_M,
  CHOICE_M,
  TRASH_M,
  RECYCLE_M
} phase_t;

typedef enum button_e {
  TRASH_BUTTON,
  RECYCLE_BUTTON,
  NONE
} button_t;


const PROGMEM uint8_t logo_recycle[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void setup(void) {
  
  tft.begin();
  tft.setRotation(1);
  // Reset Screen to black
  tft.fillScreen(ILI9341_BLACK);

  // Draw wifi icon
  wifi_icon(DISABLED);
  // Draw Banner
  banner_message(WAIT_M);
  // Draw buttons
  draw_buttons(DISABLED);
  delay(4000);
  wifi_icon(ENABLED);
  banner_message(CONNECT_M);
  delay(4000);
  banner_message(CHOICE_M);
  draw_buttons(ENABLED);
}

void loop()
{
  button_t button_pressed = NONE;
  // Retrieve a point  
  TSPoint p = ts.getPoint();

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
     return;
  }
  
  // Scale from ~0->1000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  int y = tft.height() - p.x;
  int x = p.y;

  
  // Get button pressed
  if( (x > BUTTON_TRASH_X) && (x < (BUTTON_TRASH_X+BUTTON_WIDTH))  
    &&(y > BUTTON_TRASH_Y) && (y < (BUTTON_TRASH_Y+BUTTON_HEIGHT))){
      button_pressed = TRASH_BUTTON;
  }
  else if( (x > BUTTON_RECYCLE_X) && (x < (BUTTON_RECYCLE_X+BUTTON_WIDTH))  
         &&(y > BUTTON_RECYCLE_Y) && (y < (BUTTON_RECYCLE_Y+BUTTON_HEIGHT))){
      button_pressed = RECYCLE_BUTTON;
  }
  else {
    button_pressed = NONE;
  }

  if(button_pressed == RECYCLE_BUTTON) {
    banner_message(RECYCLE_M);
  }
  else if(button_pressed == TRASH_BUTTON){
    banner_message(TRASH_M);
  }
  else if(button_pressed == NONE) {
    banner_message(CHOICE_M);
  }
    
  // Sleep Arduino
  
}

void clear_banner()
{
  tft.fillRect(TEXT_BANNER_X, 0, SCREEN_WIDTH-TEXT_BANNER_X, BANNER_HEIGHT, ILI9341_BLACK);
}

void banner_message(phase_t phase)
{
  clear_banner();

  switch(phase) {

    case CHOICE_M:
      tft.setCursor(TEXT_BANNER_X, TEXT_BANNER_Y);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.print("Choose a bin");
      break;

    case CONNECT_M:
      tft.setCursor(TEXT_BANNER_X, TEXT_BANNER_Y);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.print("Connecting...");
      break;

    case TRASH_M:
      tft.setCursor(TEXT_BANNER_X, TEXT_BANNER_Y);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.print("Go to trash !");
      break;

    case RECYCLE_M:
      tft.setCursor(TEXT_BANNER_X, TEXT_BANNER_Y);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.print("Recycle !");
      break;

    case WAIT_M:
    default:
      tft.setCursor(TEXT_BANNER_X, TEXT_BANNER_Y);
      tft.setTextColor(ILI9341_WHITE);  
      tft.setTextSize(3);
      tft.print("Wait...");
      break;
  }
}

void wifi_icon(status_t status)
{
  if(status == ENABLED) {
    draw_wifi_logo(ILI9341_CYAN);
  }
  else {
    draw_wifi_logo(COLOR_GREY);
  }
}

void draw_buttons(status_t status)
{
  if(status == DISABLED) {
    tft.fillRoundRect(BUTTON_TRASH_X, BUTTON_TRASH_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_GREY);
    tft.drawRoundRect(BUTTON_TRASH_X, BUTTON_TRASH_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_GREY);
    tft.fillRoundRect(BUTTON_RECYCLE_X, BUTTON_RECYCLE_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_GREY);
    tft.drawRoundRect(BUTTON_RECYCLE_X, BUTTON_RECYCLE_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_GREY);
  }
  else {
    tft.fillRoundRect(BUTTON_TRASH_X, BUTTON_TRASH_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_COMMON_TRASH);
    tft.drawRoundRect(BUTTON_TRASH_X, BUTTON_TRASH_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_COMMONN_TRASH_LINE);
    tft.fillRoundRect(BUTTON_RECYCLE_X, BUTTON_RECYCLE_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_RECYCLE_TRASH);
    tft.drawRoundRect(BUTTON_RECYCLE_X, BUTTON_RECYCLE_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, COLOR_RECYCLE_TRASH_LINE);
  }

  // Draw bins logos
  //tft.drawBitmap(LOGO_RECYCLE_X, LOGO_RECYCLE_Y, (uint8_t *)pgm_read_byte(logo_recycle), LOGO_RECYCLE_WIDTH, LOGO_RECYCLE_HEIGHT, COLOR_GREY);
  
}

void draw_wifi_logo(uint16_t color)
{
  // Draw wifi icon
  tft.drawPixel(LOGO_WIFI_X + 9, LOGO_WIFI_Y + 14, color);
  tft.drawPixel(LOGO_WIFI_X + 10, LOGO_WIFI_Y + 14, color);
  tft.drawPixel(LOGO_WIFI_X + 8, LOGO_WIFI_Y + 13, color);
  tft.drawPixel(LOGO_WIFI_X + 9, LOGO_WIFI_Y + 13, color);
  tft.drawPixel(LOGO_WIFI_X + 10, LOGO_WIFI_Y + 13, color);
  tft.drawPixel(LOGO_WIFI_X + 11, LOGO_WIFI_Y + 13, color);
  tft.drawPixel(LOGO_WIFI_X + 8, LOGO_WIFI_Y + 12, color);
  tft.drawPixel(LOGO_WIFI_X + 9, LOGO_WIFI_Y + 12, color);
  tft.drawPixel(LOGO_WIFI_X + 10, LOGO_WIFI_Y + 12, color);
  tft.drawPixel(LOGO_WIFI_X + 11, LOGO_WIFI_Y + 12, color);
  tft.drawPixel(LOGO_WIFI_X + 9, LOGO_WIFI_Y + 11, color);
  tft.drawPixel(LOGO_WIFI_X + 10, LOGO_WIFI_Y + 11, color);

  tft.drawPixel(LOGO_WIFI_X + 4, LOGO_WIFI_Y + 10, color);
  tft.drawPixel(LOGO_WIFI_X + 5, LOGO_WIFI_Y + 10, color);
  tft.drawPixel(LOGO_WIFI_X + 14, LOGO_WIFI_Y + 10, color);
  tft.drawPixel(LOGO_WIFI_X + 15, LOGO_WIFI_Y + 10, color);
  tft.drawPixel(LOGO_WIFI_X + 4, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 5, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 6, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 13, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 14, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 15, LOGO_WIFI_Y + 9, color);
  tft.drawPixel(LOGO_WIFI_X + 5, LOGO_WIFI_Y + 8, color);
  tft.drawPixel(LOGO_WIFI_X + 6, LOGO_WIFI_Y + 8, color);
  tft.drawPixel(LOGO_WIFI_X + 7, LOGO_WIFI_Y + 8, color);
  tft.drawPixel(LOGO_WIFI_X + 12, LOGO_WIFI_Y + 8, color);
  tft.drawPixel(LOGO_WIFI_X + 13, LOGO_WIFI_Y + 8, color);
  tft.drawPixel(LOGO_WIFI_X + 14, LOGO_WIFI_Y + 8, color);

  tft.drawFastHLine(LOGO_WIFI_X + 6, LOGO_WIFI_Y + 7, 8, color);

  tft.drawPixel(LOGO_WIFI_X, LOGO_WIFI_Y + 6, color);
  tft.drawPixel(LOGO_WIFI_X + 1, LOGO_WIFI_Y + 6, color);
  tft.drawFastHLine(LOGO_WIFI_X + 8, LOGO_WIFI_Y + 6, 4, color);
  tft.drawPixel(LOGO_WIFI_X + 18, LOGO_WIFI_Y + 6, color);
  tft.drawPixel(LOGO_WIFI_X + 19, LOGO_WIFI_Y + 6, color);

  tft.drawFastHLine(LOGO_WIFI_X, LOGO_WIFI_Y + 5, 3, color);
  tft.drawFastHLine(LOGO_WIFI_X + 17, LOGO_WIFI_Y + 5, 3, color);
  tft.drawFastHLine(LOGO_WIFI_X + 1, LOGO_WIFI_Y + 4, 3, color);
  tft.drawFastHLine(LOGO_WIFI_X + 16, LOGO_WIFI_Y + 4, 3, color);
  tft.drawFastHLine(LOGO_WIFI_X + 2, LOGO_WIFI_Y + 3, 3, color);
  tft.drawFastHLine(LOGO_WIFI_X + 15, LOGO_WIFI_Y + 3, 3, color);

  tft.drawFastHLine(LOGO_WIFI_X + 3, LOGO_WIFI_Y + 2, 5, color);
  tft.drawFastHLine(LOGO_WIFI_X + 12, LOGO_WIFI_Y + 2, 5, color);
  tft.drawFastHLine(LOGO_WIFI_X + 5, LOGO_WIFI_Y + 1, 10, color);
  tft.drawFastHLine(LOGO_WIFI_X + 7, LOGO_WIFI_Y, 6, color);
}
