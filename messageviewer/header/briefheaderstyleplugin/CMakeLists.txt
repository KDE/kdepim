set(messageviewer_briefheaderstyleplugin_SRCS
   briefheaderstyleplugin.cpp   
)

add_library(messageviewer_briefheaderstyleplugin MODULE ${messageviewer_briefheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_briefheaderstyleplugin
  messageviewer
  #Temporary
  grantleetheme
)

install(TARGETS messageviewer_briefheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_briefheaderstyleplugin messageviewer_briefheaderstyleplugin.desktop)
