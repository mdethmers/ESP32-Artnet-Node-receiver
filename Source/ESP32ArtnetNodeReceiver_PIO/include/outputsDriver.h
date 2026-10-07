#ifndef __OUTPUTS_DRIVER_H
#define __OUTPUTS_DRIVER_H

#include "globals.h"

// ─── LED type enum ────────────────────────────────────────────────────────────
enum LedType {
  LED_TYPE_WS2812 = 0,
  LED_TYPE_APA102 = 1
};

// ─── Channel mode enum ────────────────────────────────────────────────────────
enum ChannelMode {
  CHANNEL_MODE_RGB   = 0,   // 3 channels — WS2812 or APA102
  CHANNEL_MODE_RGBW  = 1,   // 4 channels — WS2812 only
  CHANNEL_MODE_RGBWW = 2    // 5 channels — WS2812 only (RGBCCT / RGBWW)
};

// ─── Default pins ─────────────────────────────────────────────────────────────
extern const int DEFAULT_PINS[GPIO_OUTPUTS_NBR];


void updateStripPins(void);

void reconfigureNeoPixelStrips(void);

void initNeoPixelStrips(void);

void initOutputDriver(void);




#endif //__OUTPUTS_DRIVER_H