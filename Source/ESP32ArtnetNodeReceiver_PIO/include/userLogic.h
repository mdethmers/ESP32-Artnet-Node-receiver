#ifndef __USER_LOGIC_H
#define __USER_LOGIC_H

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum
{
    Ethernet_Connexion_Mode,
    Wifi_Connexion_Mode,
    AP_Connexion_Mode,
    RGB_Test_Mode,
    Fixed_Color_Mode,
    SHow_About_Mode,
    State_Mode_Max,
    
    State_Mode_UNKNOWN = 0xFF,
}eStateMachineMode;


eStateMachineMode getCurrentMode(void);

void userLogic_init(eStateMachineMode startMode,
                    void (*ondStateModeChanged)(eStateMachineMode),
                    void (*ondAlternateModeChanged)(eStateMachineMode, bool));

void userLogic_loop(void);




#ifdef __cplusplus
}
#endif

#endif