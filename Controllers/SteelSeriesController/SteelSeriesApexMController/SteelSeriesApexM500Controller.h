/*---------------------------------------------------------*\
| SteelSeriesApexM500Controller.cpp                            |
|                                                           |
|   Driver for SteelSeries Apex M500                        |
|                                                           |
|   Florian Heilmann (FHeilmann)                12 Oct 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include "SteelSeriesGeneric.h"
#include "SteelSeriesApexBaseController.h"

class SteelSeriesApexM500Controller
{
public:
    SteelSeriesApexM500Controller(hid_device* dev_handle, const char* path);
    ~SteelSeriesApexM500Controller();

    void SetMode(mode mode);

private:
    void EnableLEDControl();
    void SelectProfile
        (
            unsigned char   profile
        );
protected:
    std::string             location;
    hid_device*             dev;
    unsigned char           active_mode;
};
