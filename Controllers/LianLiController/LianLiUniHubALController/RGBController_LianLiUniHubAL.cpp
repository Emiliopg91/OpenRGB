/*---------------------------------------------------------*\
| RGBController_LianLiUniHubAL.cpp                          |
|                                                           |
|   RGBController for Lian Li AL Uni Hub                    |
|                                                           |
|   Oliver P                                    26 Apr 2022 |
|   Credit to Luca Lovisa for original work                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <string>

#include "KeyboardLayoutManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "RGBController_LianLiUniHubAL.h"

std::map<std::string, unsigned char> lianli_al_zones =
{
    {"Zone 1", 1},
    {"Zone 2", 0},
    {"Zone 3", 0},
    {"Zone 4", 0}
};

keyboard_keymap_overlay lianli_a1_empty
{
    KEYBOARD_SIZE::KEYBOARD_SIZE_EMPTY,
    {
    /*---------------------------------------------------------------------------------------------------------*\
    | Edit Keys                                                                                                 |
    |   Zone,   Row,    Column,     Value,      Key,                        OpCode,                             |
    \*---------------------------------------------------------------------------------------------------------*/
        {   0,      0,       0,         0,        KEY_EN_UNUSED,            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      0,       0,         0,        KEY_EN_UNUSED,            KEYBOARD_OPCODE_INSERT_ROW,         },
        {   0,      0,       0,         0,        KEY_EN_UNUSED,            KEYBOARD_OPCODE_INSERT_ROW,         },
        {   0,      2,       0,         0,        KEY_EN_UNUSED,            KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },
    }
};

keyboard_keymap_overlay lianli_a1_fan
{
    KEYBOARD_SIZE::KEYBOARD_SIZE_EMPTY,
    {
    /*---------------------------------------------------------------------------------------------------------*\
    | Edit Keys                                                                                                 |
    |   Zone,   Row,    Column,     Value,      Key,                        OpCode,                             |
    \*---------------------------------------------------------------------------------------------------------*/
        {   0,      3,       2,         0,        "Inner LED 1",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      2,       3,         1,        "Inner LED 2",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      2,       4,         2,        "Inner LED 3",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      3,       5,         3,        "Inner LED 4",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      4,       5,         4,        "Inner LED 5",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      5,       4,         5,        "Inner LED 6",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      5,       3,         6,        "Inner LED 7",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      4,       2,         7,        "Inner LED 8",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      2,       0,         8,        "Outer LED 1",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      1,       1,         9,        "Outer LED 2",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      0,       2,        10,        "Outer LED 3",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      0,       5,        11,        "Outer LED 4",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      1,       6,        12,        "Outer LED 5",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      2,       7,        13,        "Outer LED 6",            KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      5,       7,        14,        "Outer LED 7",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      6,       6,        15,        "Outer LED 8",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      7,       5,        16,        "Outer LED 9",            KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },
        {   0,      7,       2,        17,        "Outer LED 10",           KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      6,       1,        18,        "Outer LED 11",           KEYBOARD_OPCODE_SWAP_ONLY,          },
        {   0,      5,       0,        19,        "Outer LED 12",           KEYBOARD_OPCODE_SWAP_ONLY,          },
    }
};

/**------------------------------------------------------------------*\
    @name Lian Li Uni Hub AL
    @type USB
    @save :x:
    @direct :rotating_light:
    @effects :white_check_mark:
    @detectors DetectLianLiUniHubAL
    @comment Fan counts for each Zone are set in the config file.
\*-------------------------------------------------------------------*/

