#include <msp430.h>
#include "driverlib\MSP430FR2xx_4xx\driverlib.h"
#include "hal_LCD.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>


//////////////////// Sensors ////////////////////

int sens_temperature;
int sens_turbidity;
bool sens_water_low;

int sens_feed_timer0;
int sens_feed_timer1;

void Sens_Init(void);
void Sens_Check_Feeding0(void);
void Sens_Check_Feeding1(void);
void Sens_Check_Temperature(void);
void Sens_Check_Water_Level(void);


//////////////////// UI ////////////////////

// 0 = main
// 1 = set temperature
// 2 = set feeding time
// 3 = set feeding time 2
int ui_current_menu;
int ui_reset_timer;
bool ui_first_time_on_this_menu;

void UI_Init(void);
void UI_Next(void);
void UI_Up(void);
void UI_Down(void);

volatile int setting_temperature;
volatile int setting_feed0;
volatile int setting_feed1;
volatile int setting_filter;


//////////////////// Display ////////////////////

void Display_Main(void);
void Display_Set_Temp(void);
void Display_Set_Feed0();
void Display_Set_Feed1();
void Display_Set_Filter(void);


//////////////////// PWM ////////////////////

void PWM_Init(void);
void PWM_Start(void);
void PWM_Stop(void);
void PWM_Set_Duty_Cycle(float percentage);


//////////////////// MISC ////////////////////

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}









void Display_Main(void)
{
    int temperature_celcius = (sens_temperature-0.5)*100;
    char temp[3];
    sprintf(temp, "%d", temperature_celcius);
    char* display_string = concat("TEMPERATURE ", temp);
//    displayScrollText(display_string, 1);
//
//    if(sens_water_low)
//    {
//        displayScrollText("XX WATER LEVEL LOW XX", 1);
//    }
    displayText("  TEST");

}
void Display_Set_Temp(void)
{
    if(ui_first_time_on_this_menu)
    {
        __bic_SR_register(GIE);
        displayScrollText("SET TEMPERATURE", 0);
        __bis_SR_register(GIE);
    }
    ui_first_time_on_this_menu = false;

    char temp[4];
    sprintf(temp, "%d", setting_temperature);
    displayText(temp);
}
void Display_Set_Feed0()
{
    if(ui_first_time_on_this_menu)
    {
        __bic_SR_register(GIE);
        displayScrollText("SET FEEDING CYCLE 0", 0);
        __bis_SR_register(GIE);
    }
    ui_first_time_on_this_menu = false;

    char temp[4];
    sprintf(temp, "%d", setting_feed0);
    displayText(temp);
}
void Display_Set_Feed1()
{
    if(ui_first_time_on_this_menu)
    {
        __bic_SR_register(GIE);
        displayScrollText("SET FEEDING CYCLE 1", 0);
        __bis_SR_register(GIE);
    }
    ui_first_time_on_this_menu = false;

    char temp[4];
    sprintf(temp, "%d", setting_feed1);
    displayText(temp);
}









// Init PWM on pin P1.7
// starts on 50% duty cycle
void PWM_Init()
{
    unsigned int period = 3000;
    unsigned int duty_cycle = 1500;

    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION
        );

    //Start Counter: counter is used to generate the pwm and will always be counting
    Timer_A_initUpModeParam initUpParam = {0};
    initUpParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initUpParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpParam.timerPeriod = period;
    initUpParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    initUpParam.timerClear = TIMER_A_DO_CLEAR;
    initUpParam.startTimer = false;
    Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam);

    //start compare registers to generate PWM
    Timer_A_initCompareModeParam initComp2Param = {0};
    initComp2Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initComp2Param.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initComp2Param.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
    initComp2Param.compareValue = duty_cycle;
    Timer_A_initCompareMode(TIMER_A0_BASE, &initComp2Param);


