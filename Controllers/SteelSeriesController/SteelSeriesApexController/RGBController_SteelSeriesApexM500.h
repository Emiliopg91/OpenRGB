/*---------------------------------------------------------*\
| RGBController_SteelSeriesApexM500.h                       |
|                                                           |
|   RGBController for SteelSeries Apex 7                    |
|                                                           |
|   Eric Samuelson (edbgon)                     05 Jul 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include "RGBController.h"
#include "SteelSeriesApexBaseController.h"
#include "SteelSeriesGeneric.h"

class RGBController_SteelSeriesApexM500 : public RGBController
{
public:
    RGBController_SteelSeriesApexM500(SteelSeriesApexBaseController* controller_ptr);
    ~RGBController_SteelSeriesApexM500();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SteelSeriesApexBaseController*  controller;
    steelseries_type                proto_type;

    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;
};
