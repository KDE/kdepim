
add_definitions(-DTRANSLATION_DOMAIN=\"messageviewer_text_xdiff_plugin\")
set(messageviewer_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp)
add_library(messageviewer_bodypartformatter_text_xdiff MODULE ${messageviewer_bodypartformatter_text_xdiff_PART_SRCS})
target_link_libraries(messageviewer_bodypartformatter_text_xdiff  Qt5::Core KF5::I18n KF5::CoreAddons KF5::IconThemes KF5::WidgetsAddons KF5::MessageViewer)
install(TARGETS messageviewer_bodypartformatter_text_xdiff  DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES text_xdiff.desktop DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer/plugins/bodypartformatter)
