add_subdirectory( agenda )

if (NOT WINCE)
add_subdirectory( viewerapp )
endif (NOT WINCE)

if ( KDEPIM_MOBILE_UI AND QT_QTDECLARATIVE_FOUND )
  add_subdirectory(qml)
endif ()