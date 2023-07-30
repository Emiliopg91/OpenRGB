/*-----------------------------------------------*\
|  RGBController_JginYueInternalUSB.h             |
|                                                 |
|  Generic RGB Interface JginYueInternalUSB Class |
|                                                 |
|  Adam Honse (CalcProgrammer1) 2/25/2020         |
\*-----------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "JginYueInternalUSBController.h"

class RGBController_JginYueInternalUSB : public RGBController
{
public:
    RGBController_JginYueInternalUSB(JginYueInternalUSBController* controller_ptr);

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        SetCustomMode();
    void        DeviceUpdateMode();

private:
    JginYueInternalUSBController*        controller;
};