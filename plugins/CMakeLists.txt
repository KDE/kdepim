if(MESSAGEVIEWER_SUPPORTED)
  add_subdirectory( messageviewer )
endif(MESSAGEVIEWER_SUPPORTED)

if (KDEPIM_BUILD_DESKTOP)
  add_subdirectory( ktexteditor )
endif (KDEPIM_BUILD_DESKTOP)

add_subdirectory( kaddressbook )
