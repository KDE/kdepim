
include_directories(${AKONADI_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${CMAKE_SOURCE_DIR}/messageviewer
                    ${CMAKE_SOURCE_DIR}/pimcommon
                    ${CMAKE_SOURCE_DIR}/messagecomposer/
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/ 
)


set(pimsettingexporterconsole_SRCS
    importexportprogressindicatorconsole.cpp
    pimsettingexportconsole_debug.cpp
    pimsettingexporterconsole.cpp
    loginfo.cpp
    loginfile.cpp
    main.cpp)

add_executable(pimsettingexporterconsole ${pimsettingexporterconsole_SRCS})

target_link_libraries(pimsettingexporterconsole
  pimsettingexporterprivate
)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
install(TARGETS pimsettingexporterconsole ${INSTALL_TARGETS_DEFAULT_ARGS} )

