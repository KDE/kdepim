set(messageviewer_allheaderstyleplugin_SRCS
   allheaderstyleplugin.cpp
   allheaderstyleinterface.cpp
)

add_library(messageviewer_allheaderstyleplugin MODULE ${messageviewer_allheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_allheaderstyleplugin
  messageviewer
  #Temporary
  KF5::GrantleeTheme
  KF5::XmlGui
)

install(TARGETS messageviewer_allheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_allheaderstyleplugin messageviewer_allheaderstyleplugin.desktop)
