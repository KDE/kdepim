include_directories( ${ASSUAN_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra )

add_definitions( -D_ASSUAN_ONLY_GPG_ERRORS -DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII )

kde4_add_library( kleopatraclientcore SHARED
  initialization.cpp
  command.cpp
  #connection.cpp
  )

if ( WIN32 )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_VANILLA_LIBRARIES} )
else ( WIN32 )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_PTHREAD_LIBRARIES} )
endif ( WIN32 )

install(
  TARGETS kleopatraclientcore
  DESTINATION ${LIB_INSTALL_DIR}
  )
  

install(
  FILES
    initialization.h
    command.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient/core
  )

