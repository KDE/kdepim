
add_subdirectory( tests )


set(libfeatureplancommon_SRCS)

kode_add_xml_parser(libfeatureplancommon_SRCS kde-features.rng)
kde4_add_kcfg_files(libfeatureplancommon_SRCS prefs.kcfgc )

########### next target ###############

set(kcal_resourcefeatureplan_LIB_SRCS ${libfeatureplancommon_SRCS} kcal_resourcefeatureplan.cpp kcal_resourcefeatureplanconfig.cpp )


kde4_add_library(kcal_resourcefeatureplan SHARED ${kcal_resourcefeatureplan_LIB_SRCS})

target_link_libraries(kcal_resourcefeatureplan  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(kcal_resourcefeatureplan PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_resourcefeatureplan ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############


########### next target ###############

set(kcal_resourcefeatureplan_plugin_PART_SRCS kcal_resourcefeatureplanplugin.cpp)


kde4_add_plugin(kcal_resourcefeatureplan_plugin ${kcal_resourcefeatureplan_plugin_PART_SRCS})

target_link_libraries(kcal_resourcefeatureplan_plugin ${KDE4_KDECORE_LIBS} kcal_resourcefeatureplan ${KDE4_KRESOURCES_LIBS})

install(TARGETS kcal_resourcefeatureplan_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kcal_resourcefeatureplan.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
