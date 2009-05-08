



########### next target ###############

set(ldifvcardthumbnail_PART_SRCS ldifvcardcreator.cpp )


kde4_add_plugin(ldifvcardthumbnail ${ldifvcardthumbnail_PART_SRCS})



target_link_libraries(ldifvcardthumbnail ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS ldifvcardthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ldifvcardthumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