RGBController_LianLiUniHubAL::RGBController_LianLiUniHubAL(LianLiUniHubALController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "Lian Li Uni Hub - AL";
    vendor      = "Lian Li";
    type        = DEVICE_TYPE_COOLER;
    description = name;
    version     = controller->GetFirmwareVersionString();
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    initializedMode = false;

    mode Custom;
    Custom.name                 = "Custom";
    Custom.value                = UNIHUB_AL_LED_MODE_STATIC_COLOR;
    Custom.flags                = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_PER_LED_COLOR;
    Custom.brightness_min       = 0;
    Custom.brightness_max       = 50;
    Custom.brightness           = 37;
    Custom.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    mode RainbowWave;
    RainbowWave.name            = "Rainbow Wave";
    RainbowWave.value           = UNIHUB_AL_LED_MODE_RAINBOW;
    RainbowWave.flags           = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    RainbowWave.speed_min       = 0;
    RainbowWave.speed_max       = 4;
    RainbowWave.brightness_min  = 0;
    RainbowWave.brightness_max  = 4;
    RainbowWave.speed           = 3;
    RainbowWave.brightness      = 3;
    RainbowWave.direction       = UNIHUB_AL_LED_DIRECTION_LTR;
    RainbowWave.color_mode      = MODE_COLORS_NONE;
    modes.push_back(RainbowWave);

    mode RainbowMorph;
    RainbowMorph.name           = "Rainbow Morph";
    RainbowMorph.value          = UNIHUB_AL_LED_MODE_RAINBOW_MORPH;
    RainbowMorph.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    RainbowMorph.speed_min      = 0;
    RainbowMorph.speed_max      = 4;
    RainbowMorph.brightness_min = 0;
    RainbowMorph.brightness_max = 4;
    RainbowMorph.speed          = 3;
    RainbowMorph.brightness     = 3;
    RainbowMorph.color_mode     = MODE_COLORS_NONE;
    modes.push_back(RainbowMorph);

    mode StaticColor;
    StaticColor.name            = "Static Color";
    StaticColor.value           = UNIHUB_AL_LED_MODE_STATIC_COLOR;
    StaticColor.flags           = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    StaticColor.brightness_min  = 0;
    StaticColor.brightness_max  = 4;
    StaticColor.colors_min      = 0;
    StaticColor.colors_max      = 4;
    StaticColor.brightness      = 3;
    StaticColor.color_mode      = MODE_COLORS_MODE_SPECIFIC;
    StaticColor.colors.resize(4);
    modes.push_back(StaticColor);

    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = UNIHUB_AL_LED_MODE_BREATHING;
    Breathing.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Breathing.speed_min         = 0;
    Breathing.speed_max         = 4;
    Breathing.brightness_min    = 0;
    Breathing.brightness_max    = 4;
    Breathing.colors_min        = 0;
    Breathing.colors_max        = 4;
    Breathing.speed             = 3;
    Breathing.brightness        = 3;
    Breathing.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors.resize(4);
    modes.push_back(Breathing);

    mode Taichi;
    Taichi.name                 = "Taichi";
    Taichi.value                = UNIHUB_AL_LED_MODE_TAICHI;
    Taichi.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Taichi.speed_min            = 0;
    Taichi.speed_max            = 4;
    Taichi.brightness_min       = 0;
    Taichi.brightness_max       = 4;
    Taichi.colors_min           = 0;
    Taichi.colors_max           = 2;
    Taichi.speed                = 3;
    Taichi.brightness           = 3;
    Taichi.direction            = UNIHUB_AL_LED_DIRECTION_LTR;
    Taichi.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Taichi.colors.resize(2);
    modes.push_back(Taichi);

    mode ColorCycle;
    ColorCycle.name             = "ColorCycle";
    ColorCycle.value            = UNIHUB_AL_LED_MODE_COLOR_CYCLE;
    ColorCycle.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    ColorCycle.speed_min        = 0;
    ColorCycle.speed_max        = 4;
    ColorCycle.brightness_min   = 0;
    ColorCycle.brightness_max   = 4;
    ColorCycle.colors_min       = 0;
    ColorCycle.colors_max       = 4;
    ColorCycle.speed            = 3;
    ColorCycle.brightness       = 3;
    ColorCycle.direction        = UNIHUB_AL_LED_DIRECTION_LTR;
    ColorCycle.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    ColorCycle.colors.resize(4);
    modes.push_back(ColorCycle);

    mode Runway;
    Runway.name                 = "Runway";
    Runway.value                = UNIHUB_AL_LED_MODE_RUNWAY;
    Runway.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Runway.speed_min            = 0;
    Runway.speed_max            = 4;
    Runway.brightness_min       = 0;
    Runway.brightness_max       = 4;
    Runway.colors_min           = 0;
    Runway.colors_max           = 2;
    Runway.speed                = 3;
    Runway.brightness           = 3;
    Runway.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Runway.colors.resize(2);
    modes.push_back(Runway);

    mode Meteor;
    Meteor.name                 = "Meteor";
    Meteor.value                = UNIHUB_AL_LED_MODE_METEOR;
    Meteor.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Meteor.speed_min            = 0;
    Meteor.speed_max            = 4;
    Meteor.brightness_min       = 0;
    Meteor.brightness_max       = 4;
    Meteor.colors_min           = 0;
    Meteor.colors_max           = 4;
    Meteor.speed                = 3;
    Meteor.brightness           = 3;
    Meteor.direction            = UNIHUB_AL_LED_DIRECTION_LTR;
    Meteor.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Meteor.colors.resize(4);
    modes.push_back(Meteor);

    mode Warning;
    Warning.name                = "Warning";
    Warning.value               = UNIHUB_AL_LED_MODE_WARNING;
    Warning.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Warning.speed_min           = 0;
    Warning.speed_max           = 4;
    Warning.brightness_min      = 0;
    Warning.brightness_max      = 4;
    Warning.colors_min          = 0;
    Warning.colors_max          = 4;
    Warning.speed               = 3;
    Warning.brightness          = 3;
    Warning.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    Warning.colors.resize(4);
    modes.push_back(Warning);

    mode Voice;
    Voice.name                  = "Voice";
    Voice.value                 = UNIHUB_AL_LED_MODE_VOICE;
    Voice.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Voice.speed_min             = 0;
    Voice.speed_max             = 4;
    Voice.brightness_min        = 0;
    Voice.brightness_max        = 4;
    Voice.colors_min            = 0;
    Voice.colors_max            = 4;
    Voice.speed                 = 3;
    Voice.brightness            = 3;
    Voice.direction             = UNIHUB_AL_LED_DIRECTION_LTR;
    Voice.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Voice.colors.resize(4);
    modes.push_back(Voice);

    mode SpinningTeacup;
    SpinningTeacup.name             = "SpinningTeacup";
    SpinningTeacup.value            = UNIHUB_AL_LED_MODE_SPINNING_TEACUP;
    SpinningTeacup.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    SpinningTeacup.speed_min        = 0;
    SpinningTeacup.speed_max        = 4;
    SpinningTeacup.brightness_min   = 0;
    SpinningTeacup.brightness_max   = 4;
    SpinningTeacup.colors_min       = 0;
    SpinningTeacup.colors_max       = 4;
    SpinningTeacup.speed            = 3;
    SpinningTeacup.brightness       = 3;
    SpinningTeacup.direction        = UNIHUB_AL_LED_DIRECTION_LTR;
    SpinningTeacup.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    SpinningTeacup.colors.resize(4);
    modes.push_back(SpinningTeacup);

    mode Tornado;
    Tornado.name                = "Tornado";
    Tornado.value               = UNIHUB_AL_LED_MODE_TORNADO;
    Tornado.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Tornado.speed_min           = 0;
    Tornado.speed_max           = 4;
    Tornado.brightness_min      = 0;
    Tornado.brightness_max      = 4;
    Tornado.colors_min          = 0;
    Tornado.colors_max          = 4;
    Tornado.speed               = 3;
    Tornado.brightness          = 3;
    Tornado.direction           = UNIHUB_AL_LED_DIRECTION_LTR;
    Tornado.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    Tornado.colors.resize(4);
    modes.push_back(Tornado);

    mode Mixing;
    Mixing.name                 = "Mixing";
    Mixing.value                = UNIHUB_AL_LED_MODE_MIXING;
    Mixing.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Mixing.speed_min            = 0;
    Mixing.speed_max            = 4;
    Mixing.brightness_min       = 0;
    Mixing.brightness_max       = 4;
    Mixing.colors_min           = 0;
    Mixing.colors_max           = 2;
    Mixing.speed                = 3;
    Mixing.brightness           = 3;
    Mixing.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Mixing.colors.resize(2);
    modes.push_back(Mixing);

    mode Stack;
    Stack.name                  = "Stack";
    Stack.value                 = UNIHUB_AL_LED_MODE_STACK;
    Stack.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Stack.speed_min             = 0;
    Stack.speed_max             = 4;
    Stack.brightness_min        = 0;
    Stack.brightness_max        = 4;
    Stack.colors_min            = 0;
    Stack.colors_max            = 2;
    Stack.speed                 = 3;
    Stack.brightness            = 3;
    Stack.direction             = UNIHUB_AL_LED_DIRECTION_LTR;
    Stack.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Stack.colors.resize(2);
    modes.push_back(Stack);

    mode Staggered;
    Staggered.name              = "Staggered";
    Staggered.value             = UNIHUB_AL_LED_MODE_STAGGGERED;
    Staggered.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Staggered.speed_min         = 0;
    Staggered.speed_max         = 4;
    Staggered.brightness_min    = 0;
    Staggered.brightness_max    = 4;
    Staggered.colors_min        = 0;
    Staggered.colors_max        = 4;
    Staggered.speed             = 3;
    Staggered.brightness        = 3;
    Staggered.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Staggered.colors.resize(4);
    modes.push_back(Staggered);

    mode Tide;
    Tide.name                   = "Tide";
    Tide.value                  = UNIHUB_AL_LED_MODE_TIDE;
    Tide.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Tide.speed_min              = 0;
    Tide.speed_max              = 4;
    Tide.brightness_min         = 0;
    Tide.brightness_max         = 4;
    Tide.colors_min             = 0;
    Tide.colors_max             = 4;
    Tide.speed                  = 3;
    Tide.brightness             = 3;
    Tide.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Tide.colors.resize(4);
    modes.push_back(Tide);

    mode Scan;
    Scan.name                   = "Scan";
    Scan.value                  = UNIHUB_AL_LED_MODE_SCAN;
    Scan.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Scan.speed_min              = 0;
    Scan.speed_max              = 4;
    Scan.brightness_min         = 0;
    Scan.brightness_max         = 4;
    Scan.colors_min             = 0;
    Scan.colors_max             = 2;
    Scan.speed                  = 3;
    Scan.brightness             = 3;
    Scan.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Scan.colors.resize(2);
    modes.push_back(Scan);

    mode Contest;
    Contest.name                = "Contest";
    Contest.value               = UNIHUB_AL_LED_MODE_CONTEST;
    Contest.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    Contest.speed_min           = 0;
    Contest.speed_max           = 4;
    Contest.brightness_min      = 0;
    Contest.brightness_max      = 4;
    Contest.colors_min          = 0;
    Contest.colors_max          = 2;
    Contest.speed               = 3;
    Contest.brightness          = 3;
    Contest.direction           = UNIHUB_AL_LED_DIRECTION_LTR;
    Contest.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    Contest.colors.resize(3);
    modes.push_back(Contest);

    RGBController_LianLiUniHubAL::SetupZones();
}

