cmake_minimum_required(VERSION 3.16)

project(RapidWebForgeApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_MANIFEST_MODE ON)

if(APPLE)
    message(STATUS "Configuring for macOS")
    set(MACOS_SPECIFIC_LIBS "-framework CoreFoundation" resolv)
elseif(WIN32)
    message(STATUS "Configuring for Windows")
    set(WIN32_EXECUTABLE TRUE)
endif()

# Packages
# QtCreator
find_package(Qt6 COMPONENTS Widgets CONFIG QUIET)
if(NOT TARGET Qt6::Widgets)
    find_package(Qt5 REQUIRED COMPONENTS Widgets CONFIG)
endif()

# Vcpkg
find_package(boost_algorithm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/rapidwebforge.cpp
        src/rapidwebforge.h
        src/rapidwebforge.ui
        src/components/creation-assistant/creationassistant.h
        src/components/creation-assistant/creationassistant.cpp
        src/components/creation-assistant/creationassistant.ui
        src/components/database-assistant/databaseassistant.h
        src/components/database-assistant/databaseassistant.cpp
        src/components/database-assistant/databaseassistant.ui
        src/components/frontend-assistant/frontendassistant.h
        src/components/frontend-assistant/frontendassistant.cpp
        src/components/frontend-assistant/frontendassistant.ui
        src/components/backend-assistant/backendassistant.h
        src/components/backend-assistant/backendassistant.cpp
        src/components/backend-assistant/backendassistant.ui
        src/components/summary-assistant/summaryassistant.h
        src/components/summary-assistant/summaryassistant.cpp
        src/components/summary-assistant/summaryassistant.ui
        src/components/stepper/stepper.h
        src/components/stepper/stepper.cpp
        src/components/stepper/stepper.ui
        src/core/project-manager/projectmanager.h
        src/core/project-manager/projectmanager.cpp
)

# Define the executable based on Qt version
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(RapidWebForgeApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(RapidWebForgeApp ${PROJECT_SOURCES})
endif()

target_link_libraries(RapidWebForgeApp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Boost::algorithm
    fmt::fmt
    pantor::inja
    unofficial::mysql-connector-cpp::connector
    ${MACOS_SPECIFIC_LIBS}
)

set_target_properties(RapidWebForgeApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS RapidWebForgeApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RapidWebForgeApp)
endif()
