/*---------------------------------------------------------*\
| SteelSeriesApexM500Controller.cpp                         |
|                                                           |
|   Driver for SteelSeries Apex M500                        |
|                                                           |
|   Myryk                                       12 Oct 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

class SteelSeriesApexM500Controller
{
public:
    SteelSeriesApexM500Controller(hid_device* dev_handle, const char* path);
    ~SteelSeriesApexM500Controller();

    void SetMode(mode mode);
    std::string GetDeviceLocation();
    std::string GetSerialString();
    std::string GetVersionString();
protected:
    std::string             location;
    hid_device*             dev;
    unsigned char           active_mode;
};
