/********************************************************************
	created:	2021/08/04
	created:	4:8:2021   18:31
	filename: 	c:\users\thom\Documents\Atmel Studio\7.0\CollectiveStick\CollectiveStick\CollectiveStick\Sketch.cpp
	file path:	c:\users\thom\Documents\Atmel Studio\7.0\CollectiveStick\CollectiveStick\CollectiveStick
	file base:	Sketch
	file ext:	cpp
	author:		THZ
	
	purpose:    Collective stick design with LEDs and control buttons
*********************************************************************/


#include "Arduino.h"

#include <stdint.h>
#include "stdio.h"

#include "collectiveStick.h"

//HIDcontrollerCS CStick;


//static uint8_t  LEDs[20];
static uint16_t buttonNo = 0;
static Bool     btnPressed = false;

static uint16_t collective;
static uint16_t aileronTrim;
static uint16_t cyclicTrim;

static unsigned long    previousTicks;
static unsigned long    currentTicks;

uint32_t const  interval = 1000;


void setup() 
{
    //Serial.begin(38400);
    previousTicks = millis();
    CollectiveStick.begin();
}

void loop() 
{
    
    // Keep track of time elapsed
    currentTicks = millis();
    
    // IN event processing
    if (currentTicks - previousTicks >= interval)
    {
        previousTicks = currentTicks;
        
        // Make changes to controller - buttons
        if (buttonNo >= 16)
            buttonNo = 0;
        else if (!btnPressed)
        {
            CollectiveStick.btnPress(buttonNo);
            btnPressed = true;
        }
        else
        {
            CollectiveStick.btnRelease(buttonNo);
            btnPressed = false;
            buttonNo++;
        }            
        
        // Make changes to controller - axes
        collective += 100;
        if (collective > 1000)
            collective = 0;
            
        aileronTrim += 30;
        if (aileronTrim > 1000)
            aileronTrim = 0;
            
        cyclicTrim += 10;
        if (cyclicTrim > 1000)
            cyclicTrim = 0;
        CollectiveStick.setAxis(collective, aileronTrim, cyclicTrim);
        
        // Send update and report bytes sent
        //sentBytes = CStick.write(HIDbuffer, 6);
        /*
        Serial.print("Sent HIDreport: ");
        Serial.print(sentBytes);
        Serial.println(" bytes");
        */
    }
   
   
    // OUT event processing
    //if (CStick.read(LEDs, OUT_EP_REPORT_SIZE) > 0)
    {
        /*
        Serial.print("LEDs: ");
        Serial.print(LEDs[0], HEX);
        Serial.print("/");
        Serial.print(LEDs[1], HEX);
        Serial.print("/");
        Serial.print(LEDs[2], HEX);
        Serial.println();   // newline
        */
    }
}
