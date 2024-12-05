/*---------------------------------------------------------*\
| RGBController.cpp                                         |
|                                                           |
|   OpenRGB's RGB controller hardware abstration layer,     |
|   provides a generic representation of an RGB device      |
|                                                           |
|   Adam Honse (CalcProgrammer1)                02 Jun 2019 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstring>
#include <string>
#include <unordered_map>
#include "RGBController.h"

using namespace std::chrono_literals;

mode::mode()
{
    name           = "";
    value          = 0;
    flags          = 0;
    speed_min      = 0;
    speed_max      = 0;
    brightness_min = 0;
    brightness_max = 0;
    colors_min     = 0;
    colors_max     = 0;
    speed          = 0;
    brightness     = 0;
    direction      = 0;
    color_mode     = 0;
}

mode::~mode()
{
    colors.clear();
}

RGBController::RGBController()
{
    DeviceThreadRunning = true;
    DeviceCallThread = new std::thread(&RGBController::DeviceCallThreadFunction, this);
}

RGBController::~RGBController()
{
    DeviceThreadRunning = false;
    DeviceCallThread->join();
    delete DeviceCallThread;

    leds.clear();
    colors.clear();
    zones.clear();
    modes.clear();
}

unsigned char * RGBController::GetDeviceDescription(unsigned int protocol_version)
{
    unsigned int data_ptr = 0;
    unsigned int data_size = 0;

    /*---------------------------------------------------------*\
    | Calculate data size                                       |
    \*---------------------------------------------------------*/
    unsigned short name_len         = (unsigned short)strlen(name.c_str())        + 1;
    unsigned short vendor_len       = (unsigned short)strlen(vendor.c_str())      + 1;
    unsigned short description_len  = (unsigned short)strlen(description.c_str()) + 1;
    unsigned short version_len      = (unsigned short)strlen(version.c_str())     + 1;
    unsigned short serial_len       = (unsigned short)strlen(serial.c_str())      + 1;
    unsigned short location_len     = (unsigned short)strlen(location.c_str())    + 1;
    unsigned short num_modes        = (unsigned short)modes.size();
    unsigned short num_zones        = (unsigned short)zones.size();
    unsigned short num_leds         = (unsigned short)leds.size();
    unsigned short num_colors       = (unsigned short)colors.size();

    unsigned short *mode_name_len   = new unsigned short[num_modes];
    unsigned short *zone_name_len   = new unsigned short[num_zones];
    unsigned short *led_name_len    = new unsigned short[num_leds];

    unsigned short *zone_matrix_len = new unsigned short[num_zones];
    unsigned short *mode_num_colors = new unsigned short[num_modes];

    data_size += sizeof(data_size);
    data_size += sizeof(device_type);
    data_size += name_len           + sizeof(name_len);

    if(protocol_version >= 1)
    {
        data_size += vendor_len     + sizeof(vendor_len);
    }

    data_size += description_len    + sizeof(description_len);
    data_size += version_len        + sizeof(version_len);
    data_size += serial_len         + sizeof(serial_len);
    data_size += location_len       + sizeof(location_len);

    data_size += sizeof(num_modes);
    data_size += sizeof(active_mode);

    for(int mode_index = 0; mode_index < num_modes; mode_index++)
    {
        mode_name_len[mode_index]   = (unsigned short)strlen(modes[mode_index].name.c_str()) + 1;
        mode_num_colors[mode_index] = (unsigned short)modes[mode_index].colors.size();

        data_size += mode_name_len[mode_index] + sizeof(mode_name_len[mode_index]);
        data_size += sizeof(modes[mode_index].value);
        data_size += sizeof(modes[mode_index].flags);
        data_size += sizeof(modes[mode_index].speed_min);
        data_size += sizeof(modes[mode_index].speed_max);
        if(protocol_version >= 3)
        {
            data_size += sizeof(modes[mode_index].brightness_min);
            data_size += sizeof(modes[mode_index].brightness_max);
        }
        data_size += sizeof(modes[mode_index].colors_min);
        data_size += sizeof(modes[mode_index].colors_max);
        data_size += sizeof(modes[mode_index].speed);
        if(protocol_version >= 3)
        {
            data_size += sizeof(modes[mode_index].brightness);
        }
        data_size += sizeof(modes[mode_index].direction);
        data_size += sizeof(modes[mode_index].color_mode);
        data_size += sizeof(mode_num_colors[mode_index]);
        data_size += (mode_num_colors[mode_index] * sizeof(RGBColor));
    }

    data_size += sizeof(num_zones);

    for(int zone_index = 0; zone_index < num_zones; zone_index++)
    {
        zone_name_len[zone_index]   = (unsigned short)strlen(zones[zone_index].name.c_str()) + 1;

        data_size += zone_name_len[zone_index] + sizeof(zone_name_len[zone_index]);
        data_size += sizeof(zones[zone_index].type);
        data_size += sizeof(zones[zone_index].leds_min);
        data_size += sizeof(zones[zone_index].leds_max);
        data_size += sizeof(zones[zone_index].leds_count);

        if(zones[zone_index].matrix_map == NULL)
        {
            zone_matrix_len[zone_index] = 0;
        }
        else
        {
            zone_matrix_len[zone_index] = (unsigned short)(2 * sizeof(unsigned int)) + (zones[zone_index].matrix_map->height * zones[zone_index].matrix_map->width * sizeof(unsigned int));
        }

        data_size += sizeof(zone_matrix_len[zone_index]);
        data_size += zone_matrix_len[zone_index];

        if(protocol_version >= 4)
        {
            /*---------------------------------------------------------*\
            | Number of segments in zone                                |
            \*---------------------------------------------------------*/
            data_size += sizeof(unsigned short);

            for(size_t segment_index = 0; segment_index < zones[zone_index].segments.size(); segment_index++)
            {
                /*---------------------------------------------------------*\
                | Length of segment name string                             |
                \*---------------------------------------------------------*/
                data_size += sizeof(unsigned short);

                /*---------------------------------------------------------*\
                | Segment name string data                                  |
                \*---------------------------------------------------------*/
                data_size += (unsigned int)strlen(zones[zone_index].segments[segment_index].name.c_str()) + 1;

                data_size += sizeof(zones[zone_index].segments[segment_index].type);
                data_size += sizeof(zones[zone_index].segments[segment_index].start_idx);
                data_size += sizeof(zones[zone_index].segments[segment_index].leds_count);
            }
        }
    }

    data_size += sizeof(num_leds);

    for(int led_index = 0; led_index < num_leds; led_index++)
    {
        led_name_len[led_index] = (unsigned short)strlen(leds[led_index].name.c_str()) + 1;

        data_size += led_name_len[led_index] + sizeof(led_name_len[led_index]);

        data_size += sizeof(leds[led_index].value);
    }

    data_size += sizeof(num_colors);
    data_size += num_colors * sizeof(RGBColor);

    /*---------------------------------------------------------*\
    | Create data buffer                                        |
    \*---------------------------------------------------------*/
    unsigned char *data_buf = new unsigned char[data_size];

    /*---------------------------------------------------------*\
    | Copy in data size                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &data_size, sizeof(data_size));
    data_ptr += sizeof(data_size);

    /*---------------------------------------------------------*\
    | Copy in type                                              |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &type, sizeof(device_type));
    data_ptr += sizeof(device_type);

    /*---------------------------------------------------------*\
    | Copy in name (size+data)                                  |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &name_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], name.c_str());
    data_ptr += name_len;

    /*---------------------------------------------------------*\
    | Copy in vendor (size+data) if protocol 1 or higher        |
    \*---------------------------------------------------------*/
    if(protocol_version >= 1)
    {
        memcpy(&data_buf[data_ptr], &vendor_len, sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        strcpy((char *)&data_buf[data_ptr], vendor.c_str());
        data_ptr += vendor_len;
    }

    /*---------------------------------------------------------*\
    | Copy in description (size+data)                           |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &description_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], description.c_str());
    data_ptr += description_len;

    /*---------------------------------------------------------*\
    | Copy in version (size+data)                               |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &version_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], version.c_str());
    data_ptr += version_len;

    /*---------------------------------------------------------*\
    | Copy in serial (size+data)                                |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &serial_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], serial.c_str());
    data_ptr += serial_len;

    /*---------------------------------------------------------*\
    | Copy in location (size+data)                              |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &location_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], location.c_str());
    data_ptr += location_len;

    /*---------------------------------------------------------*\
    | Copy in number of modes (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_modes, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in active mode (data)                                |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &active_mode, sizeof(active_mode));
    data_ptr += sizeof(active_mode);

    /*---------------------------------------------------------*\
    | Copy in modes                                             |
    \*---------------------------------------------------------*/
    for(int mode_index = 0; mode_index < num_modes; mode_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in mode name (size+data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &mode_name_len[mode_index], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        strcpy((char *)&data_buf[data_ptr], modes[mode_index].name.c_str());
        data_ptr += mode_name_len[mode_index];

        /*---------------------------------------------------------*\
        | Copy in mode value (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].value, sizeof(modes[mode_index].value));
        data_ptr += sizeof(modes[mode_index].value);

        /*---------------------------------------------------------*\
        | Copy in mode flags (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].flags, sizeof(modes[mode_index].flags));
        data_ptr += sizeof(modes[mode_index].flags);

        /*---------------------------------------------------------*\
        | Copy in mode speed_min (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].speed_min, sizeof(modes[mode_index].speed_min));
        data_ptr += sizeof(modes[mode_index].speed_min);

        /*---------------------------------------------------------*\
        | Copy in mode speed_max (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].speed_max, sizeof(modes[mode_index].speed_max));
        data_ptr += sizeof(modes[mode_index].speed_max);

        /*---------------------------------------------------------*\
        | Copy in mode brightness_min and brightness_max (data) if  |
        | protocol 3 or higher                                      |
        \*---------------------------------------------------------*/
        if(protocol_version >= 3)
        {
            memcpy(&data_buf[data_ptr], &modes[mode_index].brightness_min, sizeof(modes[mode_index].brightness_min));
            data_ptr += sizeof(modes[mode_index].brightness_min);

            memcpy(&data_buf[data_ptr], &modes[mode_index].brightness_max, sizeof(modes[mode_index].brightness_max));
            data_ptr += sizeof(modes[mode_index].brightness_max);
        }

        /*---------------------------------------------------------*\
        | Copy in mode colors_min (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].colors_min, sizeof(modes[mode_index].colors_min));
        data_ptr += sizeof(modes[mode_index].colors_min);

        /*---------------------------------------------------------*\
        | Copy in mode colors_max (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].colors_max, sizeof(modes[mode_index].colors_max));
        data_ptr += sizeof(modes[mode_index].colors_max);

        /*---------------------------------------------------------*\
        | Copy in mode speed (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].speed, sizeof(modes[mode_index].speed));
        data_ptr += sizeof(modes[mode_index].speed);

        /*---------------------------------------------------------*\
        | Copy in mode brightness (data) if protocol 3 or higher    |
        \*---------------------------------------------------------*/
        if(protocol_version >= 3)
        {
            memcpy(&data_buf[data_ptr], &modes[mode_index].brightness, sizeof(modes[mode_index].brightness));
            data_ptr += sizeof(modes[mode_index].brightness);
        }

        /*---------------------------------------------------------*\
        | Copy in mode direction (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].direction, sizeof(modes[mode_index].direction));
        data_ptr += sizeof(modes[mode_index].direction);

        /*---------------------------------------------------------*\
        | Copy in mode color_mode (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode_index].color_mode, sizeof(modes[mode_index].color_mode));
        data_ptr += sizeof(modes[mode_index].color_mode);

        /*---------------------------------------------------------*\
        | Copy in mode number of colors                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &mode_num_colors[mode_index], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        /*---------------------------------------------------------*\
        | Copy in mode mode colors                                  |
        \*---------------------------------------------------------*/
        for(int color_index = 0; color_index < mode_num_colors[mode_index]; color_index++)
        {
            /*---------------------------------------------------------*\
            | Copy in color (data)                                      |
            \*---------------------------------------------------------*/
            memcpy(&data_buf[data_ptr], &modes[mode_index].colors[color_index], sizeof(modes[mode_index].colors[color_index]));
            data_ptr += sizeof(modes[mode_index].colors[color_index]);
        }
    }

    /*---------------------------------------------------------*\
    | Copy in number of zones (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_zones, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in zones                                             |
    \*---------------------------------------------------------*/
    for(int zone_index = 0; zone_index < num_zones; zone_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in zone name (size+data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zone_name_len[zone_index], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        strcpy((char *)&data_buf[data_ptr], zones[zone_index].name.c_str());
        data_ptr += zone_name_len[zone_index];

        /*---------------------------------------------------------*\
        | Copy in zone type (data)                                  |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zones[zone_index].type, sizeof(zones[zone_index].type));
        data_ptr += sizeof(zones[zone_index].type);

        /*---------------------------------------------------------*\
        | Copy in zone minimum LED count (data)                     |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zones[zone_index].leds_min, sizeof(zones[zone_index].leds_min));
        data_ptr += sizeof(zones[zone_index].leds_min);

        /*---------------------------------------------------------*\
        | Copy in zone maximum LED count (data)                     |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zones[zone_index].leds_max, sizeof(zones[zone_index].leds_max));
        data_ptr += sizeof(zones[zone_index].leds_max);

        /*---------------------------------------------------------*\
        | Copy in zone LED count (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zones[zone_index].leds_count, sizeof(zones[zone_index].leds_count));
        data_ptr += sizeof(zones[zone_index].leds_count);

        /*---------------------------------------------------------*\
        | Copy in size of zone matrix                               |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zone_matrix_len[zone_index], sizeof(zone_matrix_len[zone_index]));
        data_ptr += sizeof(zone_matrix_len[zone_index]);

        /*---------------------------------------------------------*\
        | Copy in matrix data if size is nonzero                    |
        \*---------------------------------------------------------*/
        if(zone_matrix_len[zone_index] > 0)
        {
            /*---------------------------------------------------------*\
            | Copy in matrix height                                     |
            \*---------------------------------------------------------*/
            memcpy(&data_buf[data_ptr], &zones[zone_index].matrix_map->height, sizeof(zones[zone_index].matrix_map->height));
            data_ptr += sizeof(zones[zone_index].matrix_map->height);

            /*---------------------------------------------------------*\
            | Copy in matrix width                                      |
            \*---------------------------------------------------------*/
            memcpy(&data_buf[data_ptr], &zones[zone_index].matrix_map->width, sizeof(zones[zone_index].matrix_map->width));
            data_ptr += sizeof(zones[zone_index].matrix_map->width);

            /*---------------------------------------------------------*\
            | Copy in matrix map                                        |
            \*---------------------------------------------------------*/
            for(unsigned int matrix_idx = 0; matrix_idx < (zones[zone_index].matrix_map->height * zones[zone_index].matrix_map->width); matrix_idx++)
            {
                memcpy(&data_buf[data_ptr], &zones[zone_index].matrix_map->map[matrix_idx], sizeof(zones[zone_index].matrix_map->map[matrix_idx]));
                data_ptr += sizeof(zones[zone_index].matrix_map->map[matrix_idx]);
            }
        }

        /*---------------------------------------------------------*\
        | Copy in segments                                          |
        \*---------------------------------------------------------*/
        if(protocol_version >= 4)
        {
            unsigned short num_segments = (unsigned short)zones[zone_index].segments.size();

            /*---------------------------------------------------------*\
            | Number of segments in zone                                |
            \*---------------------------------------------------------*/
            memcpy(&data_buf[data_ptr], &num_segments, sizeof(num_segments));
            data_ptr += sizeof(num_segments);

            for(int segment_index = 0; segment_index < num_segments; segment_index++)
            {
                /*---------------------------------------------------------*\
                | Length of segment name string                             |
                \*---------------------------------------------------------*/
                unsigned short segment_name_length = (unsigned short)strlen(zones[zone_index].segments[segment_index].name.c_str()) + 1;

                memcpy(&data_buf[data_ptr], &segment_name_length, sizeof(segment_name_length));
                data_ptr += sizeof(segment_name_length);

                /*---------------------------------------------------------*\
                | Segment name string data                                  |
                \*---------------------------------------------------------*/
                strcpy((char *)&data_buf[data_ptr], zones[zone_index].segments[segment_index].name.c_str());
                data_ptr += segment_name_length;

                /*---------------------------------------------------------*\
                | Segment type data                                         |
                \*---------------------------------------------------------*/
                memcpy(&data_buf[data_ptr], &zones[zone_index].segments[segment_index].type, sizeof(zones[zone_index].segments[segment_index].type));
                data_ptr += sizeof(zones[zone_index].segments[segment_index].type);

                /*---------------------------------------------------------*\
                | Segment start index data                                  |
                \*---------------------------------------------------------*/
                memcpy(&data_buf[data_ptr], &zones[zone_index].segments[segment_index].start_idx, sizeof(zones[zone_index].segments[segment_index].start_idx));
                data_ptr += sizeof(zones[zone_index].segments[segment_index].start_idx);

                /*---------------------------------------------------------*\
                | Segment LED count data                                  |
                \*---------------------------------------------------------*/
                memcpy(&data_buf[data_ptr], &zones[zone_index].segments[segment_index].leds_count, sizeof(zones[zone_index].segments[segment_index].leds_count));
                data_ptr += sizeof(zones[zone_index].segments[segment_index].leds_count);
            }
        }
    }

    /*---------------------------------------------------------*\
    | Copy in number of LEDs (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_leds, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in LEDs                                              |
    \*---------------------------------------------------------*/
    for(int led_index = 0; led_index < num_leds; led_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in LED name (size+data)                              |
        \*---------------------------------------------------------*/
        unsigned short ledname_len = (unsigned short)strlen(leds[led_index].name.c_str()) + 1;
        memcpy(&data_buf[data_ptr], &ledname_len, sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        strcpy((char *)&data_buf[data_ptr], leds[led_index].name.c_str());
        data_ptr += ledname_len;

        /*---------------------------------------------------------*\
        | Copy in LED value (data)                                  |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &leds[led_index].value, sizeof(leds[led_index].value));
        data_ptr += sizeof(leds[led_index].value);
    }

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_colors, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &colors[color_index], sizeof(colors[color_index]));
        data_ptr += sizeof(colors[color_index]);
    }

    delete[] mode_name_len;
    delete[] zone_name_len;
    delete[] led_name_len;

    delete[] zone_matrix_len;
    delete[] mode_num_colors;

    return(data_buf);
}

bool RGBController::ReadDeviceDescription(unsigned char* data_buf, unsigned int protocol_version)
{
    static const std::unordered_map<std::string, std::string> device_name_migrations {
        {"ASUS GTX 1060 Strix 6G Gaming", "ASUS ROG GTX 1060 Strix 6G Gaming"},
        {"ASUS GTX 1060 Strix", "ASUS ROG GTX 1060 Strix"},
        {"ASUS GTX 1070 Strix Gaming", "ASUS ROG GTX 1070 Strix Gaming"},
        {"ASUS GTX 1070 Strix OC", "ASUS ROG GTX 1070 Strix OC"},
        {"ASUS GTX 1080 Strix OC", "ASUS ROG GTX 1080 Strix OC"},
        {"ASUS KO RTX 3060Ti O8G GAMING", "ASUS KO RTX 3060 Ti O8G GAMING"},
        {"ASUS KO RTX 3060Ti O8G V2 GAMING", "ASUS KO RTX 3060 Ti O8G V2 GAMING"},
        {"ASUS ROG GTX 1660 Ti OC 6G", "ASUS ROG Strix GTX 1660 Ti OC 6G"},
        {"ASUS ROG STRIX 3060Ti O8G OC", "ASUS ROG STRIX 3060 Ti O8G OC"},
        {"ASUS ROG STRIX 3060Ti O8G V2", "ASUS ROG STRIX 3060 Ti O8G V2"},
        {"ASUS ROG STRIX 3070Ti O8G GAMING", "ASUS ROG STRIX 3070 Ti O8G GAMING"},
        {"ASUS ROG STRIX 3080Ti O12G GAMING", "ASUS ROG STRIX 3080 Ti O12G GAMING"},
        {"ASUS ROG Strix GTX 1050 TI 4G Gaming", "ASUS ROG Strix GTX 1050 Ti 4G Gaming"},
        {"ASUS ROG Strix GTX 1050 TI O4G Gaming", "ASUS ROG Strix GTX 1050 Ti O4G Gaming"},
        {"ASUS ROG Strix GTX 1050 TI O4G Gaming", "ASUS ROG Strix GTX 1050 Ti O4G Gaming"},
        {"ASUS ROG Strix GTX 1650S OC 4G", "ASUS ROG Strix GTX 1650 SUPER A4G Gaming"},
        {"ASUS ROG Strix GTX 1660S 6G Gaming", "ASUS ROG Strix GTX 1660 SUPER 6G Gaming"},
        {"ASUS ROG Strix GTX 1660S O6G Gaming", "ASUS ROG Strix GTX 1660 SUPER O6G Gaming"},
        {"ASUS ROG Strix GTX1070 Ti 8G Gaming", "ASUS ROG Strix GTX 1070 Ti 8G Gaming"},
        {"ASUS ROG Strix GTX1070 Ti A8G Gaming", "ASUS ROG Strix GTX 1070 Ti A8G Gaming"},
        {"ASUS ROG Strix GTX1080 A8G Gaming", "ASUS ROG Strix GTX 1080 A8G Gaming"},
        {"ASUS ROG Strix GTX1080 O8G 11Gbps", "ASUS ROG Strix GTX 1080 O8G 11Gbps"},
        {"ASUS ROG Strix GTX1080 O8G Gaming", "ASUS ROG Strix GTX 1080 O8G Gaming"},
        {"ASUS ROG Strix GTX1080 Ti 11G Gaming", "ASUS ROG Strix GTX 1080 Ti 11G Gaming"},
        {"ASUS ROG Strix GTX1080 Ti Gaming", "ASUS ROG Strix GTX 1080 Ti Gaming"},
        {"ASUS ROG Strix GTX1080 Ti O11G Gaming", "ASUS ROG Strix GTX 1080 Ti O11G Gaming"},
        {"ASUS ROG Strix GTX1080 Ti O11G Gaming", "ASUS ROG Strix GTX 1080 Ti O11G Gaming"},
        {"ASUS ROG STRIX LC 3080Ti O12G GAMING", "ASUS ROG STRIX LC 3080 Ti O12G GAMING"},
        {"ASUS ROG STRIX LC RTX 3090Ti O24G OC GAMING", "ASUS ROG STRIX LC RTX 3090 Ti O24G OC GAMING"},
        {"ASUS ROG STRIX LC RX 6800XT O16G GAMING", "ASUS ROG STRIX LC RX 6800 XT O16G GAMING"},
        {"ASUS ROG STRIX LC RX 6900XT O16G GAMING TOP", "ASUS ROG STRIX LC RX 6900 XT O16G GAMING TOP"},
        {"ASUS ROG STRIX LC RX 6900XT O16G GAMING", "ASUS ROG STRIX LC RX 6900 XT O16G GAMING"},
        {"ASUS ROG STRIX LC RX 6950XT O16G GAMING", "ASUS ROG STRIX LC RX 6950 XT O16G GAMING"},
        {"ASUS ROG STRIX RTX 2060S 8G Gaming", "ASUS ROG STRIX RTX 2060 SUPER 8G Gaming"},
        {"ASUS ROG STRIX RTX 2060S A8G EVO Gaming", "ASUS ROG STRIX RTX 2060 SUPER A8G EVO Gaming"},
        {"ASUS ROG STRIX RTX 2060S A8G Gaming", "ASUS ROG STRIX RTX 2060 SUPER A8G Gaming"},
        {"ASUS ROG STRIX RTX 2060S O8G Gaming", "ASUS ROG STRIX RTX 2060 SUPER O8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S 8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER 8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S A8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER A8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S A8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER A8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S A8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER A8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S O8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER O8G Gaming"},
        {"ASUS ROG STRIX RTX 2070S O8G Gaming", "ASUS ROG STRIX RTX 2070 SUPER O8G Gaming"},
        {"ASUS ROG STRIX RTX 2080S A8G Gaming", "ASUS ROG STRIX RTX 2080 SUPER A8G Gaming"},
        {"ASUS ROG STRIX RTX 2080S O8G Gaming", "ASUS ROG STRIX RTX 2080 SUPER O8G Gaming"},
        {"ASUS ROG STRIX RTX 2080S O8G White", "ASUS ROG STRIX RTX 2080 SUPER O8G White"},
        {"ASUS ROG STRIX RTX 4070 Super O12G Gaming", "ASUS ROG STRIX RTX 4070 SUPER O12G Gaming"},
        {"ASUS ROG STRIX RX 6700XT O12G GAMING", "ASUS ROG STRIX RX 6700 XT O12G GAMING"},
        {"ASUS ROG STRIX RX 6750XT O12G GAMING", "ASUS ROG STRIX RX 6750 XT O12G GAMING"},
        {"ASUS ROG STRIX RX470 O4G Gaming", "ASUS ROG STRIX RX 470 O4G Gaming"},
        {"ASUS ROG STRIX RX480 Gaming OC", "ASUS ROG STRIX RX 480 Gaming OC"},
        {"ASUS ROG STRIX RX560 Gaming", "ASUS ROG STRIX RX 560 Gaming"},
        {"ASUS RX 5600XT Strix O6G Gaming", "ASUS ROG RX 5600 XT Strix O6G Gaming"},
        {"ASUS RX 570 Strix O4G Gaming OC", "ASUS ROG RX 570 Strix O4G Gaming OC"},
        {"ASUS RX 570 Strix O8G Gaming OC", "ASUS ROG RX 570 Strix O8G Gaming OC"},
        {"ASUS RX 5700XT Strix 08G Gaming", "ASUS ROG RX 5700 XT Strix 08G Gaming"},
        {"ASUS RX 5700XT Strix Gaming OC", "ASUS ROG RX 5700 XT Strix Gaming OC"},
        {"ASUS RX 580 Strix Gaming OC", "ASUS ROG RX 580 Strix Gaming OC"},
        {"ASUS RX 580 Strix Gaming TOP", "ASUS ROG RX 580 Strix Gaming TOP"},
        {"ASUS TUF RTX 3060Ti O8G OC", "ASUS TUF RTX 3060 Ti O8G OC"},
        {"ASUS TUF RTX 3060Ti O8G OC", "ASUS TUF RTX 3060 Ti O8G OC"},
        {"ASUS TUF RTX 3060Ti O8G", "ASUS TUF RTX 3060 Ti O8G"},
        {"ASUS TUF RTX 3070Ti O8G GAMING", "ASUS TUF RTX 3070 Ti O8G GAMING"},
        {"ASUS TUF RTX 3070Ti O8G V2 GAMING", "ASUS TUF RTX 3070 Ti O8G V2 GAMING"},
        {"ASUS TUF RTX 3070Ti O8G V2 GAMING", "ASUS TUF RTX 3070 Ti O8G V2 GAMING"},
        {"ASUS TUF RTX 3070Ti O8G V2 GAMING", "ASUS TUF RTX 3070 Ti O8G V2 GAMING"},
        {"ASUS TUF RTX 3080Ti 12G GAMING", "ASUS TUF RTX 3080 Ti 12G GAMING"},
        {"ASUS TUF RTX 3080Ti O12G GAMING", "ASUS TUF RTX 3080 Ti O12G GAMING"},
        {"ASUS TUF RTX 3090Ti 24G GAMING", "ASUS TUF RTX 3090 Ti 24G GAMING"},
        {"ASUS TUF RTX 3090Ti O24G OC GAMING", "ASUS TUF RTX 3090 Ti O24G OC GAMING"},
        {"ASUS TUF RTX 4070 Super 12G Gaming", "ASUS TUF RTX 4070 SUPER 12G Gaming"},
        {"ASUS TUF RTX 4070 Ti Super 16G Gaming", "ASUS TUF RTX 4070 Ti SUPER 16G Gaming"},
        {"ASUS TUF RTX 4080 Super 16G GAMING", "ASUS TUF RTX 4080 SUPER 16G GAMING"},
        {"ASUS TUF RTX 4080 Super O16G GAMING", "ASUS TUF RTX 4080 SUPER O16G GAMING"},
        {"ASUS TUF RX 6700XT O12G GAMING", "ASUS TUF RX 6700 XT O12G GAMING"},
        {"ASUS TUF RX 6800XT O16G GAMING", "ASUS TUF RX 6800 XT O16G GAMING"},
        {"ASUS TUF RX 6900XT O16G GAMING", "ASUS TUF RX 6900 XT O16G GAMING"},
        {"ASUS TUF RX 6900XT T16G GAMING", "ASUS TUF RX 6900 XT T16G GAMING"},
        {"ASUS TUF RX 6950XT O16G GAMING", "ASUS TUF RX 6950 XT O16G GAMING"},
        {"ASUS TUF RX 7800XT GAMING OC", "ASUS TUF RX 7800 XT GAMING OC"},
        {"ASUS TUF RX 7800XT GAMING WHITE OC", "ASUS TUF RX 7800 XT GAMING WHITE OC"},
        {"ASUS TUF RX 7900XT 020G GAMING", "ASUS TUF RX 7900 XT 020G GAMING"},
        {"ASUS TUF RX 7900XTX O24G GAMING", "ASUS TUF RX 7900 XTX O24G GAMING"},
        {"ASUS Vega 64 Strix", "ASUS ROG Vega 64 Strix"},
        {"EVGA 1080Ti FTW3 Hybrid", "EVGA GeForce GTX 1080 Ti FTW3 Hybrid"},
        {"EVGA GeForce RTX 2080Ti Black", "EVGA GeForce RTX 2080 Ti Black"},
        {"EVGA GeForce RTX 2080Ti FTW3 Ultra", "EVGA GeForce RTX 2080 Ti FTW3 Ultra"},
        {"EVGA GeForce RTX 2080Ti XC HYBRID GAMING", "EVGA GeForce RTX 2080 Ti XC HYBRID GAMING"},
        {"EVGA GeForce RTX 2080Ti XC HYDRO COPPER", "EVGA GeForce RTX 2080 Ti XC HYDRO COPPER"},
        {"EVGA GeForce RTX 2080Ti XC Ultra", "EVGA GeForce RTX 2080 Ti XC Ultra"},
        {"EVGA GeForce RTX 3060TI FTW3 Gaming", "EVGA GeForce RTX 3060 Ti FTW3 Gaming"},
        {"EVGA GeForce RTX 3060TI FTW3 Ultra LHR", "EVGA GeForce RTX 3060 Ti FTW3 Ultra LHR"},
        {"EVGA GeForce RTX 3060TI FTW3 Ultra", "EVGA GeForce RTX 3060 Ti FTW3 Ultra"},
        {"EVGA GeForce RTX 3070Ti FTW3 Ultra v2", "EVGA GeForce RTX 3070 Ti FTW3 Ultra v2"},
        {"EVGA GeForce RTX 3070Ti FTW3 Ultra", "EVGA GeForce RTX 3070 Ti FTW3 Ultra"},
        {"EVGA GeForce RTX 3070Ti XC3 Gaming", "EVGA GeForce RTX 3070 Ti XC3 Gaming"},
        {"EVGA GeForce RTX 3070Ti XC3 Ultra v2", "EVGA GeForce RTX 3070 Ti XC3 Ultra v2"},
        {"EVGA GeForce RTX 3070Ti XC3 Ultra", "EVGA GeForce RTX 3070 Ti XC3 Ultra"},
        {"EVGA GeForce RTX 3080Ti FTW3 Ultra Hybrid", "EVGA GeForce RTX 3080 Ti FTW3 Ultra Hybrid"},
        {"EVGA GeForce RTX 3080Ti FTW3 Ultra Hydro Copper", "EVGA GeForce RTX 3080 Ti FTW3 Ultra Hydro Copper"},
        {"EVGA GeForce RTX 3080Ti FTW3 Ultra", "EVGA GeForce RTX 3080 Ti FTW3 Ultra"},
        {"EVGA GeForce RTX 3080Ti XC3 Gaming Hybrid", "EVGA GeForce RTX 3080 Ti XC3 Gaming Hybrid"},
        {"EVGA GeForce RTX 3080Ti XC3 Gaming Hydro Copper", "EVGA GeForce RTX 3080 Ti XC3 Gaming Hydro Copper"},
        {"EVGA GeForce RTX 3080Ti XC3 Gaming", "EVGA GeForce RTX 3080 Ti XC3 Gaming"},
        {"EVGA GeForce RTX 3080Ti XC3 Ultra Gaming", "EVGA GeForce RTX 3080 Ti XC3 Ultra Gaming"},
        {"EVGA GeForce RTX 3090Ti FTW3 Black Gaming", "EVGA GeForce RTX 3090 Ti FTW3 Black Gaming"},
        {"EVGA GeForce RTX 3090Ti FTW3 Gaming", "EVGA GeForce RTX 3090 Ti FTW3 Gaming"},
        {"EVGA GeForce RTX 3090Ti FTW3 Ultra Gaming", "EVGA GeForce RTX 3090 Ti FTW3 Ultra Gaming"},
        {"EVGA GTX 1080 Ti FTW3", "EVGA GeForce GTX 1080 Ti FTW3"},
        {"EVGA GTX 1080 Ti K|NGP|N", "EVGA GeForce GTX 1080 Ti K|NGP|N"},
        {"EVGA GTX 1080 Ti SC2 Gaming", "EVGA GeForce GTX 1080 Ti SC2 Gaming"},
        {"GALAX RTX 2070 Super EX Gamer Black", "GALAX RTX 2070 SUPER EX Gamer Black"},
        {"Gigabyte AORUS RTX2060 SUPER 8G V1", "Gigabyte AORUS RTX 2060 SUPER 8G V1"},
        {"Gigabyte AORUS RTX2070 SUPER 8G", "Gigabyte AORUS RTX 2070 SUPER 8G"},
        {"Gigabyte AORUS RTX2070 SUPER 8G", "Gigabyte AORUS RTX 2070 SUPER 8G"},
        {"Gigabyte AORUS RTX2070 XTREME 8G", "Gigabyte AORUS RTX 2070 XTREME 8G"},
        {"Gigabyte AORUS RTX2070 XTREME 8G", "Gigabyte AORUS RTX 2070 XTREME 8G"},
        {"Gigabyte AORUS RTX2080 8G", "Gigabyte AORUS RTX 2080 8G"},
        {"Gigabyte AORUS RTX2080 SUPER 8G Rev 1.0", "Gigabyte AORUS RTX 2080 SUPER 8G Rev 1.0"},
        {"Gigabyte AORUS RTX2080 SUPER 8G", "Gigabyte AORUS RTX 2080 SUPER 8G"},
        {"Gigabyte AORUS RTX2080 SUPER Waterforce WB 8G", "Gigabyte AORUS RTX 2080 SUPER Waterforce WB 8G"},
        {"Gigabyte AORUS RTX2080 SUPER Waterforce WB 8G", "Gigabyte AORUS RTX 2080 SUPER Waterforce WB 8G"},
        {"Gigabyte AORUS RTX2080 Ti XTREME 11G", "Gigabyte AORUS RTX 2080 Ti XTREME 11G"},
        {"Gigabyte AORUS RTX2080 XTREME 8G", "Gigabyte AORUS RTX 2080 XTREME 8G"},
        {"Gigabyte AORUS RTX3060 ELITE 12G LHR", "Gigabyte AORUS RTX 3060 ELITE 12G LHR"},
        {"Gigabyte AORUS RTX3060 ELITE 12G Rev a1", "Gigabyte AORUS RTX 3060 ELITE 12G Rev a1"},
        {"Gigabyte AORUS RTX3060 ELITE 12G", "Gigabyte AORUS RTX 3060 ELITE 12G"},
        {"Gigabyte AORUS RTX3060 Ti ELITE 8G LHR", "Gigabyte AORUS RTX 3060 Ti ELITE 8G LHR"},
        {"Gigabyte AORUS RTX3070 Ti MASTER 8G", "Gigabyte AORUS RTX 3070 Ti MASTER 8G"},
        {"Gigabyte AORUS RTX3080 Ti XTREME WATERFORCE 12G", "Gigabyte AORUS RTX 3080 Ti XTREME WATERFORCE 12G"},
        {"Gigabyte AORUS RTX3080 Ti XTREME WATERFORCE 12G", "Gigabyte AORUS RTX 3080 Ti XTREME WATERFORCE 12G"},
        {"Gigabyte AORUS RTX3080 XTREME WATERFORCE 10G Rev 2.0", "Gigabyte AORUS RTX 3080 XTREME WATERFORCE 10G Rev 2.0"},
        {"Gigabyte AORUS RTX3080 XTREME WATERFORCE WB 10G", "Gigabyte AORUS RTX 3080 XTREME WATERFORCE WB 10G"},
        {"Gigabyte AORUS RTX3080 XTREME WATERFORCE WB 10G", "Gigabyte AORUS RTX 3080 XTREME WATERFORCE WB 10G"},
        {"Gigabyte AORUS RTX3080 XTREME WATERFORCE WB 12G LHR", "Gigabyte AORUS RTX 3080 XTREME WATERFORCE WB 12G LHR"},
        {"Gigabyte AORUS RTX3090 XTREME WATERFORCE 24G", "Gigabyte AORUS RTX 3090 XTREME WATERFORCE 24G"},
        {"Gigabyte AORUS RTX3090 XTREME WATERFORCE WB 24G", "Gigabyte AORUS RTX 3090 XTREME WATERFORCE WB 24G"},
        {"Gigabyte AORUS RTX4080 MASTER 16G", "Gigabyte AORUS RTX 4080 MASTER 16G"},
        {"Gigabyte AORUS RTX4090 MASTER 24G", "Gigabyte AORUS RTX 4090 MASTER 24G"},
        {"Gigabyte GTX1050 Ti G1 Gaming (rev A1)", "Gigabyte GTX 1050 Ti G1 Gaming (rev A1)"},
        {"Gigabyte GTX1050 Ti G1 Gaming", "Gigabyte GTX 1050 Ti G1 Gaming"},
        {"Gigabyte GTX1060 G1 Gaming 6G OC", "Gigabyte GTX 1060 G1 Gaming 6G OC"},
        {"Gigabyte GTX1060 G1 Gaming 6G", "Gigabyte GTX 1060 G1 Gaming 6G"},
        {"Gigabyte GTX1060 Xtreme Gaming V1", "Gigabyte GTX 1060 Xtreme Gaming V1"},
        {"Gigabyte GTX1060 Xtreme Gaming v2", "Gigabyte GTX 1060 Xtreme Gaming v2"},
        {"Gigabyte GTX1070 G1 Gaming 8G V1", "Gigabyte GTX 1070 G1 Gaming 8G V1"},
        {"Gigabyte GTX1070 Ti 8G Gaming", "Gigabyte GTX 1070 Ti 8G Gaming"},
        {"Gigabyte GTX1070 Xtreme Gaming", "Gigabyte GTX 1070 Xtreme Gaming"},
        {"Gigabyte GTX1080 G1 Gaming", "Gigabyte GTX 1080 G1 Gaming"},
        {"Gigabyte GTX1080 Ti 11G", "Gigabyte GTX 1080 Ti 11G"},
        {"Gigabyte GTX1080 Ti Gaming OC 11G", "Gigabyte GTX 1080 Ti Gaming OC 11G"},
        {"Gigabyte GTX1080 Ti Gaming OC BLACK 11G", "Gigabyte GTX 1080 Ti Gaming OC BLACK 11G"},
        {"Gigabyte GTX1080 Ti Xtreme Edition", "Gigabyte GTX 1080 Ti Xtreme Edition"},
        {"Gigabyte GTX1080 Ti Xtreme Waterforce Edition", "Gigabyte GTX 1080 Ti Xtreme Waterforce Edition"},
        {"Gigabyte GTX1650 Gaming OC", "Gigabyte GTX 1650 Gaming OC"},
        {"Gigabyte GTX1660 Gaming OC 6G", "Gigabyte GTX 1660 Gaming OC 6G"},
        {"Gigabyte GTX1660 SUPER Gaming OC", "Gigabyte GTX 1660 SUPER Gaming OC"},
        {"Gigabyte RTX2060 Gaming OC PRO V2", "Gigabyte RTX 2060 Gaming OC PRO V2"},
        {"Gigabyte RTX2060 Gaming OC PRO White", "Gigabyte RTX 2060 Gaming OC PRO White"},
        {"Gigabyte RTX2060 Gaming OC PRO", "Gigabyte RTX 2060 Gaming OC PRO"},
        {"Gigabyte RTX2060 Gaming OC", "Gigabyte RTX 2060 Gaming OC"},
        {"Gigabyte RTX2060 SUPER Gaming OC 3X 8G V2", "Gigabyte RTX 2060 SUPER Gaming OC 3X 8G V2"},
        {"Gigabyte RTX2060 SUPER Gaming OC 3X White 8G", "Gigabyte RTX 2060 SUPER Gaming OC 3X White 8G"},
        {"Gigabyte RTX2060 SUPER Gaming OC", "Gigabyte RTX 2060 SUPER Gaming OC"},
        {"Gigabyte RTX2060 SUPER Gaming", "Gigabyte RTX 2060 SUPER Gaming"},
        {"Gigabyte RTX2070 Gaming OC 8G", "Gigabyte RTX 2070 Gaming OC 8G"},
        {"Gigabyte RTX2070 Gaming OC 8GC", "Gigabyte RTX 2070 Gaming OC 8GC"},
        {"Gigabyte RTX2070 Windforce 8G", "Gigabyte RTX 2070 Windforce 8G"},
        {"Gigabyte RTX2070S Gaming OC 3X White", "Gigabyte RTX 2070 SUPER Gaming OC 3X White"},
        {"Gigabyte RTX2070S Gaming OC 3X", "Gigabyte RTX 2070 SUPER Gaming OC 3X"},
        {"Gigabyte RTX2070S Gaming OC 3X", "Gigabyte RTX 2070 SUPER Gaming OC 3X"},
        {"Gigabyte RTX2070S Gaming OC", "Gigabyte RTX 2070 SUPER Gaming OC"},
        {"Gigabyte RTX2080 Gaming OC 8G", "Gigabyte RTX 2080 Gaming OC 8G"},
        {"Gigabyte RTX2080 Gaming OC 8G", "Gigabyte RTX 2080 Gaming OC 8G"},
        {"Gigabyte RTX2080 Ti GAMING OC 11G", "Gigabyte RTX 2080 Ti GAMING OC 11G"},
        {"Gigabyte RTX2080S Gaming OC 8G", "Gigabyte RTX 2080 SUPER Gaming OC 8G"},
        {"Gigabyte RTX3050 Gaming OC 8G", "Gigabyte RTX 3050 Gaming OC 8G"},
        {"Gigabyte RTX3060 EAGLE 12G LHR V2", "Gigabyte RTX 3060 EAGLE 12G LHR V2"},
        {"Gigabyte RTX3060 EAGLE OC 12G V2", "Gigabyte RTX 3060 EAGLE OC 12G V2"},
        {"Gigabyte RTX3060 EAGLE OC 12G", "Gigabyte RTX 3060 EAGLE OC 12G"},
        {"Gigabyte RTX3060 EAGLE OC 12G", "Gigabyte RTX 3060 EAGLE OC 12G"},
        {"Gigabyte RTX3060 Gaming OC 12G (rev. 2.0)", "Gigabyte RTX 3060 Gaming OC 12G (rev. 2.0)"},
        {"Gigabyte RTX3060 Gaming OC 12G", "Gigabyte RTX 3060 Gaming OC 12G"},
        {"Gigabyte RTX3060 Gaming OC 12G", "Gigabyte RTX 3060 Gaming OC 12G"},
        {"Gigabyte RTX3060 Ti EAGLE OC 8G V2.0 LHR", "Gigabyte RTX 3060 Ti EAGLE OC 8G V2.0 LHR"},
        {"Gigabyte RTX3060 Ti EAGLE OC 8G V2.0 LHR", "Gigabyte RTX 3060 Ti EAGLE OC 8G V2.0 LHR"},
        {"Gigabyte RTX3060 Ti EAGLE OC 8G", "Gigabyte RTX 3060 Ti EAGLE OC 8G"},
        {"Gigabyte RTX3060 Ti GAMING OC 8G", "Gigabyte RTX 3060 Ti GAMING OC 8G"},
        {"Gigabyte RTX3060 Ti GAMING OC 8G", "Gigabyte RTX 3060 Ti GAMING OC 8G"},
        {"Gigabyte RTX3060 Ti GAMING OC LHR 8G", "Gigabyte RTX 3060 Ti GAMING OC LHR 8G"},
        {"Gigabyte RTX3060 Ti Gaming OC PRO 8G LHR", "Gigabyte RTX 3060 Ti Gaming OC PRO 8G LHR"},
        {"Gigabyte RTX3060 Ti GAMING OC PRO 8G", "Gigabyte RTX 3060 Ti GAMING OC PRO 8G"},
        {"Gigabyte RTX3060 Ti Vision OC 8G", "Gigabyte RTX 3060 Ti Vision OC 8G"},
        {"Gigabyte RTX3060 Vision OC 12G LHR", "Gigabyte RTX 3060 Vision OC 12G LHR"},
        {"Gigabyte RTX3060 Vision OC 12G v3.0", "Gigabyte RTX 3060 Vision OC 12G v3.0"},
        {"Gigabyte RTX3060 Vision OC 12G", "Gigabyte RTX 3060 Vision OC 12G"},
        {"Gigabyte RTX3070 Eagle OC 8G V2.0 LHR", "Gigabyte RTX 3070 Eagle OC 8G V2.0 LHR"},
        {"Gigabyte RTX3070 Eagle OC 8G", "Gigabyte RTX 3070 Eagle OC 8G"},
        {"Gigabyte RTX3070 Gaming OC 8G v3.0 LHR", "Gigabyte RTX 3070 Gaming OC 8G v3.0 LHR"},
        {"Gigabyte RTX3070 Gaming OC 8G", "Gigabyte RTX 3070 Gaming OC 8G"},
        {"Gigabyte RTX3070 MASTER 8G LHR", "Gigabyte RTX 3070 MASTER 8G LHR"},
        {"Gigabyte RTX3070 MASTER 8G", "Gigabyte RTX 3070 MASTER 8G"},
        {"Gigabyte RTX3070 Ti EAGLE 8G", "Gigabyte RTX 3070 Ti EAGLE 8G"},
        {"Gigabyte RTX3070 Ti Gaming OC 8G", "Gigabyte RTX 3070 Ti Gaming OC 8G"},
        {"Gigabyte RTX3070 Ti Vision OC 8G", "Gigabyte RTX 3070 Ti Vision OC 8G"},
        {"Gigabyte RTX3070 Vision 8G V2.0 LHR", "Gigabyte RTX 3070 Vision 8G V2.0 LHR"},
        {"Gigabyte RTX3070 Vision 8G", "Gigabyte RTX 3070 Vision 8G"},
        {"Gigabyte RTX3080 EAGLE OC 10G", "Gigabyte RTX 3080 EAGLE OC 10G"},
        {"Gigabyte RTX3080 Gaming OC 10G", "Gigabyte RTX 3080 Gaming OC 10G"},
        {"Gigabyte RTX3080 Gaming OC 10G", "Gigabyte RTX 3080 Gaming OC 10G"},
        {"Gigabyte RTX3080 Gaming OC 12G", "Gigabyte RTX 3080 Gaming OC 12G"},
        {"Gigabyte RTX3080 Ti EAGLE 12G", "Gigabyte RTX 3080 Ti EAGLE 12G"},
        {"Gigabyte RTX3080 Ti EAGLE OC 12G", "Gigabyte RTX 3080 Ti EAGLE OC 12G"},
        {"Gigabyte RTX3080 Ti Gaming OC 12G", "Gigabyte RTX 3080 Ti Gaming OC 12G"},
        {"Gigabyte RTX3080 Ti Vision OC 12G", "Gigabyte RTX 3080 Ti Vision OC 12G"},
        {"Gigabyte RTX3080 Vision OC 10G (REV 2.0)", "Gigabyte RTX 3080 Vision OC 10G (REV 2.0)"},
        {"Gigabyte RTX3080 Vision OC 10G", "Gigabyte RTX 3080 Vision OC 10G"},
        {"Gigabyte RTX3090 Gaming OC 24G", "Gigabyte RTX 3090 Gaming OC 24G"},
        {"Gigabyte RTX3090 VISION OC 24G ", "Gigabyte RTX 3090 VISION OC 24G "},
        {"Gigabyte RTX4060 Gaming OC 8G", "Gigabyte RTX 4060 Gaming OC 8G"},
        {"Gigabyte RTX4060Ti Gaming OC 8G", "Gigabyte RTX 4060 Ti Gaming OC 8G"},
        {"Gigabyte RTX4070 Gaming OC 12G", "Gigabyte RTX 4070 Gaming OC 12G"},
        {"Gigabyte RTX4070 Super Aero OC 12G", "Gigabyte RTX 4070 SUPER Aero OC 12G"},
        {"Gigabyte RTX4070 SUPER Gaming OC 12G", "Gigabyte RTX 4070 SUPER Gaming OC 12G"},
        {"Gigabyte RTX4070Ti Gaming 12G", "Gigabyte RTX 4070 Ti Gaming 12G"},
        {"Gigabyte RTX4070Ti Gaming OC 12G", "Gigabyte RTX 4070 Ti Gaming OC 12G"},
        {"Gigabyte RTX4070Ti Gaming OC 12G", "Gigabyte RTX 4070 Ti Gaming OC 12G"},
        {"Gigabyte RTX4080 AERO OC 16G", "Gigabyte RTX 4080 AERO OC 16G"},
        {"Gigabyte RTX4080 Eagle OC 16G", "Gigabyte RTX 4080 Eagle OC 16G"},
        {"Gigabyte RTX4080 Gaming OC 16G", "Gigabyte RTX 4080 Gaming OC 16G"},
        {"Gigabyte RTX4080S Gaming OC 16G", "Gigabyte RTX 4080 SUPER Gaming OC 16G"},
        {"Gigabyte RTX4090 AERO OC 24G", "Gigabyte RTX 4090 AERO OC 24G"},
        {"Gigabyte RTX4090 GAMING OC 24G", "Gigabyte RTX 4090 GAMING OC 24G"},
        {"KFA2 RTX 2080 Super EX OC", "KFA2 RTX 2080 SUPER EX OC"},
        {"KFA2 RTX 2080 TI EX OC", "KFA2 RTX 2080 Ti EX OC"},
        {"MANLI 3080TI GALLARDO", "MANLI RTX 3080 Ti GALLARDO"},
        {"MSI GeForce GTX 1660 Super Gaming 6G", "MSI GeForce GTX 1660 SUPER Gaming 6G"},
        {"MSI GeForce GTX 1660 Super Gaming X 6G", "MSI GeForce GTX 1660 SUPER Gaming X 6G"},
        {"MSI GeForce GTX 1660Ti Gaming 6G", "MSI GeForce GTX 1660 Ti Gaming 6G"},
        {"MSI GeForce GTX 1660Ti Gaming X 6G", "MSI GeForce GTX 1660 Ti Gaming X 6G"},
        {"MSI GeForce RTX 2060 Super ARMOR OC", "MSI GeForce RTX 2060 SUPER ARMOR OC"},
        {"MSI GeForce RTX 2060 Super Gaming X", "MSI GeForce RTX 2060 SUPER Gaming X"},
        {"MSI GeForce RTX 2070 Super Gaming Trio", "MSI GeForce RTX 2070 SUPER Gaming Trio"},
        {"MSI GeForce RTX 2070 Super Gaming X Trio", "MSI GeForce RTX 2070 SUPER Gaming X Trio"},
        {"MSI GeForce RTX 2070 Super Gaming X", "MSI GeForce RTX 2070 SUPER Gaming X"},
        {"MSI GeForce RTX 2070 Super Gaming Z Trio", "MSI GeForce RTX 2070 SUPER Gaming Z Trio"},
        {"MSI GeForce RTX 2070 Super Gaming", "MSI GeForce RTX 2070 SUPER Gaming"},
        {"MSI GeForce RTX 2080 Super Gaming X Trio", "MSI GeForce RTX 2080 SUPER Gaming X Trio"},
        {"MSI GeForce RTX 2080Ti 11G Gaming X Trio", "MSI GeForce RTX 2080 Ti 11G Gaming X Trio"},
        {"MSI GeForce RTX 2080Ti Gaming X Trio", "MSI GeForce RTX 2080 Ti Gaming X Trio"},
        {"MSI GeForce RTX 2080Ti Gaming Z Trio", "MSI GeForce RTX 2080 Ti Gaming Z Trio"},
        {"MSI GeForce RTX 2080Ti Sea Hawk EK X", "MSI GeForce RTX 2080 Ti Sea Hawk EK X"},
        {"MSI GeForce RTX 4060Ti 16GB Gaming X", "MSI GeForce RTX 4060 Ti 16GB Gaming X"},
        {"MSI GeForce RTX 4060Ti 8GB Gaming X", "MSI GeForce RTX 4060 Ti 8GB Gaming X"},
        {"MSI GeForce RTX 4070 Super 12GB Gaming X Slim White", "MSI GeForce RTX 4070 SUPER 12GB Gaming X Slim White"},
        {"MSI GeForce RTX 4070 Super 12GB Gaming X Slim", "MSI GeForce RTX 4070 12GB Gaming X Slim"},
        {"MSI GeForce RTX 4070Ti 12GB Gaming X Slim White", "MSI GeForce RTX 4070 Ti 12GB Gaming X Slim White"},
        {"MSI GeForce RTX 4070Ti 12GB Gaming X Trio White", "MSI GeForce RTX 4070 Ti 12GB Gaming X Trio White"},
        {"MSI GeForce RTX 4070Ti 12GB Gaming X Trio", "MSI GeForce RTX 4070 Ti 12GB Gaming X Trio"},
        {"MSI GeForce RTX 4070Ti 12GB Suprim X Trio", "MSI GeForce RTX 4070 Ti 12GB Suprim X Trio"},
        {"MSI GeForce RTX 4070Ti Super 16GB Gaming Slim", "MSI GeForce RTX 4070 Ti SUPER 16GB Gaming Slim"},
        {"MSI GeForce RTX 4070Ti Super 16GB Gaming X Trio White", "MSI GeForce RTX 4070 Ti SUPER 16GB Gaming X Trio White"},
        {"MSI GeForce RTX 4080S 16GB Gaming X Slim", "MSI GeForce RTX 4080 SUPER 16GB Gaming X Slim"},
        {"MSI GeForce RTX 4080S 16GB Suprim X", "MSI GeForce RTX 4080 SUPER 16GB Suprim X"},
        {"NVIDIA 2070 SUPER FE", "NVIDIA RTX 2070 SUPER FE"},
        {"NVIDIA 2080 FE", "NVIDIA RTX 2080 FE"},
        {"NVIDIA 2080 FE", "NVIDIA RTX 2080 FE"},
        {"NVIDIA 3060TI LHR", "NVIDIA RTX 3060 Ti LHR"},
        {"NVIDIA 3060TI V1 LHR", "NVIDIA RTX 3060 Ti V1 LHR"},
        {"NVIDIA 3070Ti XLR8 Uprising EPIC-X", "NVIDIA RTX 3070 Ti XLR8 Uprising EPIC-X"},
        {"NVIDIA 3080 FE", "NVIDIA RTX 3080 FE"},
        {"NVIDIA 3080TI FE", "NVIDIA RTX 3080 Ti FE"},
        {"NVIDIA 3090 FE", "NVIDIA RTX 3090 FE"},
        {"NVIDIA 3090TI FE", "NVIDIA RTX 3090 Ti FE"},
        {"NVIDIA 4080 FE", "NVIDIA RTX 4080 FE"},
        {"NVIDIA 4080 FE", "NVIDIA RTX 4080 FE"},
        {"NVIDIA 4080S FE", "NVIDIA RTX 4080 SUPER FE"},
        {"NVIDIA 4090 FE", "NVIDIA RTX 4090 FE"},
        {"NVIDIA 4090 FE", "NVIDIA RTX 4090 FE"},
        {"NVIDIA GeForce RTX 3060", "Palit RTX 3060 LHR"},
        {"NVIDIA RTX2060S", "NVIDIA RTX 2060 SUPER"},
        {"NVIDIA RTX2080S", "NVIDIA RTX 2080 SUPER"},
        {"Palit 1060", "Palit GTX 1060"},
        {"Palit 1070 Ti", "Palit GTX 1070 Ti"},
        {"Palit 1070", "Palit GTX 1070"},
        {"Palit 1080 Ti", "Palit GTX 1080 Ti"},
        {"Palit 1080", "Palit GTX 1080"},
        {"Palit 3060 LHR (GA104)", "Palit RTX 3060 LHR (GA104)"},
        {"Palit 3060 LHR", "Palit RTX 3060 LHR"},
        {"Palit 3060", "Palit RTX 3060"},
        {"Palit 3060TI LHR", "Palit RTX 3060 Ti LHR"},
        {"Palit 3060Ti", "Palit RTX 3060 Ti"},
        {"Palit 3070 LHR", "Palit RTX 3070 LHR"},
        {"Palit 3070", "Palit RTX 3070"},
        {"Palit 3070Ti GamingPro", "Palit RTX 3070 Ti GamingPro"},
        {"Palit 3070Ti", "Palit RTX 3070 Ti"},
        {"Palit 3080 Gamerock LHR", "Palit RTX 3080 Gamerock LHR"},
        {"Palit 3080 Gamerock", "Palit RTX 3080 Gamerock"},
        {"Palit 3080 GamingPro 12G", "Palit RTX 3080 GamingPro 12G"},
        {"Palit 3080 LHR", "Palit RTX 3080 LHR"},
        {"Palit 3080", "Palit RTX 3080"},
        {"Palit 3080Ti Gamerock", "Palit RTX 3080 Ti Gamerock"},
        {"Palit 3080Ti", "Palit RTX 3080 Ti"},
        {"Palit 3090 Gamerock", "Palit RTX 3090 Gamerock"},
        {"Palit 3090", "Palit RTX 3090"},
        {"Palit 4070 SUPER Dual", "Palit RTX 4070 SUPER Dual"},
        {"Palit 4070", "Palit RTX 4070"},
        {"Palit 4070Ti Gamerock", "Palit RTX 4070 Ti Gamerock"},
        {"Palit 4080 GamingPro", "Palit RTX 4080 GamingPro"},
        {"Palit 4090 Gamerock", "Palit RTX 4090 Gamerock"},
        {"PNY 3060 XLR8 REVEL EPIC-X", "PNY RTX 3060 XLR8 REVEL EPIC-X"},
        {"PNY 3060 XLR8 REVEL EPIC-X", "PNY RTX 3060 XLR8 REVEL EPIC-X"},
        {"PNY 3060TI XLR8 REVEL EPIC-X", "PNY RTX 3060 Ti XLR8 REVEL EPIC-X"},
        {"PNY 3060TI XLR8 REVEL EPIC-X", "PNY RTX 3060 Ti XLR8 REVEL EPIC-X"},
        {"PNY 4070TI XLR8 VERTO Epic-X", "PNY RTX 4070 Ti XLR8 VERTO Epic-X"},
        {"PNY 4070TI XLR8 VERTO OC", "PNY RTX 4070 Ti XLR8 VERTO OC"},
        {"PNY 4070TI XLR8 VERTO REV1", "PNY RTX 4070 Ti XLR8 VERTO REV1"},
        {"PNY 4070TI XLR8 VERTO REV2", "PNY RTX 4070 Ti XLR8 VERTO REV2"},
        {"PNY 4080 Super XLR8 VERTO", "PNY RTX 4080 SUPER XLR8 VERTO"},
        {"PNY 4080 XLR8 UPRISING", "PNY RTX 4080 XLR8 UPRISING"},
        {"PNY 4080 XLR8 Verto Epic-X", "PNY RTX 4080 XLR8 Verto Epic-X"},
        {"PNY 4080 XLR8 VERTO", "PNY RTX 4080 XLR8 VERTO"},
        {"PNY 4090 XLR8 Verto Epic-X OC", "PNY RTX 4090 XLR8 Verto Epic-X OC"},
        {"PNY 4090 XLR8 Verto Epic-X", "PNY RTX 4090 XLR8 Verto Epic-X"},
        {"PNY 4090 XLR8 VERTO", "PNY RTX 4090 XLR8 VERTO"},
        {"PNY XLR8 OC EDITION RTX 2060", "PNY RTX 2060 XLR8 OC EDITION"},
        {"PNY XLR8 Revel EPIC-X RTX 3060", "PNY RTX 3060 XLR8 Revel EPIC-X"},
        {"PNY XLR8 Revel EPIC-X RTX 3070 LHR", "PNY RTX 3070 XLR8 Revel EPIC-X LHR"},
        {"PNY XLR8 Revel EPIC-X RTX 3070", "PNY RTX 3070 XLR8 Revel EPIC-X"},
        {"PNY XLR8 Revel EPIC-X RTX 3080", "PNY RTX 3080 XLR8 Revel EPIC-X"},
        {"PNY XLR8 Revel EPIC-X RTX 3090", "PNY RTX 3090 XLR8 Revel EPIC-X"},
    };

    unsigned int data_ptr = 0;
    bool needs_migration = false;

    data_ptr += sizeof(unsigned int);

    /*---------------------------------------------------------*\
    | Copy in type                                              |
    \*---------------------------------------------------------*/
    memcpy(&type, &data_buf[data_ptr], sizeof(device_type));
    data_ptr += sizeof(device_type);

    /*---------------------------------------------------------*\
    | Copy in name                                              |
    \*---------------------------------------------------------*/
    unsigned short name_len;
    memcpy(&name_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    name = (char *)&data_buf[data_ptr];
    data_ptr += name_len;

    /*---------------------------------------------------------*\
    | Check if the device has been renamed                      |
    \*---------------------------------------------------------*/
    std::unordered_map<std::string, std::string>::const_iterator new_name = device_name_migrations.find(name);
    if(new_name != device_name_migrations.end())
    {
        name = new_name->second;
        needs_migration = true;
    }

    /*---------------------------------------------------------*\
    | Copy in vendor if protocol version is 1 or higher         |
    \*---------------------------------------------------------*/
    if(protocol_version >= 1)
    {
        unsigned short vendor_len;
        memcpy(&vendor_len, &data_buf[data_ptr], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        vendor = (char *)&data_buf[data_ptr];
        data_ptr += vendor_len;
    }

    /*---------------------------------------------------------*\
    | Copy in description                                       |
    \*---------------------------------------------------------*/
    unsigned short description_len;
    memcpy(&description_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    description = (char *)&data_buf[data_ptr];
    data_ptr += description_len;

    /*---------------------------------------------------------*\
    | Copy in version                                           |
    \*---------------------------------------------------------*/
    unsigned short version_len;
    memcpy(&version_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    version = (char *)&data_buf[data_ptr];
    data_ptr += version_len;

    /*---------------------------------------------------------*\
    | Copy in serial                                            |
    \*---------------------------------------------------------*/
    unsigned short serial_len;
    memcpy(&serial_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    serial = (char *)&data_buf[data_ptr];
    data_ptr += serial_len;

    /*---------------------------------------------------------*\
    | Copy in location                                          |
    \*---------------------------------------------------------*/
    unsigned short location_len;
    memcpy(&location_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    location = (char *)&data_buf[data_ptr];
    data_ptr += location_len;

    /*---------------------------------------------------------*\
    | Copy in number of modes (data)                            |
    \*---------------------------------------------------------*/
    unsigned short num_modes;
    memcpy(&num_modes, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in active mode (data)                                |
    \*---------------------------------------------------------*/
    memcpy(&active_mode, &data_buf[data_ptr], sizeof(active_mode));
    data_ptr += sizeof(active_mode);

    /*---------------------------------------------------------*\
    | Copy in modes                                             |
    \*---------------------------------------------------------*/
    for(int mode_index = 0; mode_index < num_modes; mode_index++)
    {
        mode new_mode;

        /*---------------------------------------------------------*\
        | Copy in mode name (size+data)                             |
        \*---------------------------------------------------------*/
        unsigned short modename_len;
        memcpy(&modename_len, &data_buf[data_ptr], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        new_mode.name = (char *)&data_buf[data_ptr];
        data_ptr += modename_len;

        /*---------------------------------------------------------*\
        | Copy in mode value (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.value, &data_buf[data_ptr], sizeof(new_mode.value));
        data_ptr += sizeof(new_mode.value);

        /*---------------------------------------------------------*\
        | Copy in mode flags (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.flags, &data_buf[data_ptr], sizeof(new_mode.flags));
        data_ptr += sizeof(new_mode.flags);

        /*---------------------------------------------------------*\
        | Copy in mode speed_min (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.speed_min, &data_buf[data_ptr], sizeof(new_mode.speed_min));
        data_ptr += sizeof(new_mode.speed_min);

        /*---------------------------------------------------------*\
        | Copy in mode speed_max (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.speed_max, &data_buf[data_ptr], sizeof(new_mode.speed_max));
        data_ptr += sizeof(new_mode.speed_max);

        /*---------------------------------------------------------*\
        | Copy in mode brightness min and max if protocol version   |
        | is 3 or higher                                            |
        \*---------------------------------------------------------*/
        if(protocol_version >= 3)
        {
            memcpy(&new_mode.brightness_min, &data_buf[data_ptr], sizeof(new_mode.brightness_min));
            data_ptr += sizeof(new_mode.brightness_min);

            memcpy(&new_mode.brightness_max, &data_buf[data_ptr], sizeof(new_mode.brightness_max));
            data_ptr += sizeof(new_mode.brightness_max);
        }

        /*---------------------------------------------------------*\
        | Copy in mode colors_min (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.colors_min, &data_buf[data_ptr], sizeof(new_mode.colors_min));
        data_ptr += sizeof(new_mode.colors_min);

        /*---------------------------------------------------------*\
        | Copy in mode colors_max (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.colors_max, &data_buf[data_ptr], sizeof(new_mode.colors_max));
        data_ptr += sizeof(new_mode.colors_max);

        /*---------------------------------------------------------*\
        | Copy in mode speed (data)                                 |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.speed, &data_buf[data_ptr], sizeof(new_mode.speed));
        data_ptr += sizeof(new_mode.speed);

        /*---------------------------------------------------------*\
        | Copy in mode brightness if protocol version is 3 or higher|
        \*---------------------------------------------------------*/
        if(protocol_version >= 3)
        {
            memcpy(&new_mode.brightness, &data_buf[data_ptr], sizeof(new_mode.brightness));
            data_ptr += sizeof(new_mode.brightness);
        }

        /*---------------------------------------------------------*\
        | Copy in mode direction (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.direction, &data_buf[data_ptr], sizeof(new_mode.direction));
        data_ptr += sizeof(new_mode.direction);

        /*---------------------------------------------------------*\
        | Copy in mode color_mode (data)                            |
        \*---------------------------------------------------------*/
        memcpy(&new_mode.color_mode, &data_buf[data_ptr], sizeof(new_mode.color_mode));
        data_ptr += sizeof(new_mode.color_mode);

        /*---------------------------------------------------------*\
        | Copy in mode number of colors                             |
        \*---------------------------------------------------------*/
        unsigned short mode_num_colors;
        memcpy(&mode_num_colors, &data_buf[data_ptr], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        /*---------------------------------------------------------*\
        | Copy in mode mode colors                                  |
        \*---------------------------------------------------------*/
        for(int color_index = 0; color_index < mode_num_colors; color_index++)
        {
            /*---------------------------------------------------------*\
            | Copy in color (data)                                      |
            \*---------------------------------------------------------*/
            RGBColor new_color;
            memcpy(&new_color, &data_buf[data_ptr], sizeof(RGBColor));
            data_ptr += sizeof(modes[mode_index].colors[color_index]);

            new_mode.colors.push_back(new_color);
        }

        modes.push_back(new_mode);
    }

    /*---------------------------------------------------------*\
    | Copy in number of zones (data)                            |
    \*---------------------------------------------------------*/
    unsigned short num_zones;
    memcpy(&num_zones, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in zones                                             |
    \*---------------------------------------------------------*/
    for(int zone_index = 0; zone_index < num_zones; zone_index++)
    {
        zone new_zone;

        /*---------------------------------------------------------*\
        | Copy in zone name (size+data)                             |
        \*---------------------------------------------------------*/
        unsigned short zonename_len;
        memcpy(&zonename_len, &data_buf[data_ptr], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        new_zone.name = (char *)&data_buf[data_ptr];
        data_ptr += zonename_len;

        /*---------------------------------------------------------*\
        | Copy in zone type (data)                                  |
        \*---------------------------------------------------------*/
        memcpy(&new_zone.type, &data_buf[data_ptr], sizeof(new_zone.type));
        data_ptr += sizeof(new_zone.type);

        /*---------------------------------------------------------*\
        | Copy in zone minimum LED count (data)                     |
        \*---------------------------------------------------------*/
        memcpy(&new_zone.leds_min, &data_buf[data_ptr], sizeof(new_zone.leds_min));
        data_ptr += sizeof(new_zone.leds_min);

        /*---------------------------------------------------------*\
        | Copy in zone maximum LED count (data)                     |
        \*---------------------------------------------------------*/
        memcpy(&new_zone.leds_max, &data_buf[data_ptr], sizeof(new_zone.leds_max));
        data_ptr += sizeof(new_zone.leds_max);

        /*---------------------------------------------------------*\
        | Copy in zone LED count (data)                             |
        \*---------------------------------------------------------*/
        memcpy(&new_zone.leds_count, &data_buf[data_ptr], sizeof(new_zone.leds_count));
        data_ptr += sizeof(new_zone.leds_count);

        /*---------------------------------------------------------*\
        | Copy in size of zone matrix                               |
        \*---------------------------------------------------------*/
        unsigned short zone_matrix_len;
        memcpy(&zone_matrix_len, &data_buf[data_ptr], sizeof(zone_matrix_len));
        data_ptr += sizeof(zone_matrix_len);

        /*---------------------------------------------------------*\
        | Copy in matrix data if size is nonzero                    |
        \*---------------------------------------------------------*/
        if(zone_matrix_len > 0)
        {
            /*---------------------------------------------------------*\
            | Create a map data structure to fill in and attach it to   |
            | the new zone                                              |
            \*---------------------------------------------------------*/
            matrix_map_type * new_map = new matrix_map_type;

            new_zone.matrix_map = new_map;

            /*---------------------------------------------------------*\
            | Copy in matrix height                                     |
            \*---------------------------------------------------------*/
            memcpy(&new_map->height, &data_buf[data_ptr], sizeof(new_map->height));
            data_ptr += sizeof(new_map->height);

            /*---------------------------------------------------------*\
            | Copy in matrix width                                      |
            \*---------------------------------------------------------*/
            memcpy(&new_map->width, &data_buf[data_ptr], sizeof(new_map->width));
            data_ptr += sizeof(new_map->width);

            /*---------------------------------------------------------*\
            | Copy in matrix map                                        |
            \*---------------------------------------------------------*/
            new_map->map = new unsigned int[new_map->height * new_map->width];

            for(unsigned int matrix_idx = 0; matrix_idx < (new_map->height * new_map->width); matrix_idx++)
            {
                memcpy(&new_map->map[matrix_idx], &data_buf[data_ptr], sizeof(new_map->map[matrix_idx]));
                data_ptr += sizeof(new_map->map[matrix_idx]);
            }
        }
        else
        {
            new_zone.matrix_map = NULL;
        }

        /*---------------------------------------------------------*\
        | Copy in segments                                          |
        \*---------------------------------------------------------*/
        if(protocol_version >= 4)
        {
            unsigned short num_segments = 0;

            /*---------------------------------------------------------*\
            | Number of segments in zone                                |
            \*---------------------------------------------------------*/
            memcpy(&num_segments, &data_buf[data_ptr], sizeof(num_segments));
            data_ptr += sizeof(num_segments);

            for(int segment_index = 0; segment_index < num_segments; segment_index++)
            {
                segment new_segment;

                /*---------------------------------------------------------*\
                | Copy in segment name (size+data)                          |
                \*---------------------------------------------------------*/
                unsigned short segmentname_len;
                memcpy(&segmentname_len, &data_buf[data_ptr], sizeof(unsigned short));
                data_ptr += sizeof(unsigned short);

                new_segment.name = (char *)&data_buf[data_ptr];
                data_ptr += segmentname_len;

                /*---------------------------------------------------------*\
                | Segment type data                                         |
                \*---------------------------------------------------------*/
                memcpy(&new_segment.type, &data_buf[data_ptr], sizeof(new_segment.type));
                data_ptr += sizeof(new_segment.type);

                /*---------------------------------------------------------*\
                | Segment start index data                                  |
                \*---------------------------------------------------------*/
                memcpy(&new_segment.start_idx, &data_buf[data_ptr], sizeof(new_segment.start_idx));
                data_ptr += sizeof(new_segment.start_idx);

                /*---------------------------------------------------------*\
                | Segment LED count data                                    |
                \*---------------------------------------------------------*/
                memcpy(&new_segment.leds_count, &data_buf[data_ptr], sizeof(new_segment.leds_count));
                data_ptr += sizeof(new_segment.leds_count);

                new_zone.segments.push_back(new_segment);
            }
        }
        zones.push_back(new_zone);
    }

    /*---------------------------------------------------------*\
    | Copy in number of LEDs (data)                             |
    \*---------------------------------------------------------*/
    unsigned short num_leds;
    memcpy(&num_leds, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in LEDs                                              |
    \*---------------------------------------------------------*/
    for(int led_index = 0; led_index < num_leds; led_index++)
    {
        led new_led;

        /*---------------------------------------------------------*\
        | Copy in LED name (size+data)                              |
        \*---------------------------------------------------------*/
        unsigned short ledname_len;
        memcpy(&ledname_len, &data_buf[data_ptr], sizeof(unsigned short));
        data_ptr += sizeof(unsigned short);

        new_led.name = (char *)&data_buf[data_ptr];
        data_ptr += ledname_len;

        /*---------------------------------------------------------*\
        | Copy in LED value (data)                                  |
        \*---------------------------------------------------------*/
        memcpy(&new_led.value, &data_buf[data_ptr], sizeof(new_led.value));
        data_ptr += sizeof(new_led.value);

        leds.push_back(new_led);
    }

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    unsigned short num_colors;
    memcpy(&num_colors, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        RGBColor new_color;

        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&new_color, &data_buf[data_ptr], sizeof(RGBColor));
        data_ptr += sizeof(RGBColor);

        colors.push_back(new_color);
    }

    /*---------------------------------------------------------*\
    | Setup colors                                              |
    \*---------------------------------------------------------*/
    SetupColors();

    return needs_migration;
}

unsigned char * RGBController::GetModeDescription(int mode, unsigned int protocol_version)
{
    unsigned int data_ptr = 0;
    unsigned int data_size = 0;

    unsigned short mode_name_len;
    unsigned short mode_num_colors;

    /*---------------------------------------------------------*\
    | Calculate data size                                       |
    \*---------------------------------------------------------*/
    mode_name_len   = (unsigned short)strlen(modes[mode].name.c_str()) + 1;
    mode_num_colors = (unsigned short)modes[mode].colors.size();

    data_size += sizeof(data_size);
    data_size += sizeof(mode);
    data_size += sizeof(mode_name_len);
    data_size += mode_name_len;
    data_size += sizeof(modes[mode].value);
    data_size += sizeof(modes[mode].flags);
    data_size += sizeof(modes[mode].speed_min);
    data_size += sizeof(modes[mode].speed_max);
    if(protocol_version >= 3)
    {
        data_size += sizeof(modes[mode].brightness_min);
        data_size += sizeof(modes[mode].brightness_max);
    }
    data_size += sizeof(modes[mode].colors_min);
    data_size += sizeof(modes[mode].colors_max);
    data_size += sizeof(modes[mode].speed);
    if(protocol_version >= 3)
    {
        data_size += sizeof(modes[mode].brightness);
    }
    data_size += sizeof(modes[mode].direction);
    data_size += sizeof(modes[mode].color_mode);
    data_size += sizeof(mode_num_colors);
    data_size += (mode_num_colors * sizeof(RGBColor));

    /*---------------------------------------------------------*\
    | Create data buffer                                        |
    \*---------------------------------------------------------*/
    unsigned char *data_buf = new unsigned char[data_size];

    /*---------------------------------------------------------*\
    | Copy in data size                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &data_size, sizeof(data_size));
    data_ptr += sizeof(data_size);

    /*---------------------------------------------------------*\
    | Copy in mode index                                        |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &mode, sizeof(int));
    data_ptr += sizeof(int);

    /*---------------------------------------------------------*\
    | Copy in mode name (size+data)                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &mode_name_len, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    strcpy((char *)&data_buf[data_ptr], modes[mode].name.c_str());
    data_ptr += mode_name_len;

    /*---------------------------------------------------------*\
    | Copy in mode value (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].value, sizeof(modes[mode].value));
    data_ptr += sizeof(modes[mode].value);

    /*---------------------------------------------------------*\
    | Copy in mode flags (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].flags, sizeof(modes[mode].flags));
    data_ptr += sizeof(modes[mode].flags);

    /*---------------------------------------------------------*\
    | Copy in mode speed_min (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].speed_min, sizeof(modes[mode].speed_min));
    data_ptr += sizeof(modes[mode].speed_min);

    /*---------------------------------------------------------*\
    | Copy in mode speed_max (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].speed_max, sizeof(modes[mode].speed_max));
    data_ptr += sizeof(modes[mode].speed_max);

    /*---------------------------------------------------------*\
    | Copy in mode brightness min and max if protocol version   |
    | is 3 or higher                                            |
    \*---------------------------------------------------------*/
    if(protocol_version >= 3)
    {
        memcpy(&data_buf[data_ptr], &modes[mode].brightness_min, sizeof(modes[mode].brightness_min));
        data_ptr += sizeof(modes[mode].brightness_min);

        memcpy(&data_buf[data_ptr], &modes[mode].brightness_max, sizeof(modes[mode].brightness_max));
        data_ptr += sizeof(modes[mode].brightness_max);
    }

    /*---------------------------------------------------------*\
    | Copy in mode colors_min (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].colors_min, sizeof(modes[mode].colors_min));
    data_ptr += sizeof(modes[mode].colors_min);

    /*---------------------------------------------------------*\
    | Copy in mode colors_max (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].colors_max, sizeof(modes[mode].colors_max));
    data_ptr += sizeof(modes[mode].colors_max);

    /*---------------------------------------------------------*\
    | Copy in mode speed (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].speed, sizeof(modes[mode].speed));
    data_ptr += sizeof(modes[mode].speed);

    /*---------------------------------------------------------*\
    | Copy in mode brightness if protocol version is 3 or higher|
    \*---------------------------------------------------------*/
    if(protocol_version >= 3)
    {
        memcpy(&data_buf[data_ptr], &modes[mode].brightness, sizeof(modes[mode].brightness));
        data_ptr += sizeof(modes[mode].brightness);
    }

    /*---------------------------------------------------------*\
    | Copy in mode direction (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].direction, sizeof(modes[mode].direction));
    data_ptr += sizeof(modes[mode].direction);

    /*---------------------------------------------------------*\
    | Copy in mode color_mode (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &modes[mode].color_mode, sizeof(modes[mode].color_mode));
    data_ptr += sizeof(modes[mode].color_mode);

    /*---------------------------------------------------------*\
    | Copy in mode number of colors                             |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &mode_num_colors, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in mode mode colors                                  |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < mode_num_colors; color_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &modes[mode].colors[color_index], sizeof(modes[mode].colors[color_index]));
        data_ptr += sizeof(modes[mode].colors[color_index]);
    }

    return(data_buf);
}

void RGBController::SetModeDescription(unsigned char* data_buf, unsigned int protocol_version)
{
    int mode_idx;
    unsigned int data_ptr = sizeof(unsigned int);

    /*---------------------------------------------------------*\
    | Copy in mode index                                        |
    \*---------------------------------------------------------*/
    memcpy(&mode_idx, &data_buf[data_ptr], sizeof(int));
    data_ptr += sizeof(int);

    /*---------------------------------------------------------*\
    | Check if we aren't reading beyond the list of modes.      |
    \*---------------------------------------------------------*/
    if(((size_t) mode_idx) >  modes.size())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Get pointer to target mode                                |
    \*---------------------------------------------------------*/
    mode * new_mode = &modes[mode_idx];

    /*---------------------------------------------------------*\
    | Set active mode to the new mode                           |
    \*---------------------------------------------------------*/
    active_mode = mode_idx;

    /*---------------------------------------------------------*\
    | Copy in mode name (size+data)                             |
    \*---------------------------------------------------------*/
    unsigned short modename_len;
    memcpy(&modename_len, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    new_mode->name = (char *)&data_buf[data_ptr];
    data_ptr += modename_len;

    /*---------------------------------------------------------*\
    | Copy in mode value (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->value, &data_buf[data_ptr], sizeof(new_mode->value));
    data_ptr += sizeof(new_mode->value);

    /*---------------------------------------------------------*\
    | Copy in mode flags (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->flags, &data_buf[data_ptr], sizeof(new_mode->flags));
    data_ptr += sizeof(new_mode->flags);

    /*---------------------------------------------------------*\
    | Copy in mode speed_min (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->speed_min, &data_buf[data_ptr], sizeof(new_mode->speed_min));
    data_ptr += sizeof(new_mode->speed_min);

    /*---------------------------------------------------------*\
    | Copy in mode speed_max (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->speed_max, &data_buf[data_ptr], sizeof(new_mode->speed_max));
    data_ptr += sizeof(new_mode->speed_max);

    /*---------------------------------------------------------*\
    | Copy in mode brightness_min and brightness_max (data) if  |
    | protocol 3 or higher                                      |
    \*---------------------------------------------------------*/
    if(protocol_version >= 3)
    {
        memcpy(&new_mode->brightness_min, &data_buf[data_ptr], sizeof(new_mode->brightness_min));
        data_ptr += sizeof(new_mode->brightness_min);

        memcpy(&new_mode->brightness_max, &data_buf[data_ptr], sizeof(new_mode->brightness_max));
        data_ptr += sizeof(new_mode->brightness_max);
    }

    /*---------------------------------------------------------*\
    | Copy in mode colors_min (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->colors_min, &data_buf[data_ptr], sizeof(new_mode->colors_min));
    data_ptr += sizeof(new_mode->colors_min);

    /*---------------------------------------------------------*\
    | Copy in mode colors_max (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->colors_max, &data_buf[data_ptr], sizeof(new_mode->colors_max));
    data_ptr += sizeof(new_mode->colors_max);

    /*---------------------------------------------------------*\
    | Copy in mode speed (data)                                 |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->speed, &data_buf[data_ptr], sizeof(new_mode->speed));
    data_ptr += sizeof(new_mode->speed);

    /*---------------------------------------------------------*\
    | Copy in mode brightness (data) if protocol 3 or higher    |
    \*---------------------------------------------------------*/
    if(protocol_version >= 3)
    {
        memcpy(&new_mode->brightness, &data_buf[data_ptr], sizeof(new_mode->brightness));
        data_ptr += sizeof(new_mode->brightness);
    }

    /*---------------------------------------------------------*\
    | Copy in mode direction (data)                             |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->direction, &data_buf[data_ptr], sizeof(new_mode->direction));
    data_ptr += sizeof(new_mode->direction);

    /*---------------------------------------------------------*\
    | Copy in mode color_mode (data)                            |
    \*---------------------------------------------------------*/
    memcpy(&new_mode->color_mode, &data_buf[data_ptr], sizeof(new_mode->color_mode));
    data_ptr += sizeof(new_mode->color_mode);

    /*---------------------------------------------------------*\
    | Copy in mode number of colors                             |
    \*---------------------------------------------------------*/
    unsigned short mode_num_colors;
    memcpy(&mode_num_colors, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in mode mode colors                                  |
    \*---------------------------------------------------------*/
    new_mode->colors.clear();
    for(int color_index = 0; color_index < mode_num_colors; color_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        RGBColor new_color;
        memcpy(&new_color, &data_buf[data_ptr], sizeof(RGBColor));
        data_ptr += sizeof(RGBColor);

        new_mode->colors.push_back(new_color);
    }
}

unsigned char * RGBController::GetColorDescription()
{
    unsigned int data_ptr = 0;
    unsigned int data_size = 0;

    unsigned short num_colors = (unsigned short)colors.size();

    /*---------------------------------------------------------*\
    | Calculate data size                                       |
    \*---------------------------------------------------------*/
    data_size += sizeof(data_size);
    data_size += sizeof(num_colors);
    data_size += num_colors * sizeof(RGBColor);

    /*---------------------------------------------------------*\
    | Create data buffer                                        |
    \*---------------------------------------------------------*/
    unsigned char *data_buf = new unsigned char[data_size];

    /*---------------------------------------------------------*\
    | Copy in data size                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &data_size, sizeof(data_size));
    data_ptr += sizeof(data_size);

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_colors, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &colors[color_index], sizeof(colors[color_index]));
        data_ptr += sizeof(colors[color_index]);
    }

    return(data_buf);
}

void RGBController::SetColorDescription(unsigned char* data_buf)
{
    unsigned int data_ptr = sizeof(unsigned int);

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    unsigned short num_colors;
    memcpy(&num_colors, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Check if we aren't reading beyond the list of colors.     |
    \*---------------------------------------------------------*/
    if(((size_t)num_colors) > colors.size())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        RGBColor new_color;

        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&new_color, &data_buf[data_ptr], sizeof(RGBColor));
        data_ptr += sizeof(RGBColor);

        colors[color_index] = new_color;
    }
}

unsigned char * RGBController::GetZoneColorDescription(int zone)
{
    unsigned int data_ptr = 0;
    unsigned int data_size = 0;

    unsigned short num_colors = zones[zone].leds_count;

    /*---------------------------------------------------------*\
    | Calculate data size                                       |
    \*---------------------------------------------------------*/
    data_size += sizeof(data_size);
    data_size += sizeof(zone);
    data_size += sizeof(num_colors);
    data_size += num_colors * sizeof(RGBColor);

    /*---------------------------------------------------------*\
    | Create data buffer                                        |
    \*---------------------------------------------------------*/
    unsigned char *data_buf = new unsigned char[data_size];

    /*---------------------------------------------------------*\
    | Copy in data size                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &data_size, sizeof(data_size));
    data_ptr += sizeof(data_size);

    /*---------------------------------------------------------*\
    | Copy in zone index                                        |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &zone, sizeof(zone));
    data_ptr += sizeof(zone);

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[data_ptr], &num_colors, sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&data_buf[data_ptr], &zones[zone].colors[color_index], sizeof(zones[zone].colors[color_index]));
        data_ptr += sizeof(zones[zone].colors[color_index]);
    }

    return(data_buf);
}

void RGBController::SetZoneColorDescription(unsigned char* data_buf)
{
    unsigned int data_ptr = sizeof(unsigned int);
    unsigned int zone_idx;

    /*---------------------------------------------------------*\
    | Copy in zone index                                        |
    \*---------------------------------------------------------*/
    memcpy(&zone_idx, &data_buf[data_ptr], sizeof(zone_idx));
    data_ptr += sizeof(zone_idx);

    /*---------------------------------------------------------*\
    | Check if we aren't reading beyond the list of zones.      |
    \*---------------------------------------------------------*/
    if(((size_t)zone_idx) > zones.size())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Copy in number of colors (data)                           |
    \*---------------------------------------------------------*/
    unsigned short num_colors;
    memcpy(&num_colors, &data_buf[data_ptr], sizeof(unsigned short));
    data_ptr += sizeof(unsigned short);

    /*---------------------------------------------------------*\
    | Copy in colors                                            |
    \*---------------------------------------------------------*/
    for(int color_index = 0; color_index < num_colors; color_index++)
    {
        RGBColor new_color;

        /*---------------------------------------------------------*\
        | Copy in color (data)                                      |
        \*---------------------------------------------------------*/
        memcpy(&new_color, &data_buf[data_ptr], sizeof(RGBColor));
        data_ptr += sizeof(RGBColor);

        zones[zone_idx].colors[color_index] = new_color;
    }
}

unsigned char * RGBController::GetSingleLEDColorDescription(int led)
{
    /*---------------------------------------------------------*\
    | Fixed size descrption:                                    |
    |       int:      LED index                                 |
    |       RGBColor: LED color                                 |
    \*---------------------------------------------------------*/
    unsigned char *data_buf = new unsigned char[sizeof(int) + sizeof(RGBColor)];

    /*---------------------------------------------------------*\
    | Copy in LED index                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[0], &led, sizeof(int));

    /*---------------------------------------------------------*\
    | Copy in LED color                                         |
    \*---------------------------------------------------------*/
    memcpy(&data_buf[sizeof(led)], &colors[led], sizeof(RGBColor));

    return(data_buf);
}

void RGBController::SetSingleLEDColorDescription(unsigned char* data_buf)
{
    /*---------------------------------------------------------*\
    | Fixed size descrption:                                    |
    |       int:      LED index                                 |
    |       RGBColor: LED color                                 |
    \*---------------------------------------------------------*/
    int led_idx;

    /*---------------------------------------------------------*\
    | Copy in LED index                                         |
    \*---------------------------------------------------------*/
    memcpy(&led_idx, &data_buf[0], sizeof(led_idx));

    /*---------------------------------------------------------*\
    | Check if we aren't reading beyond the list of leds.       |
    \*---------------------------------------------------------*/
    if(((size_t)led_idx) > leds.size())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Copy in LED color                                         |
    \*---------------------------------------------------------*/
    memcpy(&colors[led_idx], &data_buf[sizeof(led_idx)], sizeof(RGBColor));
}

void RGBController::SetupColors()
{
    unsigned int total_led_count;

    /*---------------------------------------------------------*\
    | Determine total number of LEDs on the device              |
    \*---------------------------------------------------------*/
    total_led_count = 0;

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        total_led_count += zones[zone_idx].leds_count;
    }

    /*---------------------------------------------------------*\
    | Set the size of the color buffer to the number of LEDs    |
    \*---------------------------------------------------------*/
    colors.resize(total_led_count);

    /*---------------------------------------------------------*\
    | Set the color buffer pointers on each zone                |
    \*---------------------------------------------------------*/
    total_led_count = 0;

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        zones[zone_idx].start_idx=total_led_count;

        if((colors.size() > 0) && (zones[zone_idx].leds_count > 0))
        {
            zones[zone_idx].colors = &colors[total_led_count];
        }
        else
        {
            zones[zone_idx].colors = NULL;
        }

        if((leds.size() > 0) && (zones[zone_idx].leds_count > 0))
        {
            zones[zone_idx].leds   = &leds[total_led_count];
        }
        else
        {
            zones[zone_idx].leds    = NULL;
        }


        total_led_count += zones[zone_idx].leds_count;
    }
}

RGBColor RGBController::GetLED(unsigned int led)
{
    if(led < colors.size())
    {
        return(colors[led]);
    }
    else
    {
        return(0x00000000);
    }
}

void RGBController::SetLED(unsigned int led, RGBColor color)
{
    if(led < colors.size())
    {
        colors[led] = color;
    }
}

void RGBController::SetAllLEDs(RGBColor color)
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        SetAllZoneLEDs((int)zone_idx, color);
    }
}

void RGBController::SetAllZoneLEDs(int zone, RGBColor color)
{
    for (std::size_t color_idx = 0; color_idx < zones[zone].leds_count; color_idx++)
    {
        zones[zone].colors[color_idx] = color;
    }
}

int RGBController::GetMode()
{
    return(active_mode);
}

void RGBController::SetMode(int mode)
{
    active_mode = mode;

    UpdateMode();
}

void RGBController::RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg)
{
    UpdateCallbacks.push_back(new_callback);
    UpdateCallbackArgs.push_back(new_callback_arg);
}

void RGBController::UnregisterUpdateCallback(void * callback_arg)
{
    for(unsigned int callback_idx = 0; callback_idx < UpdateCallbackArgs.size(); callback_idx++ )
    {
        if(UpdateCallbackArgs[callback_idx] == callback_arg)
        {
            UpdateCallbackArgs.erase(UpdateCallbackArgs.begin() + callback_idx);
            UpdateCallbacks.erase(UpdateCallbacks.begin() + callback_idx);

            break;
        }
    }
}

void RGBController::ClearCallbacks()
{
    UpdateCallbacks.clear();
    UpdateCallbackArgs.clear();
}

void RGBController::SignalUpdate()
{
    UpdateMutex.lock();

    /*-------------------------------------------------*\
    | Client info has changed, call the callbacks       |
    \*-------------------------------------------------*/
    for(unsigned int callback_idx = 0; callback_idx < UpdateCallbacks.size(); callback_idx++)
    {
        UpdateCallbacks[callback_idx](UpdateCallbackArgs[callback_idx]);
    }

    UpdateMutex.unlock();
}
void RGBController::UpdateLEDs()
{
    CallFlag_UpdateLEDs = true;

    SignalUpdate();
}

void RGBController::UpdateMode()
{
    CallFlag_UpdateMode = true;
}

void RGBController::SaveMode()
{
    DeviceSaveMode();
}

void RGBController::DeviceUpdateLEDs()
{

}

void RGBController::SetCustomMode()
{
    /*-------------------------------------------------*\
    | Search the Controller's mode list for a suitable  |
    | per-LED custom mode in the following order:       |
    | 1.    Direct                                      |
    | 2.    Custom                                      |
    | 3.    Static                                      |
    \*-------------------------------------------------*/
    #define NUM_CUSTOM_MODE_NAMES 3

    const std::string custom_mode_names[] =
    {
        "Direct",
        "Custom",
        "Static"
    };

    for(unsigned int custom_mode_idx = 0; custom_mode_idx < NUM_CUSTOM_MODE_NAMES; custom_mode_idx++)
    {
        for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
        {
            if((modes[mode_idx].name == custom_mode_names[custom_mode_idx])
            && ((modes[mode_idx].color_mode == MODE_COLORS_PER_LED)
             || (modes[mode_idx].color_mode == MODE_COLORS_MODE_SPECIFIC)))
            {
                active_mode = mode_idx;
                return;
            }
        }
    }
}

void RGBController::DeviceUpdateMode()
{

}

void RGBController::DeviceCallThreadFunction()
{
    CallFlag_UpdateLEDs = false;
    CallFlag_UpdateMode = false;

    while(DeviceThreadRunning.load() == true)
    {
        if(CallFlag_UpdateMode.load() == true)
        {
            DeviceUpdateMode();
            CallFlag_UpdateMode = false;
        }
        if(CallFlag_UpdateLEDs.load() == true)
        {
            DeviceUpdateLEDs();
            CallFlag_UpdateLEDs = false;
        }
        else
        {
           std::this_thread::sleep_for(1ms);
        }
    }
}

void RGBController::DeviceSaveMode()
{
    /*-------------------------------------------------*\
    | If not implemented by controller, does nothing    |
    \*-------------------------------------------------*/
}

std::string device_type_to_str(device_type type)
{
    switch(type)
    {
    case DEVICE_TYPE_MOTHERBOARD:
        return "Motherboard";
    case DEVICE_TYPE_DRAM:
        return "DRAM";
    case DEVICE_TYPE_GPU:
        return "GPU";
    case DEVICE_TYPE_COOLER:
        return "Cooler";
    case DEVICE_TYPE_LEDSTRIP:
        return "LED Strip";
    case DEVICE_TYPE_KEYBOARD:
        return "Keyboard";
    case DEVICE_TYPE_MOUSE:
        return "Mouse";
    case DEVICE_TYPE_MOUSEMAT:
        return "Mousemat";
    case DEVICE_TYPE_HEADSET:
        return "Headset";
    case DEVICE_TYPE_HEADSET_STAND:
        return "Headset Stand";
    case DEVICE_TYPE_GAMEPAD:
        return "Gamepad";
    case DEVICE_TYPE_LIGHT:
        return "Light";
    case DEVICE_TYPE_SPEAKER:
        return "Speaker";
    case DEVICE_TYPE_STORAGE:
        return "Storage";
    case DEVICE_TYPE_VIRTUAL:
        return "Virtual";
    case DEVICE_TYPE_CASE:
        return "Case";
    case DEVICE_TYPE_ACCESSORY:
        return "Accessory";
    case DEVICE_TYPE_KEYPAD:
        return "Keypad";
    default:
        return "Unknown";
    }
}
