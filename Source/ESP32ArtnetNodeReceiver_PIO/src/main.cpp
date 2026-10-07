#include <Arduino.h>

#include "globals.h"
#include "statusLed.h"
#include "oled.h"
#include "userLogic.h"


void StateMode_Changed(eStateMachineMode newMode)
{

}
    
 void AlternateMode_Changed(eStateMachineMode currMode, bool isAlternateMode)
 {


 }


void setup() 
{
	//Init Default VCP
	Serial.begin(115200);

	//Init Level Shifter EN pin and set it to HIGH (no lvl_Shifter management fo now)
  	pinMode(LVL_SHIFTER_PIN, OUTPUT);
  	digitalWrite(LVL_SHIFTER_PIN, LOW);

	//Init Status LED
	initStatusLED();
	setStatusLED(255, 255, 0);   // Yellow = initialising

	//Init OLED Display
	initOledDisplay();
	displayStatus("Initializing...");

	//Load Setting From intern memory
	loadSettings();

	// Enforce LED count limits
 	int maxLeds = getMaxLedsPerStrip(); //Max led per outputs depending of number per 
  	if (settings.fullFrameMode.numLedsPerOutput > maxLeds) 
	{
    	settings.fullFrameMode.numLedsPerOutput = maxLeds;
    	saveSettings();
  	}

	reconfigureNeoPixelStrips();
	
	updateStripPins();

	initOutputDriver();

	userLogic_init((eStateMachineMode)settings.device.startStateMode,StateMode_Changed,AlternateMode_Changed);



}

void loop() {
  // put your main code here, to run repeatedly:
}