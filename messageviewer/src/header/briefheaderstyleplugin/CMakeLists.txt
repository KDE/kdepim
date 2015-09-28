set(messageviewer_briefheaderstyleplugin_SRCS
   briefheaderstyleplugin.cpp
   briefheaderstyleinterface.cpp 
   briefheaderstyle.cpp
   briefheaderstrategy.cpp
)

add_library(messageviewer_briefheaderstyleplugin MODULE ${messageviewer_briefheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_briefheaderstyleplugin
  KF5::MessageViewer
  KF5::XmlGui
)

install(TARGETS messageviewer_briefheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_briefheaderstyleplugin messageviewer_briefheaderstyleplugin.desktop)
