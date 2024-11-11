/*---------------------------------------------------------*\
| RGBController_SteelSeriesApexM500.cpp                     |
|                                                           |
|   RGBController for SteelSeries Apex M500                 |
|                                                           |
|   Eric Samuelson (edbgon)                     05 Jul 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_SteelSeriesApexM500.h"
#include "SteelSeriesApexRegions.h"
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name Steel Series APEX
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectSteelSeriesApex,DetectSteelSeriesApexM
    @comment
\*-------------------------------------------------------------------*/

RGBController_SteelSeriesApexM500::RGBController_SteelSeriesApexM500(SteelSeriesApexBaseController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "SteelSeries Apex RGB Keyboard";
    vendor      = "SteelSeries";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "SteelSeries Apex RGB Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();
    version     = controller->GetVersionString();

    LOG_ERROR("steelseries location: %s\n", location.c_str());
    LOG_ERROR("steelseries serial: %s\n", serial.c_str());
    LOG_ERROR("steelseries version: %s\n", version.c_str());

    proto_type  = controller->proto_type;

    mode Static;
    Static.name       = "Static";
    Static.value      = 0x00;
    Static.brightness_max = 0x64;
    Static.flags      = MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode = MODE_COLORS_NONE;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name       = "Breathing";
    Breathing.value      = 0x01;
    Breathing.brightness_max = 0x64;
    Breathing.speed_max = 0x03;
    Breathing.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Breathing.color_mode = MODE_COLORS_NONE;
    modes.push_back(Breathing);
}

RGBController_SteelSeriesApexM500::~RGBController_SteelSeriesApexM500()
{
    delete controller;
}

void RGBController_SteelSeriesApexM500::ResizeZone(int /*zone*/, int /*new_size*/)
{
    LOG_ERROR("steelseries ResizeZone\n");
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_SteelSeriesApexM500::DeviceUpdateLEDs()
{
    LOG_ERROR("steelseries DeviceUpdateLEDs\n");
    last_update_time = std::chrono::steady_clock::now();
    controller->SetLEDsDirect(colors);
}

void RGBController_SteelSeriesApexM500::UpdateZoneLEDs(int /*zone*/)
{
    LOG_ERROR("steelseries UpdateZoneLEDs\n");
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesApexM500::UpdateSingleLED(int /*led*/)
{
    LOG_ERROR("steelseries UpdateSingleLED\n");
    DeviceUpdateLEDs();
}

void RGBController_SteelSeriesApexM500::DeviceUpdateMode()
{
    LOG_ERROR("steelseries DeviceUpdateMode\n");

    // std::vector<RGBColor> temp_colors;
    // controller->SetMode(modes[active_mode].value, temp_colors);
}
