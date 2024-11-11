/*---------------------------------------------------------*\
| RGBController_SteelSeriesApexM500.h                       |
|                                                           |
|   RGBController for SteelSeries Apex 7                    |
|                                                           |
|   Myryk                                       11 Nov 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SteelSeriesApexM500Controller.h"

class RGBController_SteelSeriesApexM500 : public RGBController
{
public:
    RGBController_SteelSeriesApexM500(SteelSeriesApexM500Controller* controller_ptr);
    ~RGBController_SteelSeriesApexM500();

    void        SetupZones() {};
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SteelSeriesApexM500Controller*  controller;
};
