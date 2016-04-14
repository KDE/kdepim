project(ktnef)

if(${CMAKE_SOURCE_DIR} STREQUAL ${ktnef_SOURCE_DIR})
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
    find_package(KF5Tnef ${KTNEF_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
    include_directories(${ktnef_SOURCE_DIR} ${ktnef_BINARY_DIR})
endif()

include(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)
configure_file(config-ktnef.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef.h)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(pics)

set(ktnef_SRCS
    attachpropertydialog.cpp
    ktnefmain.cpp
    ktnefview.cpp
    main.cpp
    messagepropertydialog.cpp
    qwmf.cpp
    )
qt5_add_resources(ktnef_SRCS ktnef.qrc)
ecm_qt_declare_logging_category(ktnef_SRCS HEADER ktnef_debug.h IDENTIFIER KTNEFAPPS_LOG CATEGORY_NAME log_ktnefapps)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hicolor/*-apps-ktnef.png")
ecm_add_app_icon(ktnef_SRCS ICONS ${ICONS_SRCS})

ki18n_wrap_ui(ktnef_SRCS ui/attachpropertywidgetbase.ui)

add_executable(ktnef ${ktnef_SRCS})
target_link_libraries(ktnef KF5::Tnef KF5::DBusAddons)


install(TARGETS ktnef ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.ktnef.desktop DESTINATION ${KDE_INSTALL_APPDIR})


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
