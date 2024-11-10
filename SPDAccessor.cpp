/*---------------------------------------------------------*\
| SPDAccessor.cpp                                           |
|                                                           |
|   Access to SPD information on various DIMMs              |
|                                                           |
|   Milan Cermak (krysmanta)                    09 Nov 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "SPDAccessor.h"

using namespace std::chrono_literals;

const char *spd_memory_type_name[] = {
    "Reserved", "FPM", "EDO", "Nibble", "SDR", "Multiplex ROM",
    "DDR", "DDR", "DDR2", "FB", "FB Probe", "DDR3", "DDR4",
    "Reserved", "DDR4e", "LPDDR3", "LPDDR4", "LPDDR4X",
    "DDR5", "LPDDR5"
};

SPDDetector::SPDDetector(i2c_smbus_interface *bus, uint8_t address, SPDMemoryType mem_type)
  : bus(bus), address(address), mem_type(mem_type), valid(false)
{
    detect_memory_type();
}

bool SPDDetector::is_valid() const
{
    return valid;
}

SPDMemoryType SPDDetector::memory_type() const
{
    return mem_type;
}

void SPDDetector::detect_memory_type()
{
    if(mem_type != SPD_DDR4_SDRAM && mem_type != SPD_DDR5_SDRAM)
    {
        LOG_DEBUG("Looking for an SPD Hub on address 0x%02x", address);
        int ddr5Magic = bus->i2c_smbus_read_byte_data(address, 0x00);
        int ddr5Sensor = bus->i2c_smbus_read_byte_data(address, 0x01);
        std::this_thread::sleep_for(1ms);

        if(ddr5Magic < 0 || ddr5Sensor < 0)
        {
            valid = false;
            return;
        }
        valid = true;

        if(ddr5Magic == 0x51 && (ddr5Sensor & 0xEF) == 0x08)
        {
            // These values are invalid for any other memory type
            mem_type = SPD_DDR5_SDRAM;
            return;
        }
    }

    if(mem_type != SPD_DDR4_SDRAM && mem_type != SPD_DDR5_SDRAM)
    {
        // Get memory type from SPD for DDR4 or older
        LOG_DEBUG("Getting memory type of slot at address 0x%02x", address);
        bus->i2c_smbus_write_byte_data(0x36, 0x00, 0xFF);
        std::this_thread::sleep_for(1ms);
        int value = bus->i2c_smbus_read_byte_data(address, 0x02);
        if(value < 0)
        {
            valid = false;
        }
        else
        {
            mem_type = (SPDMemoryType) value;
            valid = true;
        }
        return;
    }

    // We know what the memory type is, but not if the slot is occupied
    LOG_DEBUG("Probing slot at address 0x%02x", address);
    int value = bus->i2c_smbus_write_quick(address, 0x00);
    valid = (value < 0) ? false : true;
}

uint8_t SPDDetector::spd_address() const
{
    return this->address;
}

i2c_smbus_interface *SPDDetector::smbus() const
{
    return this->bus;
}

SPDWrapper::SPDWrapper(const SPDWrapper &wrapper)
{
    this->accessor = wrapper.accessor;
    this->address = wrapper.address;
    this->mem_type = wrapper.mem_type;
}

SPDWrapper::SPDWrapper(const SPDDetector &detector)
{
    this->address = detector.spd_address();
    this->mem_type = detector.memory_type();

    // Allocate a new accessor
    this->accessor = nullptr;
}

SPDWrapper::~SPDWrapper()
{
    delete accessor;
}

SPDMemoryType SPDWrapper::memory_type()
{
    return mem_type;
}

int SPDWrapper::index()
{
    return this->address - 0x50 + 1;
}

uint16_t SPDWrapper::jedec_id()
{
    if(accessor == nullptr)
    {
        return 0x0000;
    }
    return accessor->jedec_id();
}

bool is_jedec_in_slots(std::vector<SPDWrapper> &slots, uint16_t jedec_id)
{
    for(auto slot : slots)
    {
        if(slot.jedec_id() == jedec_id)
        {
            return true;
        }
    }
    return false;
}

std::vector<SPDWrapper*> slots_with_jedec(std::vector<SPDWrapper> &slots, uint16_t jedec_id)
{
    std::vector<SPDWrapper*> matching_slots;

    for(auto slot: slots)
    {
        if(slot.jedec_id() == jedec_id)
        {
            matching_slots.push_back(&slot);
        }
    }

    return matching_slots;
}
