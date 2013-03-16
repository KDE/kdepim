set( kcm_activity_SRCS
     kcmpimactivity.cpp
)

kde4_add_plugin(kcm_pimactivity ${kcm_activity_SRCS})

target_link_libraries(kcm_pimactivity ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} pimactivity)

install(TARGETS kcm_pimactivity DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcmpimactivity.desktop DESTINATION ${SERVICES_INSTALL_DIR})
