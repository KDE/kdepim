add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if(MESSAGEVIEWER_SUPPORTED)
  add_subdirectory(messageviewer)
endif()

if(KDEPIM_BUILD_DESKTOP)
  add_subdirectory(ktexteditor)
endif()

add_subdirectory(kaddressbook)
