project(kaddressbook)


add_definitions(-DTRANSLATION_DOMAIN=\"kaddressbook\")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(src)
add_subdirectory(kontactplugin)
