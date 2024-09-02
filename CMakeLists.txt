cmake_minimum_required(VERSION 3.10)
project(ntfyDesktop VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Directory variables
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/src")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app/resources")
set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
include_directories("${GEN_DIR}")

# Application variables
set(ND_DESCRIPTION "A desktop client for ntfy.")

# Setup libraries

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
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

# Find all source and header files in src directory using file globbing
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.hpp" "${SRC_DIR}/*.ui")

# Configure files
configure_file("${SRC_DIR}/ntfyDesktop.hpp.in" "${GEN_DIR}/ntfyDesktop.hpp" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.notifyrc.in" "${GEN_DIR}/${CMAKE_PROJECT_NAME}.notifyrc" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/moe.emmaexe.ntfyDesktop.desktop.in" "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" @ONLY)

# Setup Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Handle resources
qt_add_resources(RES_FILES "${RES_DIR}/resources.qrc")

# Add executable
qt_add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${MOC_FILES} ${RES_FILES})

# Link found libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json emojicpp CURL::libcurl Qt6::Core Qt6::Gui Qt6::Widgets KF6::CoreAddons KF6::I18n KF6::Notifications)

# Install
file(GLOB ICONS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/app/icons/*.png")
ecm_add_app_icon(${CMAKE_PROJECT_NAME} ICONS ${ICONS_SRC})
ecm_install_icons(ICONS ${ICONS_SRC} DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES "${GEN_DIR}/${CMAKE_PROJECT_NAME}.notifyrc" DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

install(PROGRAMS "${GEN_DIR}/moe.emmaexe.ntfyDesktop.desktop" DESTINATION ${KDE_INSTALL_APPDIR})

install(TARGETS ${CMAKE_PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
