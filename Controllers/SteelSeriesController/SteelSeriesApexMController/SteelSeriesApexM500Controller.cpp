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

SteelSeriesApexM500Controller::SteelSeriesApexM500Controller(hid_device *dev_handle, const char *path)
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

    LOG_DEBUG("Steelseries Apex M500 setting mode %i with brightness %i\
and speed %i", mode.value, mode.brightness, mode.speed);
    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x07;
    buf[0x03] = mode.speed + 1;
    hid_write(dev, buf, SS_APEX_M500_PACKET_SIZE);

    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x05;
    buf[0x03] = mode.brightness;
    hid_write(dev, buf, SS_APEX_M500_PACKET_SIZE);
}

void SteelSeriesApexM500Controller::SaveMode()
{
    unsigned char buf[SS_APEX_M500_PACKET_SIZE];

    memset(buf, 0x00, SS_APEX_M500_PACKET_SIZE);
    buf[0x00] = 0x00;
    buf[0x01] = 0x09;
    hid_write(dev, buf, SS_APEX_M500_PACKET_SIZE);
}

std::string SteelSeriesApexM500Controller::GetDeviceLocation()
{
    return ("HID: " + location);
}

std::string SteelSeriesApexM500Controller::GetSerialString()
{
    wchar_t buf[128];
    int ret = hid_get_serial_number_string(dev, buf, 128);
    LOG_DEBUG("Steelseries Apex M500 serial string: <%s>\n", buf);
    if(ret == -1)
    {
        LOG_WARNING("Steelseries Apex M500 could'nt read serial string\n");
        return("Serial number not available");
    }
    std::string serial = StringUtils::wstring_to_string(buf);
    if (serial.empty())
    {
        LOG_WARNING("Steelseries Apex M500 serial string is empty");
        return("Serial number not available");
    }
    return(serial);
}

std::string SteelSeriesApexM500Controller::GetVersionString()
{
    hid_device_info *dev_info = hid_get_device_info(dev);
    if(!dev_info)
    {
        LOG_WARNING("Steelseries Apex M500 could'nt read version number\n");
        return("Version info not available");
    }
    unsigned short version = dev_info->release_number;
    LOG_DEBUG("Steelseries Apex M500 version number: <0x%04x>\n", version);
    std::string version_str;
    version_str.append(std::to_string(version >> 12 & 0xF));
    version_str.append(std::to_string(version >> 8 & 0xF));
    version_str.append(".");
    version_str.append(std::to_string(version >> 4 & 0xF));
    version_str.append(std::to_string(version & 0xF));
    return(version_str);
}
