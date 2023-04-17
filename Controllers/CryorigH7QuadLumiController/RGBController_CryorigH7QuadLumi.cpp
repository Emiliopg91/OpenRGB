/*-----------------------------------------*\
|  RGBController_CryorigH7QuadLumi.cpp      |
|                                           |
|  Generic RGB Interface for Cryorig H7     |
|  Quad Lumi                                |
|                                           |
|  Adam Honse (CalcProgrammer1) 4/15/2023   |
\*-----------------------------------------*/

#include "RGBController_CryorigH7QuadLumi.h"


RGBController_CryorigH7QuadLumi::RGBController_CryorigH7QuadLumi(CryorigH7QuadLumiController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "CRYORIG H7 Quad Lumi";
    vendor      = "CRYORIG";
    type        = DEVICE_TYPE_COOLER;
    description = "CRYORIG H7 Quad Lumi Device";
    version     = controller->GetFirmwareVersion();
    location    = controller->GetLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = CRYORIG_H7_QUAD_LUMI_MODE_FIXED;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    // mode Fading;
    // Fading.name       = "Fading";
    // Fading.value      = CRYORIG_H7_QUAD_LUMI_MODE_FADING;
    // Fading.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Fading.speed_min  = CRYORIG_H7_QUAD_LUMI_SPEED_SLOWEST;
    // Fading.speed_max  = CRYORIG_H7_QUAD_LUMI_SPEED_FASTEST;
    // Fading.colors_min = 1;
    // Fading.colors_max = 8;
    // Fading.speed      = CRYORIG_H7_QUAD_LUMI_SPEED_NORMAL;
    // Fading.color_mode = MODE_COLORS_MODE_SPECIFIC;
    // Fading.colors.resize(2);
    // modes.push_back(Fading);

    // mode SpectrumCycle;
    // SpectrumCycle.name       = "Spectrum Cycle";
    // SpectrumCycle.value      = CRYORIG_H7_QUAD_LUMI_MODE_SPECTRUM;
    // SpectrumCycle.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR;
    // SpectrumCycle.speed_min  = CRYORIG_H7_QUAD_LUMI_SPEED_SLOWEST;
    // SpectrumCycle.speed_max  = CRYORIG_H7_QUAD_LUMI_SPEED_FASTEST;
    // SpectrumCycle.speed      = CRYORIG_H7_QUAD_LUMI_SPEED_NORMAL;
    // SpectrumCycle.direction  = MODE_DIRECTION_RIGHT;
    // SpectrumCycle.color_mode = MODE_COLORS_NONE;
    // modes.push_back(SpectrumCycle);

    // mode Fading;
    // Fading.name              = "Fading";
    // Fading.value             = HUE_1_MODE_FADING;
    // Fading.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Fading.speed_min         = HUE_1_SPEED_SLOWEST;
    // Fading.speed_max         = HUE_1_SPEED_FASTEST;
    // Fading.colors_min        = 1;
    // Fading.colors_max        = 8;
    // Fading.speed             = HUE_1_SPEED_NORMAL;
    // Fading.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    // Fading.colors.resize(1);
    // modes.push_back(Fading);

    // mode SpectrumCycle;
    // SpectrumCycle.name       = "Spectrum Cycle";
    // SpectrumCycle.value      = HUE_1_MODE_SPECTRUM;
    // SpectrumCycle.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR;
    // SpectrumCycle.speed_min  = HUE_1_SPEED_SLOWEST;
    // SpectrumCycle.speed_max  = HUE_1_SPEED_FASTEST;
    // SpectrumCycle.speed      = HUE_1_SPEED_NORMAL;
    // SpectrumCycle.direction  = MODE_DIRECTION_RIGHT;
    // SpectrumCycle.color_mode = MODE_COLORS_NONE;
    // modes.push_back(SpectrumCycle);

    // mode Marquee;
    // Marquee.name             = "Marquee";
    // Marquee.value            = HUE_1_MODE_MARQUEE;
    // Marquee.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Marquee.speed_min        = HUE_1_SPEED_SLOWEST;
    // Marquee.speed_max        = HUE_1_SPEED_FASTEST;
    // Marquee.colors_min       = 1;
    // Marquee.colors_max       = 1;
    // Marquee.speed            = HUE_2_SPEED_NORMAL;
    // Marquee.direction        = MODE_DIRECTION_RIGHT;
    // Marquee.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    // Marquee.colors.resize(1);
    // modes.push_back(Marquee);

    // mode CoverMarquee;
    // CoverMarquee.name        = "Cover Marquee";
    // CoverMarquee.value       = HUE_1_MODE_COVER_MARQUEE;
    // CoverMarquee.flags       = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // CoverMarquee.speed_min   = HUE_1_SPEED_SLOWEST;
    // CoverMarquee.speed_max   = HUE_1_SPEED_FASTEST;
    // CoverMarquee.colors_min  = 1;
    // CoverMarquee.colors_max  = 8;
    // CoverMarquee.speed       = HUE_2_SPEED_NORMAL;
    // CoverMarquee.direction   = MODE_DIRECTION_RIGHT;
    // CoverMarquee.color_mode  = MODE_COLORS_MODE_SPECIFIC;
    // CoverMarquee.colors.resize(1);
    // modes.push_back(CoverMarquee);

    // mode Alternating;
    // Alternating.name         = "Alternating";
    // Alternating.value        = HUE_1_MODE_ALTERNATING;
    // Alternating.flags        = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Alternating.speed_min    = HUE_1_SPEED_SLOWEST;
    // Alternating.speed_max    = HUE_1_SPEED_FASTEST;
    // Alternating.colors_min   = 1;
    // Alternating.colors_max   = 2;
    // Alternating.speed        = HUE_1_SPEED_NORMAL;
    // Alternating.direction    = MODE_DIRECTION_RIGHT;
    // Alternating.color_mode   = MODE_COLORS_MODE_SPECIFIC;
    // Alternating.colors.resize(1);
    // modes.push_back(Alternating);

    // mode Pulsing;
    // Pulsing.name             = "Pulsing";
    // Pulsing.value            = HUE_1_MODE_PULSING;
    // Pulsing.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Pulsing.speed_min        = HUE_1_SPEED_SLOWEST;
    // Pulsing.speed_max        = HUE_1_SPEED_FASTEST;
    // Pulsing.colors_min       = 1;
    // Pulsing.colors_max       = 8;
    // Pulsing.speed            = HUE_1_SPEED_NORMAL;
    // Pulsing.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    // Pulsing.colors.resize(1) ;
    // modes.push_back(Pulsing);

    // mode Breathing;
    // Breathing.name           = "Breathing";
    // Breathing.value          = HUE_1_MODE_BREATHING;
    // Breathing.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR;
    // Breathing.speed_min      = HUE_1_SPEED_SLOWEST;
    // Breathing.speed_max      = HUE_1_SPEED_FASTEST;
    // Breathing.colors_min     = 1;
    // Breathing.colors_max     = 8;
    // Breathing.speed          = HUE_2_SPEED_NORMAL;
    // Breathing.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    // Breathing.colors.resize( 1);
    // modes.push_back(Breathing);

    // mode Candle;
    // Candle.name              = "Candle";
    // Candle.value             = HUE_1_MODE_CANDLE;
    // Candle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // Candle.speed_min         = HUE_1_SPEED_SLOWEST;
    // Candle.speed_max         = HUE_1_SPEED_FASTEST;
    // Candle.colors_min        = 1;
    // Candle.colors_max        = 8;
    // Candle.speed             = HUE_1_SPEED_NORMAL;
    // Candle.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    // Candle.colors.resize(1) ;
    // modes.push_back(Candle);

    // mode StarryNight;
    // StarryNight.name         = "Starry Night";
    // StarryNight.value        = HUE_1_MODE_STARRY_NIGHT;
    // StarryNight.flags        = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // StarryNight.speed_min    = HUE_1_SPEED_SLOWEST;
    // StarryNight.speed_max    = HUE_1_SPEED_FASTEST;
    // StarryNight.colors_min   = 1;
    // StarryNight.colors_max   = 1;
    // StarryNight.speed        = HUE_2_SPEED_NORMAL;
    // StarryNight.color_mode   = MODE_COLORS_MODE_SPECIFIC;
    // StarryNight.colors.resize(1);
    // modes.push_back(StarryNight);

    // mode SuperRainbow;
    // SuperRainbow.name        = "Super Rainbow";
    // SuperRainbow.value       = HUE_1_MODE_SUPER_RAINBOW;
    // SuperRainbow.flags       = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR;
    // SuperRainbow.speed_min   = HUE_1_SPEED_SLOWEST;
    // SuperRainbow.speed_max   = HUE_1_SPEED_FASTEST;
    // SuperRainbow.speed       = HUE_1_SPEED_NORMAL;
    // SuperRainbow.direction   = MODE_DIRECTION_RIGHT;
    // SuperRainbow.color_mode  = MODE_COLORS_NONE;
    // modes.push_back(SuperRainbow);

    // mode RainbowPulse;
    // RainbowPulse.name        = "Rainbow Pulse";
    // RainbowPulse.value       = HUE_1_MODE_RAINBOW_PULSE;
    // RainbowPulse.flags       = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR;
    // RainbowPulse.speed_min   = HUE_1_SPEED_SLOWEST;
    // RainbowPulse.speed_max   = HUE_1_SPEED_FASTEST;
    // RainbowPulse.speed       = HUE_1_SPEED_NORMAL;
    // RainbowPulse.direction   = MODE_DIRECTION_RIGHT;
    // RainbowPulse.color_mode  = MODE_COLORS_NONE;
    // modes.push_back(RainbowPulse);

    // mode RainbowFlow;
    // RainbowFlow.name         = "Rainbow Flow";
    // RainbowFlow.value        = HUE_1_MODE_RAINBOW_FLOW;
    // RainbowFlow.flags        = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR;
    // RainbowFlow.speed_min    = HUE_1_SPEED_SLOWEST;
    // RainbowFlow.speed_max    = HUE_1_SPEED_FASTEST;
    // RainbowFlow.speed        = HUE_1_SPEED_NORMAL;
    // RainbowFlow.direction    = MODE_DIRECTION_RIGHT;
    // RainbowFlow.color_mode   = MODE_COLORS_NONE;
    // modes.push_back(RainbowFlow);

    SetupZones();
}

