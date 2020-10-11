/*
 * Calculating_Pi.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : chaos
 */ 

//#include <avr/io.h>
#include <stdio.h>
#include <math.h>

#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"
#include "ButtonHandler.h"
#define BUTTON1SHORTPRESSEDMASK     0x01
#define BUTTON2SHORTPRESSEDMASK     0x02
#define BUTTON3SHORTPRESSEDMASK     0x04
#define BUTTON4SHORTPRESSEDMASK     0x08
#define BUTTON1LONGPRESSEDMASK      0x10
#define BUTTON2LONGPRESSEDMASK      0x20
#define BUTTON3LONGPRESSEDMASK      0x40
#define BUTTON4LONGPRESSEDMASK      0x80
#define ALGSTOPP                    0x01
#define ALG_AM_WARTEN               0x02
#define ALG_GO                      0x04
#define ALGSTOPP_2                  0x08
#define ALG_AM_WARTEN_2             0x10
#define ALG_GO_2                    0x20
#define STOPPCALC                   0x0100
#define STARTCALC                   0x0200
#define RESETCALC                   0x0400
#define STOPPCALC_2                 0x0800
#define STARTCALC_2                 0x1000
#define RESETCALC_2                 0x2000
#define LEIBNIZ                     0x01
#define KELLULAR                    0x02
#define GET_TIME                    0X01
#define WAIT_TIMER                  0X02
#define RUN_TIMER                   0X04
#define RESET_TIME                  0X08




extern void vApplicationIdleHook( void );
void vLeibniz(void *pvParameters);
void vKellalur(void *pvParameters);
void vButtonTask(void *pvParameters);
void vSteuerTask(void *pvParameters);
void vGetTime(void *pvParameters);

float Pi = 0;           //Globale Variabel für Pi, zum Zwischenspeichern
float Pi_2=0;
uint32_t Timer_ms= 0;
uint32_t IR_counter=0;




typedef enum
{
    Start,
    Stopp,
    Zurueck,
    Wechsel
} eSteuerugStates;
    


TaskHandle_t xLeibniz;
TaskHandle_t xKellalur;
TaskHandle_t xSteuerTask;
TaskHandle_t xButtonTaskHandle;
TaskHandle_t xGetTime;

EventGroupHandle_t xKommunikation;
EventGroupHandle_t xTimeKom;



void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
    
	vInitClock();
	vInitDisplay();
	
    xKommunikation=xEventGroupCreate();
    xTimeKom=xEventGroupCreate();
    
    xEventGroupSetBits(xKommunikation,STOPPCALC);
    xEventGroupSetBits(xKommunikation,STOPPCALC_2);
    
    xTaskCreate(vButtonTask, (const char *) "ButtonTask", configMINIMAL_STACK_SIZE, NULL, 2, &xButtonTaskHandle);
    xTaskCreate( vSteuerTask, (const char *) "SteuerTask", configMINIMAL_STACK_SIZE+60, NULL, 2, &xSteuerTask);
    xTaskCreate( vLeibniz, (const char *) "Leibniz", configMINIMAL_STACK_SIZE+50, NULL, 1, &xLeibniz);
    xTaskCreate(vKellalur, (const char *) "KellalurTask", configMINIMAL_STACK_SIZE+50, NULL, 1, &xKellalur);
    xTaskCreate(vGetTime, (const char *) "GetTime", configMINIMAL_STACK_SIZE+10, NULL, 2, &xGetTime);
    
	
	vTaskStartScheduler();
	return 0;
}

