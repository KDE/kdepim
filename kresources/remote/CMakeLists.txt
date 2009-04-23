



########### next target ###############

set(kcal_resourceremote_LIB_SRCS resourceremote.cpp resourceremoteconfig.cpp )


kde4_add_library(kcal_resourceremote SHARED ${kcal_resourceremote_LIB_SRCS})

target_link_libraries(kcal_resourceremote  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KIO_LIBS} kdepim )

set_target_properties(kcal_resourceremote PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_resourceremote  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_remote_PART_SRCS resourceremote_plugin.cpp )


kde4_add_plugin(kcal_remote ${kcal_remote_PART_SRCS})



target_link_libraries(kcal_remote  ${KDE4_KDECORE_LIBS} kcal_resourceremote ${KDE4_KRESOURCES_LIBS} kdepim )

install(TARGETS kcal_remote  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES remote.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)