RGBController_CryorigH7QuadLumi::~RGBController_CryorigH7QuadLumi()
{
    delete controller;
}

void RGBController_CryorigH7QuadLumi::SetupZones()
{
    const char* zone_names[] = { "Logo", "Underglow" };

    /*-------------------------------------------------*\
    | Set up zones                                      |
    \*-------------------------------------------------*/
    for(unsigned int zone_idx = 0; zone_idx < 2; zone_idx++)
    {
        zone* new_zone = new zone;

        new_zone->name          = zone_names[zone_idx];
        new_zone->type          = ZONE_TYPE_LINEAR;
        new_zone->leds_min      = 5;
        new_zone->leds_max      = 5;
        new_zone->leds_count    = 5;
        new_zone->matrix_map    = NULL;

        zones.push_back(*new_zone);
    }

    /*-------------------------------------------------*\
    | Set up LEDs                                       |
    \*-------------------------------------------------*/
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        for(unsigned int led_idx = 0; led_idx < zones[zone_idx].leds_count; led_idx++)
        {
            led new_led;
            new_led.name = zone_names[zone_idx];
            new_led.name.append(", LED ");
            new_led.name.append(std::to_string(led_idx + 1));
            new_led.value = zone_idx;

            leds.push_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_CryorigH7QuadLumi::ResizeZone(int /*zone*/, int /*new_size*/)
{

}

void RGBController_CryorigH7QuadLumi::DeviceUpdateLEDs()
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        controller->SetChannelLEDs(zone_idx, zones[zone_idx].colors, zones[zone_idx].leds_count);
    }
}

