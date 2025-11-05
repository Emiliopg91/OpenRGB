cmake_minimum_required(VERSION 3.16)
project(OpenRGB LANGUAGES CXX)

#-----------------------------------------------------------------------------------------------#
# Compiler setup                                                                                #
#-----------------------------------------------------------------------------------------------#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#-----------------------------------------------------------------------------------------------#
# Qt detection                                                                                  #
#-----------------------------------------------------------------------------------------------#
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets DBus)

find_package(httplib CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NLOHMANN_JSON REQUIRED nlohmann_json)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

#-----------------------------------------------------------------------------------------------#
# Git information                                                                               #
#-----------------------------------------------------------------------------------------------#
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD OUTPUT_VARIABLE GIT_SHORTHASH OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count release_0.9..HEAD OUTPUT_VARIABLE GIT_COMMITS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:%H OUTPUT_VARIABLE GIT_COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:%ci OUTPUT_VARIABLE GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_SHORTHASH unknown)
    set(GIT_COMMITS 0)
    set(GIT_COMMIT_ID unknown)
    set(GIT_COMMIT_DATE unknown)
    set(GIT_BRANCH unknown)
endif()

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_NUM "${VERSION_MAJOR}.${VERSION_MINOR}.${GIT_COMMITS}")
set(VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}+ (git${GIT_COMMITS})")

message(STATUS "VERSION_NUM: ${VERSION_NUM}")
message(STATUS "VERSION_STR: ${VERSION_STR}")
message(STATUS "GIT_COMMIT_ID: ${GIT_COMMIT_ID}")
message(STATUS "GIT_BRANCH: ${GIT_BRANCH}")

#-----------------------------------------------------------------------------------------------#
# Dependencies                                                                                  #
#-----------------------------------------------------------------------------------------------#
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(HIDAPI QUIET hidapi-hidraw)

