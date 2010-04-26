add_subdirectory( agenda )

add_subdirectory( viewerapp )

if ( KDEPIM_MOBILE_UI AND QT_QTDECLARATIVE_FOUND )
  add_subdirectory(qml)
endif ()