//////////////////////TEMPORARY/////////////////////////
//    GPIO_setAsPeripheralModuleFunctionOutputPin(
//        GPIO_PORT_P4,
//        GPIO_PIN0,
//        GPIO_PRIMARY_MODULE_FUNCTION
//        );
//
//    Timer_A_initUpModeParam initUpParam2 = {0};
//    initUpParam2.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
//    initUpParam2.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
//    initUpParam2.timerPeriod = period;
//    initUpParam2.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
//    initUpParam2.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
//    initUpParam2.timerClear = TIMER_A_DO_CLEAR;
//    initUpParam2.startTimer = false;
//    Timer_A_initUpMode(TIMER_A1_BASE, &initUpParam2);
//
//    //start compare registers to generate PWM
//    Timer_A_initCompareModeParam initComp2Param2 = {0};
//    initComp2Param2.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
//    initComp2Param2.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
//    initComp2Param2.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
//    initComp2Param2.compareValue = duty_cycle;
//    Timer_A_initCompareMode(TIMER_A1_BASE, &initComp2Param2);
}
void PWM_Start(void)
{
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}
void PWM_Stop(void)
{
    Timer_A_stop(TIMER_A0_BASE);
}
void PWM_Set_Duty_Cycle(float percentage)
{
    unsigned int compare_value = 3000 * (1-percentage);
    HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = compare_value;
}








void Sens_Init(void)
{
    //SETUP WATER LEVEL DETECTION
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN7);

    //SETUP RELAYS
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN6); //Relay 1
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0); //Relay 2

    //SETUP FEEDING COUNTER
    //Start Counter: counter is used to track time
    Timer_A_initContinuousModeParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    param.timerClear = TIMER_A_DO_CLEAR;
    param.startTimer = true;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &param);

}


#pragma vector=TIMER1_A1_VECTOR
__interrupt
void TIMER1_A1_ISR (void)
{
    //Any access, read or write, of the TAIV register automatically resets the
    //highest "pending" interrupt flag
    switch ( __even_in_range(TA1IV,14) ){
        case  0: break;                          //No interrupt
        case  2: break;                          //CCR1 not used
        case  4: break;                          //CCR2 not used
        case  6: break;                          //CCR3 not used
        case  8: break;                          //CCR4 not used
        case 10: break;                          //CCR5 not used
        case 12: break;                          //CCR6 not used
        case 14:
            //Toggle P1.0                    // overflow
            sens_feed_timer0 ++;
            sens_feed_timer1 ++;
            break;
        default: break;
    }
}

void Sens_Check_Feeding0(void)
{
    if(setting_feed0 < sens_feed_timer0*1.6384 && 0 < setting_feed0)
    {
        int i = 0;
        bool toggle = false;

        PWM_Start();
        while(i < 7)
        {
            __delay_cycles(500000);
            int compare_value = toggle? 500 : 2500;
            HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = compare_value;
            toggle = !toggle;
            i++;
        }
        HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = 3001;
        __delay_cycles(500000);
        PWM_Stop();
        HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = 500;

        sens_feed_timer0 = 0;
    }
}

void Sens_Check_Feeding1(void)
{
    if(setting_feed1 < sens_feed_timer1*1.6384 && 0 < setting_feed1)
    {
        int i = 0;
        bool toggle = false;

        PWM_Start();
        while(i < 9)
        {
            __delay_cycles(500000);
            int compare_value = toggle? 500 : 2500;
            HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = compare_value;
            toggle = !toggle;
            i++;
        }
        HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = 2500;
        PWM_Stop();
        HWREG16(TIMER_A0_BASE + TIMER_A_CAPTURECOMPARE_REGISTER_1 + OFS_TAxR) = 500;

        sens_feed_timer1 = 0;
    }
}

void Sens_Check_Temperature(void)
{
    int temperature_celcius = (sens_temperature-0.5)*100;

    if (temperature_celcius < (setting_temperature - 2))
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6);
    }
    if((setting_temperature + 2) < temperature_celcius)
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6);
    }
}

void Sens_Check_Water_Level(void)
{
    if(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_HIGH)
    {
        sens_water_low = true;
    }
    else
    {
        sens_water_low = false;
    }
}








