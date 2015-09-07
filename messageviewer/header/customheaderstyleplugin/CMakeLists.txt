set(messageviewer_customheaderstyleplugin_SRCS
   customheaderstyleplugin.cpp   
)

add_library(messageviewer_customheaderstyleplugin MODULE ${messageviewer_customheaderstyleplugin_SRCS})

target_link_libraries(messageviewer_customheaderstyleplugin
  messageviewer
  #Temporary
  grantleetheme
)

install(TARGETS messageviewer_customheaderstyleplugin DESTINATION ${PLUGIN_INSTALL_DIR}/messageviewer)
kcoreaddons_desktop_to_json(messageviewer_customheaderstyleplugin messageviewer_customheaderstyleplugin.desktop)
