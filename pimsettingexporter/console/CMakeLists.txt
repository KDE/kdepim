
include_directories(${AKONADI_INCLUDE_DIR} 
    ${CMAKE_SOURCE_DIR}/pimsettingexporter/core/
    )


set(pimsettingexporterconsole_SRCS
    importexportprogressindicatorconsole.cpp
    pimsettingexportconsole_debug.cpp
    pimsettingexporterconsole.cpp
    loginfo.cpp
    loginfile.cpp
    main.cpp)

ecm_qt_declare_logging_category(pimsettingexporterconsole_SRCS HEADER pimsettingexportconsole_debug.h IDENTIFIER PIMSETTINGEXPORTERCONSOLE_LOG CATEGORY_NAME log_pimsettingexporterconsole)

add_executable(pimsettingexporterconsole ${pimsettingexporterconsole_SRCS})

target_link_libraries(pimsettingexporterconsole KF5::ConfigCore Qt5::Xml KF5::CoreAddons KF5::I18n KF5::Archive KF5::AkonadiCore   KF5::MailCommon
    pimsettingexporterprivate
    )
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
install(TARGETS pimsettingexporterconsole ${INSTALL_TARGETS_DEFAULT_ARGS} )

