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



extern void vApplicationIdleHook( void );
void vLeibniz(void *pvParameters);
void vMachin(void *pvParameters);
void vButtonTask(void *pvParameters);
void vSteuerTask(void *pvParameters);

float Pi = 0;
float Pi_1=0;

TaskHandle_t xLeibniz;
TaskHandle_t xMachin;
TaskHandle_t xSteuerTask;
TaskHandle_t xButtonTaskHandle;

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
    xTaskCreate(vMachin, (const char *) "MachinTask", configMINIMAL_STACK_SIZE, NULL, 2, &xMachin);

	
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
         Pi=4*Pi4;
         
        vTaskDelay(5/ portTICK_RATE_MS);
	}
}

void vMachin (void *pvParameters)
{
    (void) pvParameters;
    
    float Pi_Vier=1;
    
    while(1)
    {
        Pi_Vier=(4*atan(1/5))-atan(1/239);
        Pi_1=4*Pi_Vier;
        
        vTaskDelay(5/ portTICK_RATE_MS);
    }
    
}

void vSteuerTask(void *pvParameters)
{
    (void) pvParameters;
    
    uint32_t Buttonvalue;
    char Pi_String[10];
    
    while(1)
    
    {
        vDisplayClear();
        sprintf(Pi_String, "%f",Pi);
        vDisplayWriteStringAtPos(0,0,"Pi=%s",Pi_String);
       
        
        xTaskNotifyWait(0,0xfffffff,&Buttonvalue,0/portTICK_RATE_MS);
        
        if (Buttonvalue&BUTTON1SHORTPRESSEDMASK)
        {
            if (xLeibniz==NULL)
            {
                 xTaskCreate( vLeibniz, (const char *) "Leibniz", configMINIMAL_STACK_SIZE+10, NULL, 1, &xLeibniz);
            }
            else
            {
                vTaskResume(xLeibniz); 
            }            
            
        }
        
        if(Buttonvalue&BUTTON2SHORTPRESSEDMASK)
        {
            vTaskSuspend(xLeibniz);
        }
        if(Buttonvalue&BUTTON3SHORTPRESSEDMASK)
        {
            vTaskDelete(xLeibniz);
            xLeibniz=NULL;
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