project(blogilo)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if (QTWEBENGINE_SUPPORT_OPTION)
    add_subdirectory(composereditorwebengine)
endif()

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory(kconf_update)
