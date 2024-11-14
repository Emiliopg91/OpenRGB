/*---------------------------------------------------------*\
| RGBController_SteelSeriesApexM500.cpp                     |
|                                                           |
|   RGBController for SteelSeries Apex M500                 |
|                                                           |
|   Myryk                                       11 Nov 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_SteelSeriesApexM500.h"

/**------------------------------------------------------------------*\
    @name Steel Series APEX
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectSteelSeriesApexM500
    @comment
\*-------------------------------------------------------------------*/

RGBController_SteelSeriesApexM500::RGBController_SteelSeriesApexM500(SteelSeriesApexM500Controller* controller_ptr)
{
    controller  = controller_ptr;

    name        = "SteelSeries Apex RGB Keyboard";
    vendor      = "SteelSeries";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "SteelSeries Apex RGB Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();
    version     = controller->GetVersionString();

    mode Static;
    Static.name       = "Static";
    Static.value      = 0x00;
    Static.brightness_max = 0x64;
    Static.flags      = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Static.color_mode = MODE_COLORS_NONE;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name       = "Breathing";
    Breathing.value      = 0x01;
    Breathing.brightness_max = 0x64;
    Breathing.speed_max = 0x03;
    Breathing.speed_min = 0x01;
    Breathing.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS
        | MODE_FLAG_MANUAL_SAVE;
    Breathing.color_mode = MODE_COLORS_NONE;
    modes.push_back(Breathing);
}

RGBController_SteelSeriesApexM500::~RGBController_SteelSeriesApexM500()
{
    delete controller;
}

void RGBController_SteelSeriesApexM500::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApexM500::DeviceUpdateLEDs()
{
    /*---------------------------------------------------------*\
    | This device does not support setting LEDS                 |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApexM500::UpdateZoneLEDs(int /*zone*/)
{
    /*---------------------------------------------------------*\
    | This device does not support setting LEDS                 |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApexM500::UpdateSingleLED(int /*led*/)
{
    /*---------------------------------------------------------*\
    | This device does not support setting LEDS                 |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApexM500::DeviceUpdateMode()
{
    LOG_DEBUG("Steelseries Apex M500 DeviceUpdateMode");
    controller->SetMode(modes[active_mode]);
}

void        RGBController_SteelSeriesApexM500::SaveMode()
{
    LOG_ERROR("Steelseries Apex M500 save mode");
    controller->SaveMode();
}
