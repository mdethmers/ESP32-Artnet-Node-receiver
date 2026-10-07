#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "stdint.h"

// --- Feature flags -----------------------------------------------------------
#define DEBUG_ETHERNET_WEBSERVER_PORT Serial
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 1
#define UNIQUE_SUBARTNET
#define DEFAULT_PROTOCOL            0  // 0=ArtNet, 1=sACN, 2=Both
                         
// --- LED configuration --------------------------------------------------------
#define MAX_LEDS_RGB                680             // 170 LEDs × 4 universes  (3 ch)
#define MAX_LEDS_RGBW               512             // 128 LEDs × 4 universes  (4 ch)
#define MAX_LEDS_RGBWW              408             // 102 LEDs × 4 universes  (5 ch)  floor(512/5)*4
#define NUM_LEDS_PER_STRIP          MAX_LEDS_RGB
#define UNIVERSE_SIZE_IN_CHANNEL    512
#define MAX_GROUP_SIZE              16              // largest number of LEDs one fixture may drive
#define COLOR_GRB


// --- Hardware Config ---------------------------------------------------------
#define GPIO_OUTPUTS_NBR            4               // NUMSTRIPS
#define GPIO_STRIPS_OUTPUT_1        12
#define GPIO_STRIPS_OUTPUT_2        14
#define GPIO_STRIPS_OUTPUT_3        27
#define GPIO_STRIPS_OUTPUT_4        26
#define LVL_SHIFTER_PIN             25              //ShifterPin


#define APA102_CLOCK_PIN            17              // APA102 clock pin — MUST be > 16 on ESP32
#define STATUS_LED_PIN              16
#define BOOT_BUTTON_PIN             0

#define CYCLE_DELAY                 1000
#define FADE_TICK_MS                20              // ~50 Hz || Fade tick interval — lower = smoother/faster update rate



#endif