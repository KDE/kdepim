include_directories( ${CMAKE_SOURCE_DIR}/kleopatra )

add_definitions( -DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII )

kde4_add_library( kleopatraclientgui SHARED
  certificaterequester.cpp
  )

target_link_libraries( kleopatraclientgui ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} kleopatraclientcore )

set_target_properties( kleopatraclientgui
  PROPERTIES
    VERSION   ${libkleopatraclient_version}
    SOVERSION ${libkleopatraclient_soversion}
  )


install(
  TARGETS kleopatraclientgui
  DESTINATION ${LIB_INSTALL_DIR}
  )
  

install(
  FILES
    certificaterequester.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient/gui
  )

