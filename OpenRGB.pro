#-----------------------------------------------------------------------------------------------#
# OpenRGB 0.x QMake Project (Linux-only version)                                                #
#-----------------------------------------------------------------------------------------------#

QT += core gui

CONFIG += c++17 lrelease embed_translations silent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
MAJOR       = 0
MINOR       = 9
SUFFIX      = git

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_DEB = $$VERSION_NUM
VERSION_WIX = $$VERSION_NUM
VERSION_AUR = $$VERSION_NUM
VERSION_RPM = $$VERSION_NUM

equals(SUFFIX, "git") {
    VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX$$COMMITS")"
    VERSION_DEB = $$VERSION_DEB"~git"$$SHORTHASH
    VERSION_AUR = $$VERSION_AUR".g"$$SHORTHASH
    VERSION_RPM = $$VERSION_RPM"^git"$$SHORTHASH
} else {
    !isEmpty(SUFFIX) {
        VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX")"
        VERSION_DEB = $$VERSION_DEB"~"$$SUFFIX
        VERSION_AUR = $$VERSION_AUR"."$$SUFFIX
        VERSION_RPM = $$VERSION_RPM"^"$$SUFFIX
    }
}

TARGET      = OpenRGB
TEMPLATE    = app

message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_SFX: "$$SUFFIX)
message("VERSION_DEB: "$$VERSION_DEB)
message("VERSION_WIX: "$$VERSION_WIX)
message("VERSION_AUR: "$$VERSION_AUR)
message("VERSION_RPM: "$$VERSION_RPM)
message("QT_VERSION:  "$$QT_VERSION)

#-----------------------------------------------------------------------------------------------#
# Build info                                                                                    #
#-----------------------------------------------------------------------------------------------#
linux:BUILDDATE = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID   = $$system(git log -n 1 --pretty=format:"%H")
GIT_COMMIT_DATE = $$system(git log -n 1 --pretty=format:"%ci")
GIT_BRANCH      = $$system(sh scripts/git-get-branch.sh)

message("GIT_BRANCH: "$$GIT_BRANCH)

DEFINES += \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\" \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\" \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\" \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\" \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"

#-----------------------------------------------------------------------------------------------#
# Sources and Includes                                                                          #
#-----------------------------------------------------------------------------------------------#
FORMS += $$files("qt/*.ui", true)
for(iter, FORMS) {
    GUI_INCLUDES += $$dirname(iter)
}
GUI_INCLUDES = $$unique(GUI_INCLUDES)

GUI_H   = $$files("qt/*.h", true)
GUI_CPP = $$files("qt/*.cpp", true)
CONTROLLER_H   = $$files("Controllers/*.h", true)
CONTROLLER_CPP = $$files("Controllers/*.cpp", true)

for(iter, $$list($$CONTROLLER_H)) {
    CONTROLLER_INCLUDES += $$dirname(iter)
}
CONTROLLER_INCLUDES = $$unique(CONTROLLER_INCLUDES)

#-----------------------------------------------------------------------------------------------#
# Linux-only filtering                                                                          #
#-----------------------------------------------------------------------------------------------#
CONTROLLER_H_LINUX      = $$files("Controllers/*_Linux*.h",     true)
CONTROLLER_CPP_LINUX    = $$files("Controllers/*_Linux*.cpp",   true)
CONTROLLER_H           -= $$files("Controllers/*_Windows*.h", true)
CONTROLLER_CPP         -= $$files("Controllers/*_Windows*.cpp", true)
CONTROLLER_H           -= $$files("Controllers/*_FreeBSD*.h", true)
CONTROLLER_CPP         -= $$files("Controllers/*_FreeBSD*.cpp", true)
CONTROLLER_H           -= $$files("Controllers/*_MacOS*.h", true)
CONTROLLER_CPP         -= $$files("Controllers/*_MacOS*.cpp", true)

