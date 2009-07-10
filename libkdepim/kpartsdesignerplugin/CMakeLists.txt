
########### next target ###############

set(kpartsdesignerplugin_PART_SRCS kpartsdesignerplugin.cpp )


kde4_add_plugin(kpartsdesignerplugin ${kpartsdesignerplugin_PART_SRCS})



target_link_libraries(kpartsdesignerplugin  ${KDE4_KPARTS_LIBS} )

install(TARGETS kpartsdesignerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
