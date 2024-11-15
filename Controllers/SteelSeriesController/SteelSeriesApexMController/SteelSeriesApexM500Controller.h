/*---------------------------------------------------------*\
| SteelSeriesApexM500Controller.cpp                         |
|                                                           |
|   Driver for SteelSeries Apex M500                        |
|                                                           |
|   Myryk                                       11 Nov 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include "LogManager.h"
#include "StringUtils.h"

class SteelSeriesApexM500Controller
{
public:
    SteelSeriesApexM500Controller(hid_device* dev_handle, const char* path);
    ~SteelSeriesApexM500Controller();

    void SetMode(mode mode);
    void SaveMode();
    std::string GetDeviceLocation();
    std::string GetSerialString();
    std::string GetVersionString();
protected:
    std::string             location;
    hid_device*             dev;
    unsigned char           active_mode;
};
