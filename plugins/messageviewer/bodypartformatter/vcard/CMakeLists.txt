
include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon/
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${Boost_INCLUDE_DIRS}
)

add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_text_vcard_plugin\")

set(messageviewer_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp vcardmemento.cpp updatecontactjob.cpp)
add_library(messageviewer_bodypartformatter_text_vcard MODULE ${messageviewer_bodypartformatter_text_vcard_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_vcard  KF5::Mime KF5::Contacts kdepim messageviewer)
install(TARGETS messageviewer_bodypartformatter_text_vcard DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES text_vcard.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
