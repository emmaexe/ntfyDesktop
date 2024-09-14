cmake_minimum_required(VERSION 3.10)
project(
    ntfyDesktop
    VERSION 1.0.0
    DESCRIPTION "A desktop client for ntfy." 
    HOMEPAGE_URL "https://github.com/emmaexe/ntfyDesktop/"
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Directory variables
set(APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app}")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/src")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/resources")
set(ICON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/icons")
set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
include_directories("${GEN_DIR}")   

# Libraries
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs) # This is supposed to be KDEInstallDirs6 but KDEInstallDirs6 is broken atm
include(ECMInstallIcons)
include(ECMAddAppIcon)

# To update CPM.cmake, update the version number in the url and SHA hash as found in the most recent release asset.
# https://github.com/cpm-cmake/CPM.cmake/wiki/Downloading-CPM.cmake-in-CMake
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:emmaexe/emojicpp@2.0.0")
include_directories("${emojicpp_SOURCE_DIR}")
find_package(CURL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)
find_package(KF6CoreAddons)
find_package(KF6I18n)
find_package(KF6Notifications)

# Find all source files
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.hpp" "${SRC_DIR}/*.ui")

# Configure CPack
include(GNUInstallDirs)
set(CPACK_GENERATOR "RPM;DEB;TGZ")
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "${CMAKE_PROJECT_DESCRIPTION} Allows you to subscribe to topics from any ntfy server and recieve notifications natively on the desktop.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CMAKE_PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_STRIP_FILES TRUE)

set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_GROUP "System/Utilities")
set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
set(CPACK_RPM_PACKAGE_AUTOREQ TRUE)

set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}.deb")
# TODO: Depends on Qt6/KF6 and libcurl, needs setting up for debian
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "emmaexe <emma.git@emmaexe.moe>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_COMPONENTS_ALL ON)

# Configure files
configure_file("${SRC_DIR}/ntfyDesktop.hpp.in" "${GEN_DIR}/ntfyDesktop.hpp" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.notifyrc.in" "${GEN_DIR}/${CMAKE_PROJECT_NAME}.notifyrc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/moe.emmaexe.ntfyDesktop.desktop.in" "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" @ONLY)

# Enable MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add resources
qt_add_resources(RES_FILES "${RES_DIR}/resources.qrc")

# Add executable
qt_add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${MOC_FILES} ${RES_FILES})

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json emojicpp CURL::libcurl Qt6::Core Qt6::Gui Qt6::Widgets KF6::CoreAddons KF6::I18n KF6::Notifications)

# Install files
file(GLOB ICONS_SRC "${ICON_DIR}/*.png")
ecm_add_app_icon(${CMAKE_PROJECT_NAME} ICONS ${ICONS_SRC})
ecm_install_icons(ICONS ${ICONS_SRC} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES "${GEN_DIR}/${CMAKE_PROJECT_NAME}.notifyrc" DESTINATION "${KDE_INSTALL_DATAROOTDIR}/knotifications6")

install(PROGRAMS "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" DESTINATION ${KDE_INSTALL_APPDIR})

install(TARGETS ${CMAKE_PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Package
include(CPack)
