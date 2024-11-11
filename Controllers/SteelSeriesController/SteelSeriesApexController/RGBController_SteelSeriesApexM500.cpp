/*---------------------------------------------------------*\
| RGBController_SteelSeriesApexM500.cpp                     |
|                                                           |
|   RGBController for SteelSeries Apex 7                    |
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

static const char* zone_names[] =
{
    ZONE_EN_KEYBOARD
};

static zone_type zone_types[] =
{
    ZONE_TYPE_MATRIX
};

static const unsigned int zone_sizes[] =
{
    sizeof(led_names)/sizeof(char*),
};

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

    // SetupZones();
}

RGBController_SteelSeriesApexM500::~RGBController_SteelSeriesApexM500()
{
    /*---------------------------------------------------------*\
    | Delete the matrix map                                     |
    \*---------------------------------------------------------*/
    for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        if(zones[zone_index].matrix_map != NULL)
        {
            free(zones[zone_index].matrix_map->map);
            delete zones[zone_index].matrix_map;
        }
    }

    delete controller;
}

void RGBController_SteelSeriesApexM500::SetupZones()
{
    LOG_ERROR("steelseries SetupZones\n");
    /*---------------------------------------------------------*\
    | Set up zones                                              |
    \*---------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | The first 5 chars are the SKU which we need to determine  |
    | the region.                                               |
    \*---------------------------------------------------------*/

    std::string sku = serial.substr(0, 5);

    unsigned int total_led_count = 0;

    for(unsigned int zone_idx = 0; zone_idx < 1; zone_idx++)
    {
        zone new_zone;
        new_zone.name           = zone_names[zone_idx];
        new_zone.type           = zone_types[zone_idx];

        if(zone_types[zone_idx] == ZONE_TYPE_MATRIX)
        {
            new_zone.matrix_map         = new matrix_map_type;
            new_zone.matrix_map->map    = (unsigned int *) malloc(matrix_mapsize*sizeof(unsigned int));

            if((proto_type == APEX) || (proto_type == APEX_M))
            {
                SetSkuRegion(*new_zone.matrix_map, sku);
            }
        }
        else
        {
            new_zone.matrix_map         = NULL;
        }

        if((proto_type == APEX) || (proto_type == APEX_M))
        {
            new_zone.leds_min   = zone_sizes[zone_idx];
            new_zone.leds_max   = zone_sizes[zone_idx];
            new_zone.leds_count = zone_sizes[zone_idx];
            total_led_count    += zone_sizes[zone_idx];
        }
        zones.push_back(new_zone);
    };

    SetSkuLedNames(leds, sku, total_led_count);
    SetupColors();
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