if(HIDAPI_FOUND)
    execute_process(COMMAND pkg-config --modversion hidapi-hidraw OUTPUT_VARIABLE HIDAPI_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(HIDAPI_VERSION VERSION_GREATER_EQUAL "0.10.1")
        add_compile_definitions(USE_HID_USAGE)
    endif()
else()
    pkg_check_modules(HIDAPI REQUIRED hidapi)
endif()

include_directories(
    ${LIBUSB_INCLUDE_DIRS}
    ${HIDAPI_INCLUDE_DIRS}

    # Dependencias externas
    dependencies/CRCpp
    dependencies/mdns
    dependencies/stb

    # Código propio
    dmiinfo
    hidapi_wrapper
    i2c_smbus
    i2c_tools
    interop
    net_port
    pci_ids
    scsiapi
    serial_port
    AutoStart
    KeyboardLayoutManager
    RGBController
    qt
    SPDAccessor
    SuspendResume
)

file(GLOB_RECURSE CONTROLLER_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/Controllers/*")
foreach(dir ${CONTROLLER_DIRS})
    if(IS_DIRECTORY ${dir})
        include_directories(${dir})
    endif()
endforeach()

link_directories(
    ${LIBUSB_LIBRARY_DIRS}
    ${HIDAPI_LIBRARY_DIRS}
)

#-----------------------------------------------------------------------------------------------#
# Source files (Linux only + filtra controladores problemáticos)                                #
#-----------------------------------------------------------------------------------------------#

file(GLOB_RECURSE OPENRGB_COMMON_CPP
    ${CMAKE_SOURCE_DIR}/qt/*.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/*.cpp
    ${CMAKE_SOURCE_DIR}/startup/*.cpp
    ${CMAKE_SOURCE_DIR}/dmiinfo/*.cpp
    ${CMAKE_SOURCE_DIR}/interop/*.cpp
    ${CMAKE_SOURCE_DIR}/i2c_smbus/*.cpp
    ${CMAKE_SOURCE_DIR}/i2c_tools/*.cpp
    ${CMAKE_SOURCE_DIR}/net_port/*.cpp
    ${CMAKE_SOURCE_DIR}/serial_port/*.cpp
    ${CMAKE_SOURCE_DIR}/super_io/*.cpp
    ${CMAKE_SOURCE_DIR}/AutoStart/*.cpp
    ${CMAKE_SOURCE_DIR}/KeyboardLayoutManager/*.cpp
    ${CMAKE_SOURCE_DIR}/SPDAccessor/*.cpp
    ${CMAKE_SOURCE_DIR}/RGBController/*.cpp
    ${CMAKE_SOURCE_DIR}/LogManager.cpp
    ${CMAKE_SOURCE_DIR}/NetworkClient.cpp
    ${CMAKE_SOURCE_DIR}/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/NetworkServer.cpp
    ${CMAKE_SOURCE_DIR}/PluginManager.cpp
    ${CMAKE_SOURCE_DIR}/ProfileManager.cpp
    ${CMAKE_SOURCE_DIR}/ResourceManager.cpp
    ${CMAKE_SOURCE_DIR}/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/StringUtils.cpp
    ${CMAKE_SOURCE_DIR}/cli.cpp
)

# Quita los ficheros de otras plataformas
list(FILTER OPENRGB_COMMON_CPP EXCLUDE REGEX "_Windows")
list(FILTER OPENRGB_COMMON_CPP EXCLUDE REGEX "_MacOS")
list(FILTER OPENRGB_COMMON_CPP EXCLUDE REGEX "_FreeBSD")

# Añade las fuentes específicas de Linux
set(OPENRGB_LINUX_CPP
    ${CMAKE_SOURCE_DIR}/dependencies/NVFC/nvapi.cpp
    ${CMAKE_SOURCE_DIR}/i2c_smbus/Linux/i2c_smbus_linux.cpp
    ${CMAKE_SOURCE_DIR}/scsiapi/scsiapi_linux.c
    ${CMAKE_SOURCE_DIR}/serial_port/find_usb_serial_port_linux.cpp
    ${CMAKE_SOURCE_DIR}/AutoStart/AutoStart-Linux.cpp
    ${CMAKE_SOURCE_DIR}/SPDAccessor/EE1004Accessor_Linux.cpp
    ${CMAKE_SOURCE_DIR}/SPDAccessor/SPD5118Accessor_Linux.cpp
    ${CMAKE_SOURCE_DIR}/SuspendResume/SuspendResume_Linux_FreeBSD.cpp
    ${CMAKE_SOURCE_DIR}/startup/main_FreeBSD_Linux_MacOS.cpp
)

# Combina ambas listas
set(OPENRGB_SOURCES ${OPENRGB_COMMON_CPP} ${OPENRGB_LINUX_CPP})

#-----------------------------------------------------------------------------------------------#
# Headers (sin los de Windows, macOS o FreeBSD)                                                 #
#-----------------------------------------------------------------------------------------------#
file(GLOB_RECURSE OPENRGB_HEADERS
    ${CMAKE_SOURCE_DIR}/qt/*.h
    ${CMAKE_SOURCE_DIR}/Controllers/**/*.h
    ${CMAKE_SOURCE_DIR}/dependencies/**/*.h
    ${CMAKE_SOURCE_DIR}/*.h
)
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "_Windows")
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "_MacOS")
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "_FreeBSD")
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "Controllers/SeagateController")
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "Controllers/ENESMBusController/ROGArion")
list(FILTER OPENRGB_HEADERS EXCLUDE REGEX "Controllers/ENESMBusController/ENESMBusInterface/ENESMBusInterface_ROGArion")

# Recursos y traducciones
file(GLOB OPENRGB_TRANSLATIONS "${CMAKE_SOURCE_DIR}/qt/i18n/*.ts")



#-----------------------------------------------------------------------------------------------#
# Executable                                                                                   #
#-----------------------------------------------------------------------------------------------#
qt_add_executable(OpenRGB
    ${OPENRGB_SOURCES}
    ${OPENRGB_HEADERS}
    ${OPENRGB_RESOURCES}
    ${OPENRGB_TRANSLATIONS}
)

target_include_directories(OpenRGB PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OpenRGB PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${LIBUSB_LIBRARIES}
    ${HIDAPI_LIBRARIES}
    dl
)

#-----------------------------------------------------------------------------------------------#
# Definitions                                                                                  #
#-----------------------------------------------------------------------------------------------#
set(OPENRGB_SYSTEM_PLUGIN_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib/openrgb/plugins")
add_compile_definitions(
    VERSION_STRING="${VERSION_STR}"
    BUILDDATE_STRING="$ENV{SOURCE_DATE_EPOCH}"
    GIT_COMMIT_ID="${GIT_COMMIT_ID}"
    GIT_COMMIT_DATE="${GIT_COMMIT_DATE}"
    GIT_BRANCH="${GIT_BRANCH}"
    OPENRGB_SYSTEM_PLUGIN_DIRECTORY="${OPENRGB_SYSTEM_PLUGIN_DIRECTORY}"
)

#-----------------------------------------------------------------------------------------------#
# Install rules                                                                                 #
#-----------------------------------------------------------------------------------------------#
install(TARGETS OpenRGB DESTINATION bin)

# ---------------------------------------------------------------------------------------
# Remove -pipe from global flags (incompatible with -save-temps)
# ---------------------------------------------------------------------------------------
foreach(var CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
  if(${var} MATCHES "-pipe")
    string(REPLACE "-pipe" "" ${var} "${${var}}")
    message(STATUS "Removed -pipe from ${var}: ${${var}}")
  endif()
endforeach()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(OpenRGB PRIVATE
    -Wno-implicit-fallthrough -Wno-psabi -Wno-unused-parameter
    -Wno-sign-compare -Wno-unused-variable -Wno-unused-result -Wno-deprecated-declarations
    -O3 -funroll-loops -fstrict-aliasing -ftree-vectorize -fprefetch-loop-arrays
    -fomit-frame-pointer -fno-math-errno -save-temps
  )
  include(CheckIPOSupported)
  check_ipo_supported(RESULT lto_ok OUTPUT lto_msg)
  if(lto_ok)
    set_property(TARGET OpenRGB PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

#-----------------------------------------------------------------------------------------------#
# Summary                                                                                       #
#-----------------------------------------------------------------------------------------------#
message(STATUS "")
message(STATUS "OpenRGB will be installed to: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Plugin directory: ${OPENRGB_SYSTEM_PLUGIN_DIRECTORY}")
message(STATUS "Qt version: ${QT_VERSION_MAJOR}")
message(STATUS "")
