
add_subdirectory( tests )


set(libfeatureplancommon_SRCS)

kode_add_xml_parser(libfeatureplancommon_SRCS kde-features.rng)
kde4_add_kcfg_files(libfeatureplancommon_SRCS prefs.kcfgc )

########### next target ###############

set(kcal_resourcefeatureplan_LIB_SRCS ${libfeatureplancommon_SRCS} kcal_resourcefeatureplan.cpp kcal_resourcefeatureplanconfig.cpp )

kde4_automoc(${kcal_resourcefeatureplan_LIB_SRCS})

kde4_add_library(kcal_resourcefeatureplan SHARED ${kcal_resourcefeatureplan_LIB_SRCS})

target_link_libraries(kcal_resourcefeatureplan  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim)

set_target_properties(kcal_resourcefeatureplan PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_resourcefeatureplan  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### next target ###############

set(kcal_resourcefeatureplan_PART_SRCS kcal_resourcefeatureplanplugin.cpp ${kcal_resourcefeatureplan_LIB_SRCS} ${libfeatureplancommon_SRCS})

kde4_automoc(${kcal_resourcefeatureplan_PART_SRCS})

kde4_add_plugin(kcal_resourcefeatureplan ${kcal_resourcefeatureplan_PART_SRCS})



target_link_libraries(kcal_resourcefeatureplan  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcal_resourcefeatureplan  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kcal_resourcefeatureplan.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
