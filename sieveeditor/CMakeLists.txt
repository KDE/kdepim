project( sieveeditor )
if(${CMAKE_SOURCE_DIR} STREQUAL ${sieveeditor_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)
    find_package(ECM 5.20.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ktnef_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
    include(ECMInstallIcons)
    include(KDEInstallDirs)
    include(ECMAddAppIcon)
    include(ECMQtDeclareLoggingCategory)
    include(GenerateExportHeader)
    include(ECMGenerateHeaders)
    include(CMakePackageConfigHelpers)
    include(FeatureSummary)

    set(KTNEF_LIB_VERSION "5.2.40")
    set(KF5_VERSION "5.20.0")
    find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
    include_directories(${sieveeditor_SOURCE_DIR} ${sieveeditor_BINARY_DIR})


endif()

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set( sieveeditor_kcfg_SRCS settings/sieveeditorglobalconfig.kcfgc )

kconfig_add_kcfg_files(sieveeditor_kcfg_SRCS ${sieveeditor_kcfg_SRCS})


set(sieveeditor_SRCS
    ${sieveeditor_kcfg_SRCS}
    main.cpp
    sieveeditormainwindow.cpp
    serversievesettingsdialog.cpp
    serversievesettings.cpp
    sieveeditormainwidget.cpp
    sieveeditorscriptwidget.cpp
    sieveeditorconfiguredialog.cpp
    sieveeditorscriptmanagerwidget.cpp
    sieveeditorutil.cpp
    serversievelistwidget.cpp
    sieveeditorconfigureserverwidget.cpp
    sieveeditormanagesievewidget.cpp
    sieveeditorpagewidget.cpp
    sieveserversettings.cpp
    sieveeditortabwidget.cpp
    sieveeditorcentralwidget.cpp
    sieveeditorconfigureserverpage.cpp
    sieveeditorbookmarks.cpp
    sieveeditoremptytabwidgetlabel.cpp
    )

ki18n_wrap_ui(sieveeditor_SRCS
    ui/serversievesettings.ui
    ui/sieveeditorconfigureserverwidget.ui
    )

qt5_add_resources(sieveeditor_SRCS sieveeditor.qrc)

ecm_qt_declare_logging_category(sieveeditor_SRCS HEADER sieveeditor_debug.h IDENTIFIER SIEVEEDITOR_LOG CATEGORY_NAME log_sieveeditor)

add_executable(sieveeditor ${sieveeditor_SRCS} )
target_link_libraries(sieveeditor
    KF5::MailTransport
    KF5::Bookmarks
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::XmlGui
    KF5::PimCommon
    KF5::KSieveUi
    KF5::KManageSieve
    )

install(TARGETS sieveeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.sieveeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})

add_subdirectory(kconf_update)

install(FILES settings/sieveeditorglobalconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
