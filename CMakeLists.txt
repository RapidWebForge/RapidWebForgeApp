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
find_package(boost_algorithm REQUIRED CONFIG)
find_package(boost_uuid REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/rapidwebforge.cpp
        src/rapidwebforge.h
        src/rapidwebforge.ui
        # Components
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
        src/components/stepper-dashboard/stepperdashboard.h
        src/components/stepper-dashboard/stepperdashboard.cpp
        src/components/stepper-dashboard/stepperdashboard.ui
        src/components/backend-dashboard/backenddashboard.h
        src/components/backend-dashboard/backenddashboard.cpp
        src/components/backend-dashboard/backenddashboard.ui
        src/components/frontend-dashboard/frontenddashboard.h
        src/components/frontend-dashboard/frontenddashboard.cpp
        src/components/frontend-dashboard/frontenddashboard.ui
        src/components/create-table-dashboard/createtabledialog.h
        src/components/create-table-dashboard/createtabledialog.cpp
        src/components/create-table-dashboard/createtabledialog.ui
        src/components/addfield-dashboard/addfielddialog.h
        src/components/addfield-dashboard/addfielddialog.cpp
        src/components/addfield-dashboard/addfielddialog.ui
        src/components/projects-panel/projectspanel.h
        src/components/projects-panel/projectspanel.cpp
        src/components/projects-panel/projectspanel.ui
        src/components/project-preview/projectpreview.h
        src/components/project-preview/projectpreview.cpp
        src/components/project-preview/projectpreview.ui
        src/components/create-section/createsection.h
        src/components/create-section/createsection.cpp
        src/components/create-section/createsection.ui
        src/components/custom-tree-widget/customtreewidget.h
        src/components/custom-tree-widget/customtreewidget.cpp
        src/components/create-version/createversion.h
        src/components/create-version/createversion.cpp
        src/components/create-version/createversion.ui
        src/components/manage-version/manageversion.h
        src/components/manage-version/manageversion.cpp
        src/components/manage-version/manageversion.ui
        src/components/version-history/versionhistory.h
        src/components/version-history/versionhistory.cpp
        src/components/version-history/versionhistory.ui
        src/components/configuration-view/configurationview.h
        src/components/configuration-view/configurationview.cpp
        src/components/configuration-view/configurationview.ui
        src/components/editfield-dashboard/editfielddialog.h
        src/components/editfield-dashboard/editfielddialog.cpp
        src/components/editfield-dashboard/editfielddialog.ui
        src/components/customprogress-dialog/customprogressdialog.h
        src/components/customprogress-dialog/customprogressdialog.cpp
        src/components/delete-version/deleteversion.h
        src/components/delete-version/deleteversion.cpp
        src/components/delete-version/deleteversion.ui
        src/components/overview-panel/overviewpanel.h
        src/components/overview-panel/overviewpanel.cpp
        src/components/overview-panel/overviewpanel.ui
        src/components/tutorials-panel/tutorialspanel.h
        src/components/tutorials-panel/tutorialspanel.cpp
        src/components/tutorials-panel/tutorialspanel.ui
        src/components/pro-panel/propanel.h
        src/components/pro-panel/propanel.cpp
        src/components/pro-panel/propanel.ui
        src/components/overview-panel/overviewpanel.h
        src/components/overview-panel/overviewpanel.cpp
        src/components/overview-panel/overviewpanel.ui
        # Core
        src/core/project-manager/projectmanager.h
        src/core/project-manager/projectmanager.cpp
        src/core/code-generator/codegenerator.h
        src/core/code-generator/codegenerator.cpp
        src/core/backend-generator/backendgenerator.h
        src/core/backend-generator/backendgenerator.cpp
        src/core/frontend-generator/frontendgenerator.h
        src/core/frontend-generator/frontendgenerator.cpp
        src/core/version-manager/versionmanager.h
        src/core/version-manager/versionmanager.cpp
        src/core/configuration-manager/configurationmanager.h
        src/core/configuration-manager/configurationmanager.cpp
        src/core/deploy-manager/deploymanager.h
        src/core/deploy-manager/deploymanager.cpp
        src/core/project-worker/projectworker.h
        src/core/project-worker/projectworker.cpp
        src/core/logging/actionloggerjson.h
        src/core/logging/actionloggerjson.cpp
        src/core/logging/stepvalidator.h
        src/core/logging/stepvalidator.cpp
        # Models
        src/models/project/project.h
        src/models/project/project.cpp
        src/models/database-data/databasedata.h
        src/models/database-data/databasedata.cpp
        src/models/transaction/transaction.h
        src/models/transaction/transaction.cpp
        src/models/field/field.h
        src/models/field/field.cpp
        src/models/component/component.h
        src/models/component/component.cpp
        src/models/section/section.h
        src/models/section/section.cpp
        src/models/configuration/configuration.h
        src/models/configuration/configuration.cpp
        src/models/base-node/basenode.h
        src/models/base-node/basenode.cpp
        src/models/generic-node/genericnode.h
        src/models/generic-node/genericnode.cpp
        src/models/node-operation/nodeoperation.h
        src/models/transaction-operation/transactionoperation.h
        # Utils
        src/utils/database/database.h
        src/utils/database/database.cpp
        src/utils/ziphelper/ziphelper.h
        src/utils/ziphelper/ziphelper.cpp
        src/utils/file/fileutiils.h
        src/utils/file/fileutiils.cpp
        src/utils/render_callback/rendercallback.h
        src/utils/render_callback/rendercallback.cpp
        src/utils/file/filewatcher.h
        src/utils/file/filewatcher.cpp
        src/utils/vscode/fileopener.h
        src/utils/vscode/fileopener.cpp
        src/utils/ui/codeviewer.h
        src/utils/ui/codeviewer.cpp
        # Resources
        src/resources/resources.qrc
)


# Define the executable based on Qt version
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(RapidWebForgeApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(RapidWebForgeApp ${PROJECT_SOURCES}
    )
endif()

pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

target_link_libraries(RapidWebForgeApp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Boost::algorithm
    Boost::uuid
    OpenSSL::SSL
    fmt::fmt
    pantor::inja
    unofficial::mysql-connector-cpp::connector
    unofficial::sqlite3::sqlite3
    libzip::zip
    PkgConfig::LIBGIT2
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
