
add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_text_vcard_plugin\")

set(messageviewer_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp vcardmemento.cpp updatecontactjob.cpp vcard_debug.cpp)
add_library(messageviewer_bodypartformatter_text_vcard MODULE ${messageviewer_bodypartformatter_text_vcard_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_vcard  KF5::Mime KF5::Contacts kdepim messageviewer KF5::KDELibs4Support)
install(TARGETS messageviewer_bodypartformatter_text_vcard DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES text_vcard.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