//set up push buttons with interrupts
void UI_Init(void)
{
    GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5);
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN_ALL8);

    PMM_unlockLPM5();           //disable default high impedance
    __bis_SR_register(GIE);     //enable interrupts
}
void UI_Reset(void)
{
    if(ui_current_menu != 0){
        ui_reset_timer++;
        if(5000 < ui_reset_timer)
        {
            ui_reset_timer = 0;
            ui_current_menu = 0;
        }
    }
}
//Push button ISR
#pragma vector=PORT1_VECTOR
__interrupt
void P1_ISR (void)
{
    if(GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN3) == GPIO_PIN3)
    {
        UI_Up();
        GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    }

    else if(GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN4) == GPIO_PIN4)
    {
        UI_Down();
        GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN4);
    }

    else if(GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN5) == GPIO_PIN5)
    {
        UI_Next();
        GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
    }
    ui_reset_timer = 0;
}




void UI_Up(void)
{
    __delay_cycles(1000); //debounce
    if(GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW)
    {
        switch (ui_current_menu)
        {
            case 0 :
                break;
            case 1 :
                setting_temperature ++;
                if(40 < setting_temperature)
                {
                    setting_temperature = 40;
                }
                break;
            case 2 :
                setting_feed0 ++;
                if(24 < setting_feed0)
                {
                    setting_feed0 = 0;
                }
                break;
            case 3 :
                setting_feed1 ++;
                if(24 < setting_feed1)
                {
                    setting_feed1 = 0;
                }
                break;
            default :
                break;
        }
    }
}
void UI_Down(void)
{
    __delay_cycles(1000); //debounce
    if(GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4) == GPIO_INPUT_PIN_LOW)
    {
        switch (ui_current_menu)
        {
            case 0 :
                break;
            case 1 :
                setting_temperature --;
                if(setting_temperature < 0)
                {
                    setting_temperature = 0;
                }
                break;
            case 2 :
                setting_feed0 --;
                if(setting_feed0 < 0)
                {
                    setting_feed0 = 24;
                }
                break;
            case 3 :
                setting_feed1 --;
                if(setting_feed1 < 0)
                {
                    setting_feed1 = 24;
                }
                break;
            default :
                break;
        }
    }
}
void UI_Next(void)
{
    __delay_cycles(1000); //debounce
    if(GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_LOW)
    {
        ui_current_menu++;
        if(3 < ui_current_menu)
        {
            ui_current_menu = 0;
        }
        ui_first_time_on_this_menu = true;
    }
}








char ADCState = 0;
int16_t ADCResult = 0;

//Input to ADC - in this case input A0 maps to pin P1.0
#define ADC_IN_PORT     GPIO_PORT_P1
#define ADC_IN_PIN      GPIO_PIN0
#define ADC_IN_CHANNEL  ADC_INPUT_A0

//ADD to main.c
void Init_ADC(void)
{
    /*
     * To use the ADC, you need to tell a physical pin to be an analog input instead
     * of a GPIO, then you need to tell the ADC to use that analog input. Defined
     * these in main.h for A9 on P8.1.
     */

    //Set ADC_IN to input direction
    GPIO_setAsPeripheralModuleFunctionInputPin(ADC_IN_PORT, ADC_IN_PIN, GPIO_PRIMARY_MODULE_FUNCTION);

    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use internal ADC bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC_init(ADC_BASE,
             ADC_SAMPLEHOLDSOURCE_SC,
             ADC_CLOCKSOURCE_ADCOSC,
             ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    /*
     * Base Address for the ADC Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC_setupSamplingTimer(ADC_BASE,
                           ADC_CYCLEHOLD_1024_CYCLES,
                           ADC_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input ADC_IN_CHANNEL
     * Use positive reference of AVcc
     * Use negative reference of AVss
     */
    ADC_configureMemory(ADC_BASE,
                        ADC_IN_CHANNEL,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    ADC_clearInterrupt(ADC_BASE,
                       ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
                        ADC_COMPLETED_INTERRUPT);
}

//ADC interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt
void ADC_ISR(void)
{
    uint8_t ADCStatus = ADC_getInterruptStatus(ADC_BASE, ADC_COMPLETED_INTERRUPT_FLAG);

    ADC_clearInterrupt(ADC_BASE, ADCStatus);

    if (ADCStatus)
    {
        ADCState = 0; //Not busy anymore
        sens_temperature = ADC_getResults(ADC_BASE);
    }
}



