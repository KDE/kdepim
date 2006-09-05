



########### next target ###############

set(ldifvcardthumbnail_PART_SRCS ldifvcardcreator.cpp )

kde4_automoc(${ldifvcardthumbnail_PART_SRCS})

kde4_add_plugin(ldifvcardthumbnail ${ldifvcardthumbnail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ldifvcardthumbnail )

target_link_libraries(ldifvcardthumbnail  ${KDE4_KDEFX_LIBS} kabc)

install(TARGETS ldifvcardthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ldifvcardthumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

