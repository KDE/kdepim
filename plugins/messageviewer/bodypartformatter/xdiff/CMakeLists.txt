
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)




set(messageviewer_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp)
add_library(messageviewer_bodypartformatter_text_xdiff MODULE ${messageviewer_bodypartformatter_text_xdiff_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_xdiff  Qt5::Core KF5::I18n KF5::CoreAddons KF5::IconThemes KF5::WidgetsAddons)
install(TARGETS messageviewer_bodypartformatter_text_xdiff  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES text_xdiff.desktop DESTINATION ${DATA_INSTALL_DIR}/messageviewer/plugins/bodypartformatter)
