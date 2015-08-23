
include_directories(${AKONADI_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${CMAKE_SOURCE_DIR}/messageviewer
                    ${CMAKE_SOURCE_DIR}/pimcommon
                    ${CMAKE_SOURCE_DIR}/messagecomposer/
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/core/ 
)


set(pimsettingexporterconsole_SRCS
    importexportprogressindicatorconsole.cpp
    pimsettingexportconsole_debug.cpp
    pimsettingexporterconsole.cpp
    loginfo.cpp
    loginfile.cpp
    main.cpp)

add_executable(pimsettingexporterconsole ${pimsettingexporterconsole_SRCS})

target_link_libraries(pimsettingexporterconsole KF5::ConfigCore Qt5::Xml KF5::CoreAddons KF5::I18n KF5::Archive
  pimsettingexporterprivate
)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
install(TARGETS pimsettingexporterconsole ${INSTALL_TARGETS_DEFAULT_ARGS} )

