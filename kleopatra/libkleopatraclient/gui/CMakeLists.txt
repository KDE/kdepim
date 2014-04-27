include_directories(${CMAKE_SOURCE_DIR}/kleopatra)

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII)

add_library(kleopatraclientgui ${LIBRARY_TYPE} certificaterequester.cpp)

target_link_libraries(kleopatraclientgui
  
  
  kleopatraclientcore
)

set_target_properties(kleopatraclientgui PROPERTIES
  VERSION ${libkleopatraclient_version}
  SOVERSION ${libkleopatraclient_soversion}
)

install(TARGETS kleopatraclientgui ${INSTALL_TARGETS_DEFAULT_ARGS})
