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
#define ALG_GO                       0x04
#define ALGSTOPP_2                  0x08
#define ALG_AM_WARTEN_2             0x10
#define ALG_GO_2



extern void vApplicationIdleHook( void );
void vLeibniz(void *pvParameters);
void vKellalur(void *pvParameters);
void vButtonTask(void *pvParameters);
void vSteuerTask(void *pvParameters);

float Pi = 0;
float Pi_2=0;


TaskHandle_t xLeibniz;
TaskHandle_t xKellalur;
TaskHandle_t xSteuerTask;
TaskHandle_t xButtonTaskHandle;

EventGroupHandle_t xKommunikation;



void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
    resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
    xTaskCreate(vButtonTask, (const char *) "ButtonTask", configMINIMAL_STACK_SIZE, NULL, 2, &xButtonTaskHandle);
	//xTaskCreate( vLeibniz, (const char *) "Leibniz", configMINIMAL_STACK_SIZE+10, NULL, 1, &xLeibniz);
    xTaskCreate( vSteuerTask, (const char *) "SteuerTask", configMINIMAL_STACK_SIZE+10, NULL, 2, &xSteuerTask);
    //xTaskCreate(vKellalur, (const char *) "KellalurTask", configMINIMAL_STACK_SIZE, NULL, 1, &xKellalur);
    
    xKommunikation=xEventGroupCreate();
	
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
         Pi4-=(1/n);
         n+=2;
         Pi4+=(1/n);
         n+=2;
         
         if(xEventGroupGetBits(xKommunikation)&ALGSTOPP)
         {
             Pi=4*Pi4;
             xEventGroupSetBits(xKommunikation,ALG_AM_WARTEN);
             xEventGroupWaitBits(xKommunikation,ALG_GO,pdTRUE);
             
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
        Pi_1+=4/(n_1^3-n_1);
        n_1+=2;
        Pi_1-=4/(n_1^3-n_1);
        n_1+=2;
        
        if(xEventGroupGetBits(xKommunikation)&ALGSTOPP_2)
        {
            Pi_2=Pi_1;
            xEventGroupSetBits(xKommunikation,ALG_AM_WARTEN_2);
            xEventGroupWaitBits(xKommunikation,ALG_GO_2,pdTRUE);
            
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
    uint8_t Steuerung=1;
    
    typedef enum
    {
        Start,
        Stopp,
        Zurueck,
        Wechsel,
    } eSteuerugStates;
      eSteuerugStates = Start;
    
    while(1)
    {
        vDisplayClear();
        xEventGroupSetBits(xKommunikation,ALGSTOPP);
        xEventGroupWaitBits(xKommunikation,ALG_AM_WARTEN,pdTRUE);
        LocalPi=Pi
        xEventGroupSetBits(xKommunikation,ALG_GO);
        sprintf(Pi_String, "%f",LocalPi);
        vDisplayWriteStringAtPos(0,0,"Pi=%s",Pi_String);
        xEventGroupSetBits(xKommunikation,ALGSTOPP_2);
        xEventGroupWaitBits(xKommunikation,ALG_AM_WARTEN_2,pdTRUE);
        LocalPi_2=Pi_2
        xEventGroupSetBits(xKommunikation,ALG_GO_2);
        sprintf(Pi_String, "%f",LocalPi_2);
        vDisplayWriteStringAtPos(0,0,"Pi_2=%s",Pi_2_String);
        
        xTaskNotifyWait(0,0xfffffff,&Buttonvalue,0/portTICK_RATE_MS);
        
        switch(Steuerung)
        
        case Start: 
        {   
            if (Buttonvalue&BUTTON2SHORTPRESSEDMASK)
            {
                eSteuerugStates=Stopp;
            }
            
            if (Buttonvalue&BUTTON3SHORTPRESSEDMASK)
            {
                eSteuerugStates=Zurueck;
            }
            if (Buttonvalue&BUTTON4SHORTPRESSEDMASK)
            {
                eSteuerugStates=Wechsel;
            }
                 if (Buttonvalue&BUTTON1SHORTPRESSEDMASK)
                 {
                     if (xLeibniz==NULL)||(xKellalur==NULL)
                    {
                        xTaskCreate( vLeibniz, (const char *) "Leibniz", configMINIMAL_STACK_SIZE+10, NULL, 1, &xLeibniz);
                        xTaskCreate( vKellalur(), (const char *) "Kellular", configMINIMAL_STACK_SIZE+10, NULL, 1, &xKellalur);
                    }
                    else
                    {
                        vTaskResume(xLeibniz);
                        vTaskResume(xKellalur); 
                    }            
                
            }
        break;    
        }         
        
        case Stopp:
        {
            if (Buttonvalue&BUTTON2SHORTPRESSEDMASK)
            {
                eSteuerugStates=Start;
            }
            
            if(Buttonvalue&BUTTON2SHORTPRESSEDMASK)
            {
                vTaskSuspend(xLeibniz);
                vTaskSuspend(xKellalur);
            }
        break;    
        }
        
        case Zurueck:
        {        
                
                if(Buttonvalue&BUTTON3SHORTPRESSEDMASK)
            {
                vTaskDelete(xLeibniz);
                xLeibniz=NULL;
            }
        break;    
        }            
        
        case Wechsel:
        {
            if (Buttonvalue&BUTTON4SHORTPRESSEDMASK)
            {
            
            }
        break;    
        }            
        vTaskDelay(500 / portTICK_RATE_MS);
    }
        
    
}

void vButtonTask(void *pvParameters) {
    initButtons();
    vTaskDelay(3000);

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
        if(getButtonPress(BUTTON1) == LONG_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON1LONGPRESSEDMASK,eSetValueWithOverwrite);
            
        }
        if(getButtonPress(BUTTON2) == LONG_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON2LONGPRESSEDMASK,eSetValueWithOverwrite);
            
        }
        if(getButtonPress(BUTTON3) == LONG_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON3LONGPRESSEDMASK,eSetValueWithOverwrite);
            
        }
        if(getButtonPress(BUTTON4) == LONG_PRESSED) {
            
            xTaskNotify(xSteuerTask,BUTTON4LONGPRESSEDMASK,eSetValueWithOverwrite);
            
        }
        vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
    }

}