void RGBController_CryorigH7QuadLumi::UpdateZoneLEDs(int zone)
{
    controller->SetChannelLEDs(zone, zones[zone].colors, zones[zone].leds_count);
}

void RGBController_CryorigH7QuadLumi::UpdateSingleLED(int led)
{
    unsigned int zone_idx = leds[led].value;

    controller->SetChannelLEDs(zone_idx, zones[zone_idx].colors, zones[zone_idx].leds_count);
}

void RGBController_CryorigH7QuadLumi::SetCustomMode()
{
    active_mode = 0;
}

void RGBController_CryorigH7QuadLumi::DeviceUpdateMode()
{
    if(modes[active_mode].value == CRYORIG_H7_QUAD_LUMI_MODE_FIXED)
    {
        DeviceUpdateLEDs();
    }
    else
    {
        for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
        {
            RGBColor*   colors      = NULL;
            bool        direction   = false;

            if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
            {
                direction = true;
            }

            if(modes[active_mode].colors.size() > 0)
            {
                colors = &modes[active_mode].colors[0];
            }

            controller->SetChannelEffect
                    (
                    zone_idx,
                    modes[active_mode].value,
                    modes[active_mode].speed,
                    direction,
                    colors,
                    modes[active_mode].colors.size()
                    );
        }
    }
}