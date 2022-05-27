/*
 * collectiveStick.cpp
 *
 * Created: 2021-08-04 18:40:13
 *  Author: Thom
 */ 

// Guard for endpoints
#define HID_NUMBER_OF_ENDPOINTS     1   // IN and OUT

#include "collectiveStick.h"

#if defined(_USING_HID)

#define INCLUDE_OUT_LEDS_ENDPOINT           0   // IN events only


static const uint8_t _hidReportDescriptorCS[] PROGMEM = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,                    //   USAGE_MAXIMUM (Button 16)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //   USAGE (Pointer)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03,              //   LOGICAL_MAXIMUM (1023)
    0x75, 0x0a,                    //   REPORT_SIZE (10)
    0x95, 0x03,                    //   REPORT_COUNT (3)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x32,                    //     USAGE (Z)
    0x09, 0x33,                    //     USAGE (Rx)
    0x09, 0x34,                    //     USAGE (Ry)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION

// #if INCLUDE_OUT_LEDS_ENDPOINT == 1
//     0x05, 0x08,                    //   USAGE_PAGE (LEDs)
//     0x09, 0x4b,                    //   USAGE (Generic Indicator)
//     0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//     0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//     0x75, 0x08,                    //   REPORT_SIZE (8)
//     0x95, 0x02,                    //   REPORT_COUNT (2)
//     0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
// #endif
};


//================================================================================
//================================================================================
//  Collective stick

CollectiveStick_::CollectiveStick_(uint8_t reportID = 1) : _reportID(reportID), _stick_buttons(0), _buttons_shadow(0), _LEDs(0)
{
    static HIDSubDescriptor node(_hidReportDescriptorCS, sizeof(_hidReportDescriptorCS));
    HID().AppendDescriptor(&node); 
}


/*
 *	author: Thom
 *	date:	4-8-2021
 *
 *	brief:	private method
 *          Packs the HID report into 6 bytes and sends
 *          data to IN endpoint.
 *******************************************************
 */
void CollectiveStick_::packAndSendtoEndpoint(void)
{
    uint16_t    *hidReport_buttons = (uint16_t *)_hidBuffer;
    uint32_t    *hidReport_axes = (uint32_t *)&_hidBuffer[2];
    
    // copy/pack buttons
    *hidReport_buttons = _stick_buttons;
    // Pack axes
    *hidReport_axes = _cyclicTrim & TEN_BIT_MASK;
    *hidReport_axes <<= 10;
    *hidReport_axes |= _aileronTrim & TEN_BIT_MASK;
    *hidReport_axes <<= 10;
    *hidReport_axes |= _collective & TEN_BIT_MASK;

    // Send it...
    _lastSendResult = HID().SendReport(_reportID, _hidBuffer, REPORT_LENGTH_IN);
}


/*
 *	author: Thom
 *	date:	4-8-2021
 *
 *	brief:	begin() method
 *          Dummy start of object
 *******************************************************
 */
void CollectiveStick_::begin(void)
{
}


/*
 *	author: Thom
 *	date:	4-8-2021
 *
 *	brief:	end() method
 *          Dummy stop of object
 *******************************************************
 */
void CollectiveStick_::end(void)
{
}


/*
 *	author: Thom
 *	date:	4-8-2021
 *
 *	brief:	setAxis() method
 *          Update value for one axis
 *******************************************************
 */
void CollectiveStick_::setAxis(uint16_t coll, uint16_t aTrim, uint16_t cTrim)
{
    uint8_t  doUpdate = 0;
    uint16_t play_deviation;
    
    coll &= TEN_BIT_MASK;
    aTrim &= TEN_BIT_MASK;
    cTrim &= TEN_BIT_MASK;
    
    // Collective stick axis
    play_deviation = max(coll, _collective) - min(coll, _collective);
    if (play_deviation > AXIS_VALUE_ALLOWED_PLAY)
    {
        _collective = coll;
        doUpdate++;     // Change, indicate update is needed
    }        
        
    // Aileron trim axis
    play_deviation = max(aTrim, _aileronTrim) - min(aTrim, _aileronTrim);
    if (play_deviation > AXIS_VALUE_ALLOWED_PLAY)
    {
        _aileronTrim = aTrim;
        doUpdate++;     // Change, indicate update is needed
    }
    
    // Cyclic trim axis   
    play_deviation = max(cTrim, _cyclicTrim) - min(cTrim, _cyclicTrim);
    if (play_deviation > AXIS_VALUE_ALLOWED_PLAY)
    {
        _cyclicTrim = cTrim;
        doUpdate++;     // Change, indicate update is needed
    }
    
    if (doUpdate)
    {
       // Pack and send axes
       packAndSendtoEndpoint();
    }       
}

/*
 *	author: Thom
 *	date:	5-8-2021
 *
 *	brief:	btnPress() method
 *          Update value register for buttons - make
 *******************************************************
 */
void CollectiveStick_::btnPress(uint8_t b_no)
{
    if (b_no < CSTICK_NUMBER_OF_BUTTONS)
    {
        _stick_buttons |= 1 << b_no;
        if (_stick_buttons != _buttons_shadow)
        {
            _buttons_shadow = _stick_buttons;
            packAndSendtoEndpoint();        // IN event to host
        }
    }
}   


/*
 *	author: Thom
 *	date:	5-8-2021
 *
 *	brief:	btnRelease() method
 *          Update value register for buttons - break
 *******************************************************
 */
void CollectiveStick_::btnRelease(uint8_t b_no)
{
    if (b_no < CSTICK_NUMBER_OF_BUTTONS)
    {
        _stick_buttons &= ~(1 << b_no);
        if (_stick_buttons != _buttons_shadow)
        {
            _buttons_shadow = _stick_buttons;
            packAndSendtoEndpoint();        // IN event to host
        }
    }
}     
    

/*
 *	author: Thom
 *	date:	5-8-2021
 *
 *	brief:	IsBtnPressed() method
 *          Test specific button for make/break.
 *          Returns true if pressed
 *******************************************************
 */
Bool CollectiveStick_::isBtnPressed(uint8_t b_no)
{
    if (b_no < CSTICK_NUMBER_OF_BUTTONS)
    {
        if (_stick_buttons & (1 << b_no))
            return true;
    }
    return false;
}


/*
 *	author: Thom
 *	date:	5-8-2021
 *
 *	brief:	Fetch data from LEDs OUT event.
 *
 *  NOTE: Requires INCLUDE_OUT_LEDS_ENDPOINT set to 2
 *******************************************************
 */
uint16_t CollectiveStick_::getLEDs(void)
{
#if HID_NUMBER_OF_ENDPOINTS == 2
    return _LEDs;
#else
    return 0;        
#endif
}

CollectiveStick_ CollectiveStick;

#endif