RGBController_LianLiUniHubAL::~RGBController_LianLiUniHubAL()
{
    /*---------------------------------------------------------*\
    | Delete the matrix map                                     |
    \*---------------------------------------------------------*/
    for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        if(zones[zone_index].matrix_map != NULL)
        {
            delete zones[zone_index].matrix_map;
        }
    }

    delete controller;
}

void RGBController_LianLiUniHubAL::SetupZones()
{
    const std::string   zone_config     = "zones";
    SettingsManager*    set_man         = ResourceManager::get()->GetSettingsManager();
    json                device_settings = set_man->GetSettings(name);

    /*-------------------------------------------------*\
    | Get Linux LED settings from settings manager      |
    \*-------------------------------------------------*/
    if(!device_settings.contains(zone_config))
    {
        //If supported devices is not found then write it to settings
        device_settings[zone_config] = lianli_al_zones;
        set_man->SetSettings(name, device_settings);
        set_man->SaveSettings();
    }

    /*---------------------------------------------------------*\
    | Fill in zones from the device data                        |
    \*---------------------------------------------------------*/
    for(json::iterator it = device_settings[zone_config].begin(); it != device_settings[zone_config].end(); ++it)
    {
        if(zones.size() < LIANLI_AL_ZONES_MAX)
        {
            zone new_zone;

            new_zone.name               = it.key();
            unsigned char fan_count     = std::clamp(it.value().get<int>(), 0, LIANLI_AL_FANS_MAX);
            new_zone.type               = ZONE_TYPE_MATRIX;

            KeyboardLayoutManager new_kb(KEYBOARD_LAYOUT_DEFAULT, lianli_a1_fan.base_size);

            matrix_map_type * new_map   = new matrix_map_type;
            new_zone.matrix_map         = new_map;
            /*---------------------------------------------------------*\
            | Set up the matrix ready for each fan                      |
            \*---------------------------------------------------------*/
            new_kb.ChangeKeys(lianli_a1_empty.edit_keys);

            /*---------------------------------------------------------*\
            | For each fan in the config add a fan matrix               |
            \*---------------------------------------------------------*/
            keyboard_keymap_overlay fans = lianli_a1_fan;

            for(size_t fan_idx = 0; fan_idx < fan_count; fan_idx++)
            {
                new_kb.ChangeKeys(fans.edit_keys);

                for(size_t led_idx = 0; led_idx < fans.edit_keys.size(); led_idx++)
                {
                    fans.edit_keys.at(led_idx).col   += 9;
                    fans.edit_keys.at(led_idx).value += fans.edit_keys.size();
                }
            }

            /*---------------------------------------------------------*\
            | Matrix map still uses declared zone rows and columns      |
            |   as the packet structure depends on the matrix map       |
            \*---------------------------------------------------------*/
            new_map->height             = new_kb.GetRowCount();
            new_map->width              = new_kb.GetColumnCount();
            new_map->map                = new unsigned int[new_map->height * new_map->width];
            new_kb.GetKeyMap(new_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT, new_map->height, new_map->width);

            /*---------------------------------------------------------*\
            | Create LEDs for the Matrix zone                           |
            |   Place keys in the layout to populate the matrix         |
            \*---------------------------------------------------------*/
            new_zone.leds_count         = new_kb.GetKeyCount();
            LOG_DEBUG
            (
                "[%s] Created KB matrix with %d rows and %d columns containing %d keys",
                name.c_str(),
                new_kb.GetRowCount(),
                new_kb.GetColumnCount(),
                new_zone.leds_count
            );

            new_zone.leds_min           = new_zone.leds_count;
            new_zone.leds_max           = new_zone.leds_count;

            for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
            {
                led new_led;

                new_led.name                = new_kb.GetKeyNameAt(led_idx);
                new_led.value               = new_kb.GetKeyValueAt(led_idx);
                leds.push_back(new_led);
            }

            zones.push_back(new_zone);
        }
    }

    SetupColors();
}

