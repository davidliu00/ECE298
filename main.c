#include "main.h"
#include "hal_LCD.h"

void Setup(void)
{
    WDT_A_hold(WDT_A_BASE);     //stop watchdog timer

    ui_first_time_on_this_menu = true;

    sens_temperature = 0;
    sens_turbidity = 0;
    sens_water_low = false;

    setting_temperature = 21;
    setting_feed0 = 0;
    setting_feed1 = 0;
    setting_filter = 0;


    //Sensor initialization
    Sens_Init();

   //PWM initialization
    PWM_Init();
    PWM_Set_Duty_Cycle(0.1);

    //UI initialization
    UI_Init();
    ui_current_menu = 0;
    ui_reset_timer = 0;

    //LCD initialization
    Init_LCD();

    Init_ADC();

    PMM_unlockLPM5();
}

int main(void)
{
    Setup();

	for(;;)
	{

	    __delay_cycles(30);
	    switch (ui_current_menu)
	    {
	        case 0 : //main menu; display measurements
	            Display_Main();
	            Sens_Check_Feeding0();
	            Sens_Check_Feeding1();
	            Sens_Check_Temperature();
	            Sens_Check_Water_Level();
	            break;
	        case 1 : //
	            Display_Set_Temp();
	            break;
	        case 2 :
	            Display_Set_Feed0();
	            break;
	        case 3 :
	            Display_Set_Feed1();
	            break;
	        default :
	            break;
	    }

	    UI_Reset();
	}
}














