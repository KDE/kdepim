set(messageviewer_enterpriseheaderstyleplugin_SRCS
   enterpriseheaderstyleplugin.cpp
   enterpriseheaderstyleinterface.cpp 
   enterpriseheaderstyle.cpp
)

add_library(messageviewer_enterpriseheaderstyleplugin MODULE ${messageviewer_enterpriseheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_enterpriseheaderstyleplugin
  KF5::MessageViewer 
  KF5::GrantleeTheme
  KF5::XmlGui
)

install(TARGETS messageviewer_enterpriseheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_enterpriseheaderstyleplugin messageviewer_enterpriseheaderstyleplugin.desktop)