void RGBController_LianLiUniHubAL::ResizeZone(int /* zone */, int /* new_size */)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_LianLiUniHubAL::DeviceUpdateLEDs()
{
    if(!initializedMode)
    {
        DeviceUpdateMode();
    }

    float brightness_scale = static_cast<float>(modes[active_mode].brightness)
                             / modes[active_mode].brightness_max;

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        controller->SetChannelLEDs
        (
            zone_idx,
            zones[zone_idx].colors,
            zones[zone_idx].leds,
            zones[zone_idx].leds_count,
            brightness_scale
        );
    }
}

void RGBController_LianLiUniHubAL::UpdateZoneLEDs(int zone)
{
    if(!initializedMode)
    {
        DeviceUpdateMode();
    }

    float brightness_scale = static_cast<float>(modes[active_mode].brightness)
                             / modes[active_mode].brightness_max;

    controller->SetChannelLEDs
    (
        zone,
        zones[zone].colors,
        zones[zone].leds,
        zones[zone].leds_count,
        brightness_scale
    );
}

void RGBController_LianLiUniHubAL::UpdateSingleLED(int /* led */)
{
    DeviceUpdateMode();
}

void RGBController_LianLiUniHubAL::DeviceUpdateMode()
{
    if(!active_mode)
    {
        return;                 // Do nothing, custom mode should go through DeviceUpdateLEDs() to avoid flooding controller
    }

    initializedMode             = true;

    int     fan_idx             = 0;
    bool    upd_both_fan_edge   = false;

    /*-----------------------------------------------------*\
    | Check modes that requires updating both arrays        |
    \*-----------------------------------------------------*/

    switch(modes[active_mode].value)
    {
        case UNIHUB_AL_LED_MODE_STATIC_COLOR:
        case UNIHUB_AL_LED_MODE_BREATHING:
        case UNIHUB_AL_LED_MODE_RUNWAY:
        case UNIHUB_AL_LED_MODE_METEOR:
            upd_both_fan_edge = true;
            break;
    }

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].leds_count == 0)
        {
            return;     // Do nothing, channel isn't in use
        }
        /*-----------------------------------------------------*\
        | Indexes start at 0                                    |
        \*-----------------------------------------------------*/
        fan_idx = ((zones[zone_idx].leds_count / lianli_a1_fan.edit_keys.size()) - 1);

        controller->SetChannelMode
        (
            zone_idx,
            modes[active_mode].value,
            modes[active_mode].colors,
            modes[active_mode].colors.size(),
            (fan_idx >= 0 ? fan_idx : 0),
            upd_both_fan_edge,
            modes[active_mode].brightness,
            modes[active_mode].speed,
            modes[active_mode].direction
        );
    }
}
