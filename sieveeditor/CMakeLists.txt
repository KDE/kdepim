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


add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

