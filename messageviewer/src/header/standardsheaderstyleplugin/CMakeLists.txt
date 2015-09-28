set(messageviewer_standardsheaderstyleplugin_SRCS
   standardsheaderstyleplugin.cpp
   standardsheaderstyleinterface.cpp
   standardheaderstrategy.cpp
)

add_library(messageviewer_standardsheaderstyleplugin MODULE ${messageviewer_standardsheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_standardsheaderstyleplugin
  KF5::MessageViewer
  KF5::XmlGui
  KF5::GrantleeTheme
)

install(TARGETS messageviewer_standardsheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_standardsheaderstyleplugin messageviewer_standardsheaderstyleplugin.desktop)