void vLeibniz(void *pvParameters) 
{
        (void) pvParameters;
        
        float Pi4 = 1;
        float n = 3;
        
        
        while(1)
    {     
        if (xEventGroupGetBits(xKommunikation)&STARTCALC)
        {
            Pi4-=(1/n);
            n+=2;
            Pi4+=(1/n);
            n+=2;
            if ((uint32_t)(Pi4*100000)==(uint32_t)(M_PI_4*100000))
            {
                TCC1.CTRLA=0;
            }
        }
        else if(xEventGroupGetBits(xKommunikation)&STOPPCALC)
        {
            if (xEventGroupGetBits(xKommunikation)&RESETCALC)
            {
                Pi4=1;
                n=3;
                xEventGroupClearBits(xKommunikation,RESETCALC);
            }
            vTaskDelay(20 / portTICK_RATE_MS);
        }
                 
        if(xEventGroupGetBits(xKommunikation)&ALGSTOPP)    //Wartet auf die Information
         {
             Pi=4*Pi4;
             xEventGroupSetBits(xKommunikation,ALG_AM_WARTEN);  //Sendet die Information
             xEventGroupWaitBits(xKommunikation,ALG_GO,pdTRUE,pdTRUE,5/ portTICK_RATE_MS); // Bekommt die Information zurück
             xEventGroupClearBits(xKommunikation,ALG_AM_WARTEN);
         }
         
        
	}
}

void vKellalur (void *pvParameters)
{
    (void) pvParameters;
    
    float Pi_1=3;
    float n_1=3;
    
    while(1)
    {         
        if (xEventGroupGetBits(xKommunikation)&STARTCALC_2)
        {
          Pi_1+=4/(pow(n_1,3)-n_1);
          n_1+=2;
          Pi_1-=4/(pow(n_1,3)-n_1);
          n_1+=2;
          if ((uint32_t)(Pi_1*100000)==(uint32_t)(M_PI*100000))
          {
              TCC1.CTRLA=0;
          }
        }
        else if(xEventGroupGetBits(xKommunikation)&STOPPCALC_2)
        {
            if (xEventGroupGetBits(xKommunikation)&RESETCALC_2)
            {
                Pi_1=3;
                n_1=3;
                xEventGroupClearBits(xKommunikation,RESETCALC_2);
            }
            vTaskDelay(20 / portTICK_RATE_MS);
        }          
        
        if(xEventGroupGetBits(xKommunikation)&ALGSTOPP_2)
        {
            Pi_2=Pi_1;
            xEventGroupSetBits(xKommunikation,ALG_AM_WARTEN_2);
            xEventGroupWaitBits(xKommunikation,ALG_GO_2,pdTRUE,pdTRUE,5/ portTICK_RATE_MS);
            xEventGroupClearBits(xKommunikation,ALG_AM_WARTEN_2);
            
        }
        
       
    }
    
}

