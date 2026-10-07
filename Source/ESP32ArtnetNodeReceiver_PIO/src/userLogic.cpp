#include "globals.h"
#include <GPIO_in.h>


#include "userLogic.h"

#define SHORT_PRESS_DURATION_MS  500
#define LONG_PRESS_DURATION_MS  1500


GPIO_in gpioUserBtn = {0};
eStateMachineMode currentStateMode = State_Mode_UNKNOWN;
uint32_t currModeEnteringTick = 0;
bool alternateMode = false;

void (*onStateModeChanged)(eStateMachineMode);
void (*onAlternateModeChanged)(eStateMachineMode, bool);

static uint32_t btnPressedTick = 0;
static uint32_t btnReleasedTick = 0;



void onUserButtonPressed(const GPIO_in * const pGpio)
{
    btnPressedTick = millis();
}


void onUserButtonReleased(const GPIO_in * const pGpio)
{
    btnReleasedTick = millis();
}



void userLogic_init(eStateMachineMode startMode,
                    void (*ondStateModeChanged)(eStateMachineMode),
                    void (*ondAlternateModeChanged)(eStateMachineMode, bool)

)
{
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    onStateModeChanged = ondStateModeChanged;
    onAlternateModeChanged = ondAlternateModeChanged;


    currentStateMode = startMode;
    if(onStateModeChanged != NULL)
            onStateModeChanged(currentStateMode);

    GPIO_in_init(&gpioUserBtn,BOOT_BUTTON_PIN,GPIO_PIN_SET,50, onUserButtonPressed, onUserButtonReleased,false);

}


void userLogic_loop(void)
{
    GPIO_in_handler(&gpioUserBtn);

    if(btnReleasedTick - btnPressedTick >= LONG_PRESS_DURATION_MS)
    {
        alternateMode = !alternateMode;
        if(onAlternateModeChanged != NULL)
            onAlternateModeChanged(currentStateMode, alternateMode);
    }
    else if(btnReleasedTick - btnPressedTick >= SHORT_PRESS_DURATION_MS)
    {
        currentStateMode = (eStateMachineMode)(((int)currentStateMode)+1);
        if(currentStateMode >= State_Mode_Max)
            currentStateMode = (eStateMachineMode)0;

        if(onStateModeChanged != NULL)
            onStateModeChanged(currentStateMode);
    }
}

eStateMachineMode getCurrentMode(void){    return currentStateMode;    }