#-----------------------------------------------------------------------------------------------#
# Common Include Paths                                                                          #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH += \
    $$CONTROLLER_INCLUDES \
    $$GUI_INCLUDES \
    dependencies/ColorWheel \
    dependencies/CRCpp/ \
    dependencies/hueplusplus-1.2.0/include \
    dependencies/hueplusplus-1.2.0/include/hueplusplus \
    dependencies/httplib \
    dependencies/json/ \
    dependencies/mdns \
    dmiinfo/ \
    hidapi_wrapper/ \
    i2c_smbus/ \
    i2c_tools/ \
    interop/ \
    net_port/ \
    pci_ids/ \
    scsiapi/ \
    serial_port/ \
    super_io/ \
    AutoStart/ \
    KeyboardLayoutManager/ \
    RGBController/ \
    qt/ \
    SPDAccessor/ \
    SuspendResume/ \
    dependencies/stb/

#-----------------------------------------------------------------------------------------------#
# Headers and Sources                                                                           #
#-----------------------------------------------------------------------------------------------#
HEADERS += \
    $$GUI_H \
    $$CONTROLLER_H \
    Colors.h \
    dependencies/ColorWheel/ColorWheel.h \
    dependencies/json/nlohmann/json.hpp \
    LogManager.h \
    NetworkClient.h \
    NetworkProtocol.h \
    NetworkServer.h \
    OpenRGBPluginInterface.h \
    PluginManager.h \
    ProfileManager.h \
    ResourceManager.h \
    ResourceManagerInterface.h \
    SettingsManager.h \
    Detector.h \
    DeviceDetector.h \
    dmiinfo/dmiinfo.h \
    filesystem.h \
    hidapi_wrapper/hidapi_wrapper.h \
    i2c_smbus/i2c_smbus.h \
    i2c_tools/i2c_tools.h \
    interop/DeviceGuard.h \
    interop/DeviceGuardLock.h \
    interop/DeviceGuardManager.h \
    net_port/net_port.h \
    pci_ids/pci_ids.h \
    scsiapi/scsiapi.h \
    serial_port/find_usb_serial_port.h \
    serial_port/serial_port.h \
    super_io/super_io.h \
    StringUtils.h \
    SuspendResume/SuspendResume.h \
    AutoStart/AutoStart.h \
    KeyboardLayoutManager/KeyboardLayoutManager.h \
    RGBController/RGBController.h \
    RGBController/RGBController_Dummy.h \
    RGBController/RGBControllerKeyNames.h \
    RGBController/RGBController_Network.h \
    startup/startup.h

