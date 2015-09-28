set(messageviewer_grantleeheaderstyleplugin_SRCS
   grantleeheaderstyleplugin.cpp
   grantleeheaderstyleinterface.cpp
   grantleeheaderstrategy.cpp
)

add_library(messageviewer_grantleeheaderstyleplugin MODULE ${messageviewer_grantleeheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_grantleeheaderstyleplugin
  KF5::MessageViewer
  KF5::GrantleeTheme
  KF5::XmlGui
)

install(TARGETS messageviewer_grantleeheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_grantleeheaderstyleplugin messageviewer_grantleeheaderstyleplugin.desktop)
