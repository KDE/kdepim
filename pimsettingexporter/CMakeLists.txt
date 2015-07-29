project(pimsettingexporter)
add_definitions(-DTRANSLATION_DOMAIN=\"pimsettingexporter\")
include_directories( 
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/
                    ${CMAKE_BINARY_DIR}/pimsettingexporter/ 
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


install(FILES backup-structure.txt DESTINATION ${DATA_INSTALL_DIR}/pimsettingexporter)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(console)