SOURCES += \
    $$GUI_CPP \
    $$CONTROLLER_CPP \
    dependencies/ColorWheel/ColorWheel.cpp \
    dependencies/hueplusplus-1.2.0/src/*.cpp \
    startup/startup.cpp \
    cli.cpp \
    dmiinfo/dmiinfo.cpp \
    LogManager.cpp \
    NetworkClient.cpp \
    NetworkProtocol.cpp \
    NetworkServer.cpp \
    PluginManager.cpp \
    ProfileManager.cpp \
    ResourceManager.cpp \
    SPDAccessor/*.cpp \
    SettingsManager.cpp \
    i2c_smbus/i2c_smbus.cpp \
    i2c_tools/i2c_tools.cpp \
    interop/DeviceGuard.cpp \
    interop/DeviceGuardLock.cpp \
    interop/DeviceGuardManager.cpp \
    net_port/net_port.cpp \
    serial_port/serial_port.cpp \
    StringUtils.cpp \
    AutoStart/AutoStart.cpp \
    KeyboardLayoutManager/KeyboardLayoutManager.cpp \
    RGBController/*.cpp

RESOURCES += qt/resources.qrc

#-----------------------------------------------------------------------------------------------#
# Translations                                                                                  #
#-----------------------------------------------------------------------------------------------#
TRANSLATIONS += qt/i18n/*.ts

#-----------------------------------------------------------------------------------------------#
# Linux-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, linux) {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0

    TARGET = $$lower($$TARGET)

    HEADERS += $$CONTROLLER_H_LINUX \
        dependencies/NVFC/nvapi.h \
        i2c_smbus/Linux/i2c_smbus_linux.h \
        AutoStart/AutoStart-Linux.h \
        SPDAccessor/EE1004Accessor_Linux.h \
        SPDAccessor/SPD5118Accessor_Linux.h \
        SuspendResume/SuspendResume_Linux_FreeBSD.h \
        super_io/super_io.h

    INCLUDEPATH += dependencies/NVFC i2c_smbus/Linux /usr/include/mbedtls/

    LIBS += -L/usr/lib/mbedtls/ -lmbedx509 -lmbedtls -lmbedcrypto -ldl

    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    if (!versionAtLeast(COMPILER_VERSION, "9")) {
        LIBS += -lstdc++fs
    }

    QT += dbus
    QMAKE_CXXFLAGS += -Wno-implicit-fallthrough -Wno-psabi

    packagesExist(hidapi-hidraw) {
        PKGCONFIG += hidapi-hidraw
        HIDAPI_HIDRAW_VERSION = $$system($$PKG_CONFIG --modversion hidapi-hidraw)
        if(versionAtLeast(HIDAPI_HIDRAW_VERSION, "0.10.1")) {
            DEFINES += USE_HID_USAGE
        }
    } else {
        packagesExist(hidapi-libusb) {
            PKGCONFIG += hidapi-libusb
        } else {
            PKGCONFIG += hidapi
        }
    }

    SOURCES += $$CONTROLLER_CPP_LINUX \
        dependencies/hueplusplus-1.2.0/src/LinHttpHandler.cpp \
        dependencies/NVFC/nvapi.cpp \
        i2c_smbus/Linux/i2c_smbus_linux.cpp \
        scsiapi/scsiapi_linux.c \
        serial_port/find_usb_serial_port_linux.cpp \
        AutoStart/AutoStart-Linux.cpp \
        SPDAccessor/EE1004Accessor_Linux.cpp \
        SPDAccessor/SPD5118Accessor_Linux.cpp \
        SuspendResume/SuspendResume_Linux_FreeBSD.cpp \
        startup/main_FreeBSD_Linux_MacOS.cpp \
        super_io/super_io.cpp

    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    !defined(OPENRGB_SYSTEM_PLUGIN_DIRECTORY, var):OPENRGB_SYSTEM_PLUGIN_DIRECTORY = "$$PREFIX/lib/openrgb/plugins"
    DEFINES += OPENRGB_SYSTEM_PLUGIN_DIRECTORY=\\"\"\"$$OPENRGB_SYSTEM_PLUGIN_DIRECTORY\\"\"\" 

    CONFIG(release, debug|release) {
        udev_rules.CONFIG = no_check_exist
        udev_rules.target = 60-openrgb.rules
        udev_rules.path   = $$PREFIX/lib/udev/rules.d/

        exists($$udev_rules.target) {
            message($$udev_rules.target " - UDEV rules file exists. Removing from build")
            udev_rules.files = $$udev_rules.target
        } else {
            message($$udev_rules.target " - UDEV rules file missing. Adding script to build")
            QMAKE_CXXFLAGS+=-save-temps
            QMAKE_CXXFLAGS-=-pipe
            udev_rules.extra = $$PWD/scripts/build-udev-rules.sh $$PWD $$GIT_COMMIT_ID
            udev_rules.files = $$OUT_PWD/60-openrgb.rules
        }
    }

    target.path=$$PREFIX/bin/
    desktop.path=$$PREFIX/share/applications/
    desktop.files+=qt/org.openrgb.OpenRGB.desktop
    icon.path=$$PREFIX/share/icons/hicolor/128x128/apps/
    icon.files+=qt/org.openrgb.OpenRGB.png
    metainfo.path=$$PREFIX/share/metainfo/
    metainfo.files+=qt/org.openrgb.OpenRGB.metainfo.xml
    systemd_service.path=$$PREFIX/lib/systemd/system/
    systemd_service.files+=qt/openrgb.service
    INSTALLS += target desktop icon metainfo udev_rules systemd_service
}

DISTFILES += \
    debian/openrgb-udev.postinst \
    debian/openrgb.postinst