void vSteuerTask(void *pvParameters)
{
    (void) pvParameters;
    
    float LocalPi;
    float LocalPi_2;
    uint32_t Buttonvalue;
    char Pi_String[10];
    char Pi_2_String[10];
    char Timer_String[10];
    uint8_t Algo =LEIBNIZ;
    uint32_t Local_ms=0;
    eSteuerugStates Steuerung = Start;
    

    
    while(1)
    {
        xTaskNotifyWait(0,0xfffffff,&Buttonvalue,1/portTICK_RATE_MS); // Warte auf eine Aufgabe oder Aufgabe wird vom blockierten Zustand entfernt.
        
        vDisplayClear();
        if (Algo==LEIBNIZ)
        {
            xEventGroupSetBits(xKommunikation,ALGSTOPP);                                         //Sendet die Infos an den LeibnizTask
            xEventGroupWaitBits(xKommunikation,ALG_AM_WARTEN,pdTRUE,pdTRUE,5/portTICK_RATE_MS); //Bekommt die Infos von LeibnizTask
            LocalPi=Pi;
            xEventGroupClearBits(xKommunikation,ALGSTOPP);      // Die Bits Infos werden gelöscht
            xEventGroupSetBits(xKommunikation,ALG_GO);          // Die Bit Info werden gesendet an Leibniz
            sprintf(Pi_String, "%f",LocalPi);                   // Local Pi wird in den String kopiert
            vDisplayWriteStringAtPos(0,0,"Pi=%s",Pi_String);    // Pi wird auf den Display ausgegeben
        }
        else if(Algo==KELLULAR)
        {
            xEventGroupSetBits(xKommunikation,ALGSTOPP_2);          //Sendet die Bit Indos an Kellular Alg. 
            xEventGroupWaitBits(xKommunikation,ALG_AM_WARTEN_2,pdTRUE,pdTRUE,5/portTICK_RATE_MS); //Bekommt die Bit Infos von Kellular
            LocalPi_2=Pi_2;
            xEventGroupClearBits(xKommunikation,ALGSTOPP_2);
            xEventGroupSetBits(xKommunikation,ALG_GO_2);
            sprintf(Pi_2_String, "%f",LocalPi_2);
            vDisplayWriteStringAtPos(1,0,"Pi_2=%s",Pi_2_String);
        }
                
       
        
        switch(Steuerung)
        {

            case Start: 
            {   
                xEventGroupSetBits(xTimeKom,GET_TIME);
                xEventGroupWaitBits(xTimeKom,WAIT_TIMER,pdTRUE,pdTRUE,5/portTICK_RATE_MS);
                Local_ms=Timer_ms;
                xEventGroupClearBits(xTimeKom,GET_TIME);
                xEventGroupSetBits(xTimeKom,RUN_TIMER);
                sprintf(Timer_String, "%lu",Local_ms);
                vDisplayWriteStringAtPos(2,0,"Timer %s",Timer_String);
                
                if (Buttonvalue&BUTTON2SHORTPRESSEDMASK)
                {
                    Steuerung=Stopp;
                }
            
                if (Buttonvalue&BUTTON3SHORTPRESSEDMASK)
                {
                    Steuerung=Zurueck;
                }
                if (Buttonvalue&BUTTON4SHORTPRESSEDMASK)
                {
                    Steuerung=Wechsel;
                }
                if (Buttonvalue&BUTTON1SHORTPRESSEDMASK)
                {
                    if (Algo==LEIBNIZ)
                    {
                        xEventGroupSetBits(xKommunikation,STARTCALC);
                        xEventGroupClearBits(xKommunikation,STOPPCALC);
                        TCC1.CTRLA|= 0b0100 << TC1_CLKSEL_gp;      
                    }
                    else if(Algo==KELLULAR)
                    {
                        xEventGroupSetBits(xKommunikation,STARTCALC_2);
                        xEventGroupClearBits(xKommunikation,STOPPCALC_2);
                        TCC1.CTRLA|= 0b0100 << TC1_CLKSEL_gp;                              
                    }            
                
                }
            break;    
            }         
        
            case Stopp:
            {
                    TCC1.CTRLA=0;
                    if (Algo==LEIBNIZ)
                    {
                        xEventGroupSetBits(xKommunikation,STOPPCALC);
                        xEventGroupClearBits(xKommunikation,STARTCALC);
                    }
                    else if (Algo==KELLULAR)
                    {
                       xEventGroupSetBits(xKommunikation,STOPPCALC_2);
                       xEventGroupClearBits(xKommunikation,STARTCALC_2);
                    }
                Steuerung=Start; 
            break;    
            }
        
            case Zurueck:
            {     
                    
                    if (Algo==LEIBNIZ)
                    {
                       if (xEventGroupGetBits(xKommunikation)&STOPPCALC)
                       {
                            xEventGroupSetBits(xKommunikation,RESETCALC);
                            
                            xEventGroupSetBits(xTimeKom,RESET_TIME);
                            xEventGroupWaitBits(xTimeKom,WAIT_TIMER,pdTRUE,pdTRUE,5/portTICK_RATE_MS);
                            xEventGroupClearBits(xTimeKom,RESET_TIME);
                            xEventGroupSetBits(xTimeKom,RUN_TIMER);
                       }
                        
                       
                    }
                    else if (Algo==KELLULAR)
                    {
                        if (xEventGroupGetBits(xKommunikation)&STOPPCALC_2)
                        {
                           xEventGroupSetBits(xKommunikation,RESETCALC_2);
                           
                           xEventGroupSetBits(xTimeKom,RESET_TIME);
                           xEventGroupWaitBits(xTimeKom,WAIT_TIMER,pdTRUE,pdTRUE,5/portTICK_RATE_MS);
                           xEventGroupClearBits(xTimeKom,RESET_TIME);
                           xEventGroupSetBits(xTimeKom,RUN_TIMER);
                        }           
                    }
                Steuerung=Start;
            
            break;    
            }            
        
            case Wechsel:
            {
              TCC1.CTRLA=0;
              xEventGroupSetBits(xTimeKom,RESET_TIME);
              xEventGroupWaitBits(xTimeKom,WAIT_TIMER,pdTRUE,pdTRUE,5/portTICK_RATE_MS);
              xEventGroupClearBits(xTimeKom,RESET_TIME);
              xEventGroupSetBits(xTimeKom,RUN_TIMER);
              
              if (Algo==LEIBNIZ)
              {
                  xEventGroupSetBits(xKommunikation,STOPPCALC);
                  xEventGroupClearBits(xKommunikation,STARTCALC);
                  Algo=KELLULAR;
              }
              else
              {
                  xEventGroupSetBits(xKommunikation,STOPPCALC_2);
                  xEventGroupClearBits(xKommunikation,STARTCALC_2);
                  Algo=LEIBNIZ;
              }              

            Steuerung=Start;
            break;    
            }       
            default:
            {
                Steuerung=Start;
                break;
            }
        }             
        vTaskDelay(500 / portTICK_RATE_MS);
                      
    }
        
    
}

