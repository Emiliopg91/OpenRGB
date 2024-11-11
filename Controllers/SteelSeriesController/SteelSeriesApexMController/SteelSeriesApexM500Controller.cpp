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

#include <cstring>
#include "SteelSeriesApexM500Controller.h"
#include "LogManager.h"

#define SS_APEX_M500_PACKET_SIZE 32 + 1

SteelSeriesApexM500Controller::SteelSeriesApexM500Controller(hid_device* dev_handle, steelseries_type type, const char* path)
{
    dev         = dev_handle;
    location    = path;
    proto_type  = type;
    EnableLEDControl();
}

SteelSeriesApexM500Controller::~SteelSeriesApexM500Controller()
{
    hid_close(dev);
}

void SteelSeriesApexM500Controller::EnableLEDControl()
{
    LOG_ERROR("steelseries EnableLEDControl\n");
    unsigned char buf[33];
    memset(buf, 0x00, 33);

    buf[0x00] = 0x00;
    buf[0x01] = 0x04;
    buf[0x03] = 0x01;
    // LOG_ERROR("steelseries bytes send: %i\n", hid_send_feature_report(dev, buf, SS_APEX_M500_PACKET_SIZE));

    int bytes = hid_write(dev,buf,33);

    if(bytes < 0 ) {
        printf("Error: %ls\n", hid_error(dev));
    }

    memset(buf, 0x00, 33);

    buf[0x00] = 0x00;
    buf[0x01] = 0x06;
    buf[0x03] = 0x01;
    // LOG_ERROR("steelseries bytes send: %i\n", hid_send_feature_report(dev, buf, SS_APEX_M500_PACKET_SIZE));

    bytes = hid_write(dev,buf,33);

    if(bytes < 0 ) {
        printf("Error: %ls\n", hid_error(dev));
    }

    memset(buf, 0x00, 33);

    buf[0x00] = 0x00;
    buf[0x01] = 0x07;
    buf[0x03] = 0x01;
    // LOG_ERROR("steelseries bytes send: %i\n", hid_send_feature_report(dev, buf, SS_APEX_M500_PACKET_SIZE));

    bytes = hid_write(dev,buf,33);

    if(bytes < 0 ) {
        printf("Error: %ls\n", hid_error(dev));
    }

    memset(buf, 0x00, 33);

    buf[0x00] = 0x00;
    buf[0x01] = 0x05;
    buf[0x03] = 0x64;
    // LOG_ERROR("steelseries bytes send: %i\n", hid_send_feature_report(dev, buf, SS_APEX_M500_PACKET_SIZE));

    bytes = hid_write(dev,buf,33);

    if(bytes < 0 ) {
        printf("Error: %ls\n", hid_error(dev));
    }

    memset(buf, 0x00, 33);

    buf[0x00] = 0x00;
    buf[0x01] = 0x09;
    // LOG_ERROR("steelseries bytes send: %i\n", hid_send_feature_report(dev, buf, SS_APEX_M500_PACKET_SIZE));

    bytes = hid_write(dev,buf,33);

    if(bytes < 0 ) {
        printf("Error: %ls\n", hid_error(dev));
    }
}

void SteelSeriesApexM500Controller::SetMode(unsigned char /*mode*/, std::vector<RGBColor> /*colors*/)
{
    LOG_ERROR("steelseries SetMode\n");
}

void SteelSeriesApexM500Controller::SetLEDsDirect(std::vector<RGBColor> colors)
{
    LOG_ERROR("steelseries SetLEDsDirect\n");
}
