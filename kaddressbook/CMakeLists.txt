project(kaddressbook)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/xxport/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/
    )


add_definitions(-DTRANSLATION_DOMAIN=\"kaddressbook\")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(src)
add_subdirectory(kontactplugin)
