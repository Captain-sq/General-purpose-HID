/*
 * collectiveStick.h
 *
 * Created: 2021-08-04 18:39:07
 *  Author: Thom
 */ 


#ifndef COLLECTIVESTICK_H_
#define COLLECTIVESTICK_H_

#include "HID.h"

#if !defined(_USING_HID)

#warning "Using legacy HID core (non pluggable)"
#else

//================================================================================
//================================================================================
//  Collective stick
#define CSTICK_AXIS_COLLECIVE_CONTROL               1
#define CSTICK_AXIS_AILERONTRIM_CONTROL             2
#define CSTICK_AXIS_CYCLICTRIM_CONTROL              3
#define CSTICK_NUMBER_OF_BUTTONS                    16
#define CSTICK_NUMBER_OF_INDICATOR_LEDS             16

#define TEN_BIT_MASK                                0x3FF
#define AXIS_VALUE_ALLOWED_PLAY                     4
#define REPORTID_0                                  0
#define REPORT_LENGTH_IN                            6
#define REPORT_LENGTH_OUT                           2

typedef unsigned char       Bool;

// Class
class CollectiveStick_
{
  private:
    uint8_t     _reportID;
    uint16_t    _stick_buttons;
    uint16_t    _buttons_shadow;
    uint16_t    _collective;
    uint16_t    _aileronTrim;
    uint16_t    _cyclicTrim;
    uint16_t    _LEDs;
    uint8_t     _hidBuffer[10];
    int         _lastSendResult;
    int         _lastReceiveResult;
    void        packAndSendtoEndpoint(void);
    
  public:
    CollectiveStick_(uint8_t r_id);   // constructor
    
    void        begin(void);
    void        end(void);
    void        setAxis(uint16_t coll, uint16_t aTrim, uint16_t cTrim); // All analog axes
    void        btnPress(uint8_t b_no);         // press button no
    void        btnRelease(uint8_t b_no);       // release button no
    Bool        isBtnPressed(uint8_t b_no);     // test, is button no pressed?
    uint16_t    getLEDs(void);
};

extern CollectiveStick_ CollectiveStick;


#endif  /* _USING_HID   */
#endif /* COLLECTIVESTICK_H_ */