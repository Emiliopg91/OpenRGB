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

#include <cstring>
#include "SteelSeriesApexM500Controller.h"

#define SS_APEX_M500_PACKET_SIZE 32 + 1

SteelSeriesApexM500Controller::SteelSeriesApexM500Controller(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
}

SteelSeriesApexM500Controller::~SteelSeriesApexM500Controller()
{
    hid_close(dev);
}

void SteelSeriesApexM500Controller::SetMode(mode mode)
{
    unsigned char buf[SS_APEX_M500_PACKET_SIZE];

    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x07;
    buf[0x03] = mode.speed + 1;
    hid_write(dev,buf,SS_APEX_M500_PACKET_SIZE);

    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x05;
    buf[0x03] = mode.brightness;
    hid_write(dev,buf,SS_APEX_M500_PACKET_SIZE);

    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x09;
    hid_write(dev,buf,SS_APEX_M500_PACKET_SIZE);
}

std::string SteelSeriesApexM500Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string SteelSeriesApexM500Controller::GetSerialString()
{
    std::string return_string = "";
    return(return_string);
}


std::string SteelSeriesApexM500Controller::GetVersionString()
{

    std::string return_string = "";
    return(return_string);
}
