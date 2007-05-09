include_directories(${STRIGI_INCLUDE_DIR})


########### next target ###############

set(kfile_vcf_PART_SRCS kfile_vcf.cpp )

kde4_automoc(${kfile_vcf_PART_SRCS})

kde4_add_plugin(kfile_vcf ${kfile_vcf_PART_SRCS})



target_link_libraries(kfile_vcf  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_vcf  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kfile_vcf.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

