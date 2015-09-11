set(messageviewer_standardsheaderstyleplugin_SRCS
   standardsheaderstyleplugin.cpp
   standardsheaderstyleinterface.cpp
)

add_library(messageviewer_standardsheaderstyleplugin MODULE ${messageviewer_standardsheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_standardsheaderstyleplugin
  KF5::MessageViewer
  #Temporary
  KF5::GrantleeTheme
  KF5::XmlGui
)

install(TARGETS messageviewer_standardsheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_standardsheaderstyleplugin messageviewer_standardsheaderstyleplugin.desktop)
