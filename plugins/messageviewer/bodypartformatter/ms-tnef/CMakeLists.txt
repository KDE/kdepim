
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)



add_library(messageviewer_bodypartformatter_application_mstnef MODULE application_ms-tnef.cpp)
target_link_libraries(messageviewer_bodypartformatter_application_mstnef
  Qt5::Core
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::PimUtils
  KF5::Tnef
  messagecore
  messageviewer
)
install(TARGETS messageviewer_bodypartformatter_application_mstnef DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES application_ms-tnef.desktop DESTINATION ${DATA_INSTALL_DIR}/messageviewer/plugins/bodypartformatter)
