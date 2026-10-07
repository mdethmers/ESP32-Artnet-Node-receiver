#include "globals.h"
#include <Preferences.h>
#include "settings.h"
#include "outputsDriver.h"


Settings    settings;
Preferences preferences;

// ─── Load / save settings ─────────────────────────────────────────────────────
void loadSettings() {
    preferences.begin("esp32config", true);

    settings.device.ledType                   = preferences.getInt("ledType",         settings.device.ledType);
    settings.device.protocolMode              = preferences.getInt("protocolMode",    settings.device.protocolMode);
    settings.device.screenRotation            = preferences.getBool("screenRotation", settings.device.screenRotation);
    settings.device.numOutputs                = preferences.getInt("deviceNumoutput", settings.device.numOutputs);
    settings.device.nodeName                  = preferences.getString("nodename",     settings.device.nodeName);

    settings.fullFrameMode.numLedsPerOutput   = preferences.getInt("numledsoutput",   settings.fullFrameMode.numLedsPerOutput);    
    settings.fullFrameMode.startUniverse      = preferences.getInt("startuniverse",   settings.fullFrameMode.startUniverse);
    settings.fullFrameMode.ledBrightness      = preferences.getInt("ledbrightness",   settings.fullFrameMode.ledBrightness);
    settings.fullFrameMode.whiteLevel         = preferences.getInt("whiteLevel",      settings.fullFrameMode.whiteLevel);
    settings.fullFrameMode.white2Level        = preferences.getInt("white2Level",     settings.fullFrameMode.white2Level);

    settings.network.ssid                     = preferences.getString("ssid",         settings.network.ssid);
    settings.network.password                 = preferences.getString("password",     settings.network.password);
    settings.network.useWiFi                  = preferences.getBool("useWiFi",        settings.network.useWiFi);
    settings.misc.enableCustomPins            = preferences.getBool("enCustomPins",   false);
    settings.misc.ledCycleEnabled             = preferences.getBool("ledCycleEnabled",false);
    settings.misc.randomFadeEnabled           = preferences.getBool("randFadeEn",     false);   // NEW
    settings.network.useStaticIP              = preferences.getBool("useStaticIP",    settings.network.useStaticIP);
    settings.network.staticIP                 = preferences.getString("staticIP",     settings.network.staticIP);
    settings.network.gateway                  = preferences.getString("gateway",      settings.network.gateway);
    settings.network.subnet                   = preferences.getString("subnet",       settings.network.subnet);
    settings.device.startStateMode            = preferences.getInt("startStateMode",  -1);


    settings.misc.staticColor                 = preferences.getString("staticColor",  settings.misc.staticColor);
    settings.misc.enStatColor                 = preferences.getBool("enStatColor",    settings.misc.enStatColor);
    

    // Absent on a node upgraded from older firmware, which is exactly the
    // ungrouped behaviour it had before.
    settings.fullFrameMode.groupSize          = preferences.getInt("groupSize",       1);
    if (settings.fullFrameMode.groupSize < 1)              settings.fullFrameMode.groupSize = 1;
    if (settings.fullFrameMode.groupSize > MAX_GROUP_SIZE) settings.fullFrameMode.groupSize = MAX_GROUP_SIZE;


    // ── channelMode migration ─────────────────────────────────────────────────
    if (preferences.isKey("channelMode")) {
        settings.fullFrameMode.channelMode = preferences.getInt("channelMode", CHANNEL_MODE_RGB);
    } else {
        bool legacyRGBW = preferences.getBool("useRGBW", false);
        settings.fullFrameMode.channelMode = legacyRGBW ? CHANNEL_MODE_RGBW : CHANNEL_MODE_RGB;
    }

    if (settings.device.startStateMode == -1)
        settings.device.startStateMode = settings.network.useWiFi ? 1 : 0;

    for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) {
        char key[10]; sprintf(key, "pin%d", i);
        settings.device.pins[i] = preferences.getInt(key, DEFAULT_PINS[i]);
    }
    if (!settings.misc.enableCustomPins) {
        for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) settings.device.pins[i] = DEFAULT_PINS[i];
    }
    preferences.end();
}

