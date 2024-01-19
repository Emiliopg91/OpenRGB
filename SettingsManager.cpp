/*---------------------------------------------------------*\
| SettingsManager.cpp                                       |
|                                                           |
|   OpenRGB Settings Manager maintains a list of application|
|   settings in JSON format.  Other components may register |
|   settings with this class and store/load values.         |
|                                                           |
|   Adam Honse (CalcProgrammer1)                04 Nov 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <fstream>
#include <iostream>
#include "SettingsManager.h"
#include "LogManager.h"

SettingsManager::SettingsManager()
{
    firstrun = true;
    config_found = false;
    backup_needed = true;
}

SettingsManager::~SettingsManager()
{

}

json SettingsManager::GetSettings(std::string settings_key)
{
    /*---------------------------------------------------------*\
    | Check to see if the key exists in the settings store and  |
    | return the settings associated with the key if it exists  |
    | We lock the mutex to protect the value from changing      |
    | while data is being read and copy before unlocking        |
    \*---------------------------------------------------------*/
    json result;

    mutex.lock();
    if(settings_data.contains(settings_key))
    {
        result = settings_data[settings_key];
    }

    mutex.unlock();

    return result;
}

void SettingsManager::SetSettings(std::string settings_key, json new_settings)
{
    mutex.lock();
    settings_data[settings_key] = new_settings;
    mutex.unlock();
}

void SettingsManager::LoadSettings(const filesystem::path& filename)
{
    mutex.lock();

    /*---------------------------------------------------------*\
    | Store settings filename, so we can save to it later       |
    \*---------------------------------------------------------*/
    settings_filename = filename;

    /*---------------------------------------------------------*\
    | Open input file in binary mode                            |
    \*---------------------------------------------------------*/
    config_found = filesystem::exists(filename);
    if(config_found)
    {
        /*---------------------------------------------------------*\
        | Clear any stored settings before loading                  |
        \*---------------------------------------------------------*/
        settings_data.clear();

        firstrun = false;
        std::ifstream settings_file(settings_filename, std::ios::in | std::ios::binary);

        /*---------------------------------------------------------*\
        | Read settings into JSON store                             |
        \*---------------------------------------------------------*/
        if(settings_file)
        {
            try
            {
                settings_file >> settings_data;
            }
            catch(const std::exception& e)
            {
                /*-------------------------------------------------*\
                | If an exception was caught, that means the JSON   |
                | parsing failed.  Clear out any data in the store  |
                | as it is corrupt.                                 |
                | We could attempt a reload for backup location     |
                \*-------------------------------------------------*/
                LOG_ERROR("[SettingsManager] JSON parsing failed: %s", e.what());
                filesystem::path corrupted_name = settings_filename;
                corrupted_name.append("_corrupted.txt");
                LOG_DEBUG("[SettingsManager] Preserving the corrupted file as [%s]", corrupted_name.u8string().c_str());
                filesystem::rename(settings_filename, corrupted_name);

                settings_data.clear();
            }
        }
        else
        {
            LOG_ERROR("[SettingsManager] Settings file is found, but an unknown error has occured when reading it");
        }

        settings_file.close();
    }
    else
    {
        backup_needed = false;
        if(firstrun)
        {
            LOG_DEBUG("[SettingsManager] Application is running for the first time");
        }
        else
        {
            LOG_ERROR("[SettingsManager] The config DID exist, but went missing; we preserved the original settings to attempt to store them again");
        }
    }

    mutex.unlock();
}

void SettingsManager::SaveSettings()
{
    mutex.lock();

    if(backup_needed && filesystem::exists(settings_filename))
    {
        filesystem::path backup_name = settings_filename;
        backup_name.append(".bak");
        LOG_DEBUG("[SettingsManager] Config overwrite is requested, backing up the file as it was before launch as [%s]", backup_name.u8string().c_str());
        filesystem::rename(settings_filename, backup_name);
        backup_needed = false;
    }
    std::ofstream settings_file(settings_filename, std::ios::out | std::ios::binary);

    if(settings_file)
    {
        try
        {
            settings_file << std::setw(4) << settings_data;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR("[SettingsManager] Cannot write to file: %s", e.what());
        }

        settings_file.close();
    }
    mutex.unlock();
}

bool SettingsManager::IsFirstRun() const
{
    return firstrun;
}
