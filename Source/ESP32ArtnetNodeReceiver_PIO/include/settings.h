#ifndef     __SETTINGS_H
#define __SETTINGS_H

#include "globals.h"
#include <Preferences.h>
#include "outputsDriver.h"

enum eDeviceOutputsMode
{
	FULL_FRAME_OUTPUTS_MODE,
	INDEPENDANT_OUTPUTS_MODE,
};

enum eIndependantOutDriveMode
{
	INDEP_OUT_LEDS_STRIP_MODE,
	INDEP_OUT_GPIO_MODE,
};

struct sIndependantOutput
{
	bool 						enabled 		= true;
	eIndependantOutDriveMode	driveMode 		= INDEP_OUT_LEDS_STRIP_MODE;
	//Drived Outputs ==> for now assume that binding is done using Idx
	int 						channelMode     = CHANNEL_MODE_RGB; // 0=RGB, 1=RGBW, 2=RGBWW
	int 						numLEDs			= NUM_LEDS_PER_STRIP;
	int 						startUniverse	= 0;
	uint16_t 					gpioModeDmxChannel = 1;
	uint8_t						gpioModeDmxLvlThreshold = 0x80;
	//number of universe?
};


// ─── Settings ─────────────────────────────────────────────────────────────────
struct Settings {
	struct sDevice
  	{
		String  nodeName                      = "Artnet Node ESP32";
		int     numOutputs                    = GPIO_OUTPUTS_NBR;
    	int     pins[GPIO_OUTPUTS_NBR]	      = {GPIO_STRIPS_OUTPUT_1,GPIO_STRIPS_OUTPUT_2,GPIO_STRIPS_OUTPUT_3,GPIO_STRIPS_OUTPUT_4};
    	int     ledType                       = LED_TYPE_WS2812; // 0=WS2812, 1=APA102
  	
		// Art-Net only by default. Listening for both protocols costs a second
  		// socket and a second parse of every frame — overhead a node earns only if
		// the console actually sends sACN.
		int     protocolMode                  = DEFAULT_PROTOCOL;  // 0=ArtNet, 1=sACN, 2=Both
	
		bool    screenRotation                = false;

		int     startStateMode                = 0;   // 0=Ethernet 1=WiFi 2=AP 3=RGB-test 4=Static 5=Info

		eDeviceOutputsMode deviceOutputsMode = FULL_FRAME_OUTPUTS_MODE;
	}device;

	struct sFullFrameMode
	{
		int     numLedsPerOutput              = NUM_LEDS_PER_STRIP;
 		int     startUniverse                 = 0;
 		int     ledBrightness                 = 255;
		int     channelMode                   = CHANNEL_MODE_RGB; // 0=RGB, 1=RGBW, 2=RGBWW
  		// How many physical LEDs act as one addressable fixture. 1 = every LED is its
		// own fixture (the original behaviour). Higher values cut the patch by the
		// same factor: the console addresses ceil(leds / groupSize) fixtures and the
		// node fans each one out across its LEDs.
		int     groupSize                     = 1;
		int     whiteLevel                    = 0;
		int     white2Level                   = 0;   // second white channel for RGBWW static colour

		// Convenience helpers -------------------------------------------------------
		bool isRGBW()   const { return channelMode == CHANNEL_MODE_RGBW;  }
		bool isRGBWW()  const { return channelMode == CHANNEL_MODE_RGBWW; }
		// Deprecated accessor kept so existing call-sites still compile
		bool useRGBW_compat() const { return channelMode != CHANNEL_MODE_RGB; }
	}fullFrameMode;

	sIndependantOutput independantOutputsMode [GPIO_OUTPUTS_NBR];

	struct sNetwork
	{
		String  ssid                          = "";
  		String  password                      = "";
  		bool    useWiFi                       = false;
		bool    useStaticIP                   = false;
		String  staticIP                      = "192.168.1.100";
		String  gateway                       = "192.168.1.1";
		String  subnet                        = "255.255.255.0";
	}network;

	struct sMisc
	{
		String  staticColor                   = "FF8000";
		bool    enStatColor                   = false;
		bool    enableCustomPins              = false;
		bool    ledCycleEnabled               = false;
		bool    randomFadeEnabled             = false;   // NEW: smooth random colour fade
	}misc;
};

extern Settings settings;
extern Preferences preferences;




void loadSettings();
void saveSettings();
void resetToDefaultPins();


int getMaxLedsPerStrip();
int getChannelsPerLed();
int getFixturesPerOutput();
int getGroupSize();
int getLedsPerUniverse();
int getUniversesPerOutput();


#endif