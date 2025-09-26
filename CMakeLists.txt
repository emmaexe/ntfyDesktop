cmake_minimum_required(VERSION 3.20)
project(
    ntfyDesktop
    VERSION 1.5.1
    DESCRIPTION "Receive ntfy notifications"
    HOMEPAGE_URL "https://github.com/emmaexe/ntfyDesktop/"
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Directory variables
set(APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/src")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/resources")
set(ICON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/icons")
set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
include_directories("${GEN_DIR}")

# Libraries
set(QT_DEFAULT_MAJOR_VERSION "6")
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs) # This is supposed to be KDEInstallDirs6 but KDEInstallDirs6 is broken atm
include(ECMInstallIcons)
include(ECMAddAppIcon)

if (ND_BUILD_TYPE STREQUAL "Flatpak")
    find_package(nlohmann_json 3.12.0 REQUIRED)
    find_package(emojicpp 3.0.0 REQUIRED)
else()
    # To update CPM.cmake, update the version number in the url and SHA hash as found in the most recent release asset.
    # https://github.com/cpm-cmake/CPM.cmake/wiki/Downloading-CPM.cmake-in-CMake
    file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
    )
    include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
    CPMUsePackageLock(package-lock.cmake)
    CPMAddPackage("gh:nlohmann/json@3.12.0")
    CPMAddPackage("gh:emmaexe/emojicpp@3.0.0")
endif()

find_package(CURL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
)
find_package(KF6CoreAddons)
find_package(KF6I18n)
find_package(KF6Notifications)
find_package(KF6XmlGui)

# Find all source files
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.hpp" "${SRC_DIR}/*.ui")

# Configure CPack
include(GNUInstallDirs)
set(CPACK_GENERATOR "RPM;DEB;TGZ")
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION "Subscribe to topics from any ntfy server and receive notifications natively on the desktop.")
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
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Emma <contact@emmaexe.moe>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_COMPONENTS_ALL ON)

# Configure files
configure_file("${SRC_DIR}/ntfyDesktop.hpp.in" "${GEN_DIR}/ntfyDesktop.hpp" @ONLY)
configure_file("${APP_DIR}/moe.emmaexe.ntfyDesktop.notifyrc.in" "${GEN_DIR}/moe.emmaexe.ntfyDesktop.notifyrc" @ONLY)
configure_file("${APP_DIR}/moe.emmaexe.ntfyDesktop.metainfo.xml.in" "${GEN_DIR}/moe.emmaexe.ntfyDesktop.metainfo.xml" @ONLY)
configure_file("${APP_DIR}/moe.emmaexe.ntfyDesktop.desktop.in" "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" @ONLY)

# Enable MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add resources
qt_add_resources(RES_FILES "${RES_DIR}/resources.qrc")

# Add executable
qt_add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${MOC_FILES} ${RES_FILES})

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json emojicpp CURL::libcurl Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql KF6::CoreAddons KF6::I18n KF6::Notifications KF6::XmlGui)

# Install files
file(GLOB ICONS_SRC "${ICON_DIR}/*.svg")
ecm_install_icons(ICONS ${ICONS_SRC} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES "${GEN_DIR}/moe.emmaexe.ntfyDesktop.notifyrc" DESTINATION "${KDE_INSTALL_DATAROOTDIR}/knotifications6")

install(FILES "${GEN_DIR}/moe.emmaexe.ntfyDesktop.metainfo.xml" DESTINATION "${KDE_INSTALL_DATAROOTDIR}/metainfo")

install(PROGRAMS "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" DESTINATION ${KDE_INSTALL_APPDIR})

install(TARGETS ${CMAKE_PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Package
if (NOT ND_BUILD_TYPE STREQUAL "Flatpak")
    include(CPack)
endif()