ISR( TCC1_OVF_vect)
{
    BaseType_t Priority = pdFALSE;
    xTaskNotifyFromISR( xGetTime,IR_counter, eIncrement, &Priority );

}

void vGetTime(void *pvParameters) 
{
    (void) pvParameters;
    uint32_t NotificationValue;
    uint32_t TimerCnt= 0;
    
    
    TCC1.INTCTRLA|= 3 << TC1_OVFINTLVL_gp;
    TCC1.PER= 3999;
    PMIC.CTRL|= 1 << PMIC_HILVLEN_bp;
    
    for(;;) 
    {
        xTaskNotifyWait(0,0xffffffff,&NotificationValue, pdMS_TO_TICKS(0));
        
        if(NotificationValue>=1)
        {
            TimerCnt+=NotificationValue;
            NotificationValue=0;
            
        }
        if (xEventGroupWaitBits(xTimeKom,RESET_TIME|GET_TIME,pdFALSE,pdFALSE,pdMS_TO_TICKS(100))) //Bekommt die Information
        {
            if (xEventGroupGetBits(xTimeKom)&RESET_TIME)//Wartet auf die Information
            {
                IR_counter=0;
                TimerCnt=0;
                xEventGroupSetBits(xTimeKom,WAIT_TIMER);  //Sendet die Information
                xEventGroupWaitBits(xTimeKom,RUN_TIMER,pdTRUE,pdTRUE,5/ portTICK_RATE_MS); // Bekommt die Information zurück
                xEventGroupClearBits(xTimeKom,WAIT_TIMER);
            }
             
            if(xEventGroupGetBits(xTimeKom)&GET_TIME)    //Wartet auf die Information
            {
                Timer_ms=TimerCnt;
                xEventGroupSetBits(xTimeKom,WAIT_TIMER);  //Sendet die Information
                xEventGroupWaitBits(xTimeKom,RUN_TIMER,pdTRUE,pdTRUE,5/ portTICK_RATE_MS); // Bekommt die Information zurück
                xEventGroupClearBits(xTimeKom,WAIT_TIMER);
            }
        }
    }
}

void vButtonTask(void *pvParameters) {
    initButtons();

    for(;;) {
        updateButtons();
        
        if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON1SHORTPRESSEDMASK,eSetValueWithOverwrite);
            
            
        }
        if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON2SHORTPRESSEDMASK,eSetValueWithOverwrite);
        }
        if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON3SHORTPRESSEDMASK,eSetValueWithOverwrite);
            
        }
        if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON4SHORTPRESSEDMASK,eSetValueWithOverwrite);
            
        }

        vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
    }

}