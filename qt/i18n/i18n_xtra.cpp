/*---------------------------------------------------------*\
| i18n_xtra.cpp                                             |
|                                                           |
|   List of extra translations that need to be used         |
|     outside QT objects                                    |
|                                                           |
|   Chris M (Dr_No)                             26 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "i18n_xtra.h"


const char* I2C_ERR_WIN =   QT_TRANSLATE_NOOP("i18n_xtra",
                                              "<h2>Some internal devices may not be detected:</h2>"
                                              "<p>One or more I2C or SMBus interfaces failed to initialize.</p>"
                                              "<p><b>RGB DRAM modules, some motherboards' onboard RGB lighting, and RGB Graphics Cards, will not be available in OpenRGB</b> without I2C or SMBus.</p>"
                                              "<h4>How to fix this:</h4>"
                                              "<p>On Windows, this is usually caused by a failure to load the WinRing0 driver.</p>"
                                              "<p>You must run OpenRGB as administrator at least once to allow WinRing0 to set up.</p>"
                                              "<p>See <a href='https://help.openrgb.org/'>help.openrgb.org</a> for additional troubleshooting steps if you keep seeing this message.<br></p>"
                                              "<h3>If you are not using internal RGB on a desktop this message is not important to you.</h3>");
const char* I2C_ERR_LINUX = QT_TRANSLATE_NOOP("i18n_xtra",
                                              "<h2>Some internal devices may not be detected:</h2>"
                                              "<p>One or more I2C or SMBus interfaces failed to initialize.</p>"
                                              "<p><b>RGB DRAM modules, some motherboards' onboard RGB lighting, and RGB Graphics Cards, will not be available in OpenRGB</b> without I2C or SMBus.</p>"
                                              "<h4>How to fix this:</h4>"
                                              "<p>On Linux, this is usually because the i2c-dev module is not loaded.</p>"
                                              "<p>You must load the i2c-dev module along with the correct i2c driver for your motherboard. "
                                              "This is usually i2c-piix4 for AMD systems and i2c-i801 for Intel systems.</p>"
                                              "<p>See <a href='https://help.openrgb.org/'>help.openrgb.org</a> for additional troubleshooting steps if you keep seeing this message.<br></p>"
                                              "<h3>If you are not using internal RGB on a desktop this message is not important to you.</h3>");

const char* UDEV_MISSING =  QT_TRANSLATE_NOOP("i18n_xtra",
                                              "<h2>WARNING:</h2>"
                                              "<p>The OpenRGB udev rules are not installed.</p>"
                                              "<p>Most devices will not be available unless running OpenRGB as root.</p>"
                                              "<p>If using AppImage, Flatpak, or self-compiled versions of OpenRGB you must install the udev rules manually</p>"
                                              "<p>See <a href='https://openrgb.org/udev'>https://openrgb.org/udev</a> to install the udev rules manually</p>");
const char* UDEV_MUTLI =    QT_TRANSLATE_NOOP("i18n_xtra",
                                              "<h2>WARNING:</h2>"
                                              "<p>Multiple OpenRGB udev rules are installed.</p>"
                                              "<p>The udev rules file 60-openrgb.rules is installed in both /etc/udev/rules.d and /usr/lib/udev/rules.d.</p>"
                                              "<p>Multiple udev rules files can conflict, it is recommended to remove one of them.</p>");