void saveSettings() {
  preferences.begin("esp32config", false);
  preferences.putInt("numledsoutput",   settings.fullFrameMode.numLedsPerOutput);
  preferences.putInt("numoutput",       settings.device.numOutputs);
  preferences.putInt("startuniverse",   settings.fullFrameMode.startUniverse);
  preferences.putString("nodename",     settings.device.nodeName);
  preferences.putInt("ledbrightness",   settings.fullFrameMode.ledBrightness);
  preferences.putString("ssid",         settings.network.ssid);
  preferences.putString("password",     settings.network.password);
  preferences.putBool("useWiFi",        settings.network.useWiFi);
  preferences.putString("staticColor",  settings.misc.staticColor);
  preferences.putBool("enStatColor",    settings.misc.enStatColor);
  preferences.putBool("enCustomPins",   settings.misc.enableCustomPins);
  preferences.putBool("ledCycleEnabled",settings.misc.ledCycleEnabled);
  preferences.putBool("randFadeEn",     settings.misc.randomFadeEnabled);   // NEW
  preferences.putBool("useStaticIP",    settings.network.useStaticIP);
  preferences.putString("staticIP",     settings.network.staticIP);
  preferences.putString("gateway",      settings.network.gateway);
  preferences.putString("subnet",       settings.network.subnet);
  preferences.putInt("startStateMode",     settings.device.startStateMode);
  preferences.putInt("channelMode",     settings.fullFrameMode.channelMode);
  preferences.putInt("whiteLevel",      settings.fullFrameMode.whiteLevel);
  preferences.putInt("white2Level",     settings.fullFrameMode.white2Level);
  preferences.putInt("ledType",         settings.device.ledType);
  preferences.putInt("protocolMode",    settings.device.protocolMode);
  preferences.putInt("groupSize",       settings.fullFrameMode.groupSize);
  preferences.putBool("screenRotation", settings.device.screenRotation);

  for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) {
    char key[10]; sprintf(key, "pin%d", i);
    preferences.putInt(key, settings.device.pins[i]);
  }
  preferences.end();
}

void resetToDefaultPins() {   
  for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) settings.device.pins[i] = DEFAULT_PINS[i];
}


// What the console actually addresses on one output. A strip whose length is
// not a multiple of the group size ends in a short group rather than losing
// its tail — the last fixture simply drives fewer LEDs.
int getFixturesPerOutput() {
  int g = getGroupSize();
  return (settings.fullFrameMode.numLedsPerOutput + g - 1) / g;
}

int getGroupSize() {
  int g = settings.fullFrameMode.groupSize;
  if (g < 1)              g = 1;
  if (g > MAX_GROUP_SIZE) g = MAX_GROUP_SIZE;
  return g;
}


// ─── Helper accessors ─────────────────────────────────────────────────────────
//Return the Max number of LEDs per physical outputs in worst case scenario (4 universes per outputs)
int getMaxLedsPerStrip() {
  switch (settings.fullFrameMode.channelMode) {
    case CHANNEL_MODE_RGBWW: return MAX_LEDS_RGBWW;
    case CHANNEL_MODE_RGBW:  return MAX_LEDS_RGBW;
    default:                 return MAX_LEDS_RGB;
  }
}

int getChannelsPerLed() {
  switch (settings.fullFrameMode.channelMode) {
    case CHANNEL_MODE_RGBWW: return 5;
    case CHANNEL_MODE_RGBW:  return 4;
    default:                 return 3;
  }
}

int getLedsPerUniverse() {
  // How many fixtures fit in one 512-byte ArtNet universe. Without grouping a
  // fixture is a single LED, which is why this reads as "LEDs per universe"
  // everywhere it is used.
  return UNIVERSE_SIZE_IN_CHANNEL / getChannelsPerLed();
}



// Outputs always begin on a universe boundary, so an output that does not fill
// its last universe still reserves the whole thing and the next output starts
// at the following universe number. That keeps the patch stable: changing the
// group size never shifts the outputs after it.
int getUniversesPerOutput() {
  int per = getLedsPerUniverse();
  int u   = (getFixturesPerOutput() + per - 1) / per;
  if (u < 1) u = 1;
  if (u > 4) u = 4;
  return u;
}