/*---------------------------------------------------------*\
| SPDAccessor.h                                             |
|                                                           |
|   Access to SPD information on various DIMMs              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include "i2c_smbus.h"

typedef enum
{
    JEDEC_KINGSTON    = 0x0117,
    JEDEC_CORSAIR     = 0x021d,
    JEDEC_ADATA       = 0x044a,
    JEDEC_GSKILL      = 0x044c,
    JEDEC_TEAMGROUP   = 0x046e,
    JEDEC_MUSHKIN     = 0x8313,
    JEDEC_GIGABYTE    = 0x8971,
    JEDEC_THERMALTAKE = 0x8a41
} JedecIdentifier;

typedef enum
{
    SPD_RESERVED      =  0,
    SPD_FPM_DRAM      =  1,
    SPD_EDO           =  2,
    SPD_NIBBLE        =  3,
    SPD_SDR_SDRAM     =  4,
    SPD_MUX_ROM       =  5,
    SPD_DDR_SGRAM     =  6,
    SPD_DDR_SDRAM     =  7,
    SPD_DDR2_SDRAM    =  8,
    SPD_FB_DIMM       =  9,
    SPD_FB_PROBE      = 10,
    SPD_DDR3_SDRAM    = 11,
    SPD_DDR4_SDRAM    = 12,
    SPD_RESERVED2     = 13,
    SPD_DDR4E_SDRAM   = 14,
    SPD_LPDDR3_SDRAM  = 15,
    SPD_LPDDR4_SDRAM  = 16,
    SPD_LPDDR4X_SDRAM = 17,
    SPD_DDR5_SDRAM    = 18,
    SPD_LPDDR5_SDRAM  = 19
} SPDMemoryType;

extern const char *spd_memory_type_name[];

class SPDDetector
{
  public:
    SPDDetector(i2c_smbus_interface *bus, uint8_t address, SPDMemoryType mem_type);

    bool is_valid() const;
    SPDMemoryType memory_type() const;
    uint8_t spd_address() const;
    i2c_smbus_interface *smbus() const;

  private:
    i2c_smbus_interface *bus;
    uint8_t address;
    SPDMemoryType mem_type;
    bool valid;

    void detect_memory_type();
};

class SPDAccessor
{
  public:
    virtual uint16_t jedec_id() = 0;
};

class SPDWrapper
{
  public:
    SPDWrapper(const SPDWrapper &wrapper);
    SPDWrapper(const SPDDetector &detector);
    ~SPDWrapper();

    SPDMemoryType memory_type();
    int index();
    uint16_t jedec_id();

  private:
    SPDAccessor *accessor;
    uint8_t address;
    SPDMemoryType mem_type;
};

bool is_jedec_in_slots(std::vector<SPDWrapper> &slots, uint16_t jedec_id);
std::vector<SPDWrapper*> slots_with_jedec(std::vector<SPDWrapper> &slots, uint16_t jedec_id);
