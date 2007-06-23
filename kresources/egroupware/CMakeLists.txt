
include_directories( ${CMAKE_SOURCE_DIR}/libical/src/libical ${CMAKE_SOURCE_DIR}/libical/src/libicalss ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/knotes ${CMAKE_SOURCE_DIR}/kaddressbook/common  )


########### next target ###############


########### next target ###############

set(libegwcommon_SRCS 
	xmlrpciface.cpp
	synchronizer.cpp
	debugdialog.cpp
	)


set(kabc_xmlrpc_LIB_SRCS kabc_resourcexmlrpc.cpp kabc_resourcexmlrpcconfig.cpp ${libkabcommon_SRCS} ${libegwcommon_SRCS})

kde4_add_kcfg_files(kabc_xmlrpc_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )


kde4_automoc(${kabc_xmlrpc_LIB_SRCS})

kde4_add_kcfg_files(kabc_xmlrpc_LIB_SRCS kabc_egroupwareprefs.kcfgc )

kde4_add_library(kabc_xmlrpc SHARED ${kabc_xmlrpc_LIB_SRCS})

target_link_libraries(kabc_xmlrpc  ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS} kdepim )

set_target_properties(kabc_xmlrpc PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabc_xmlrpc  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_xmlrpc_LIB_SRCS ${libegwcommon_SRCS} 
   kcal_resourcexmlrpc.cpp 
   kcal_resourcexmlrpcconfig.cpp 
   todostatemapper.cpp )

kde4_add_kcfg_files(kcal_xmlrpc_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )

kde4_automoc(${kcal_xmlrpc_LIB_SRCS})

kde4_add_kcfg_files(kcal_xmlrpc_LIB_SRCS kcal_egroupwareprefs.kcfgc )

kde4_add_library(kcal_xmlrpc SHARED ${kcal_xmlrpc_LIB_SRCS})

target_link_libraries(kcal_xmlrpc  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim )

set_target_properties(kcal_xmlrpc PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_xmlrpc  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(knotes_xmlrpc_LIB_SRCS knotes_resourcexmlrpc.cpp knotes_resourcexmlrpcconfig.cpp  ${libegwcommon_SRCS} 
${libknotesresources_SRCS})

kde4_automoc(${knotes_xmlrpc_LIB_SRCS})

kde4_add_kcfg_files(knotes_xmlrpc_LIB_SRCS knotes_egroupwareprefs.kcfgc )

kde4_add_library(knotes_xmlrpc SHARED ${knotes_xmlrpc_LIB_SRCS})
set_target_properties(knotes_xmlrpc PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

target_link_libraries(knotes_xmlrpc  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KDEPRINT_LIBS} kdepim )


install(TARGETS knotes_xmlrpc  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kabc_xmlrpc_plugin_PART_SRCS kabc_resourcexmlrpcplugin.cpp )

kde4_automoc(${kabc_xmlrpc_plugin_PART_SRCS})

kde4_add_plugin(kabc_xmlrpc_plugin ${kabc_xmlrpc_plugin_PART_SRCS})
set_target_properties(kabc_xmlrpc_plugin PROPERTIES OUTPUT_NAME kabc_xmlrpc)



target_link_libraries(kabc_xmlrpc_plugin  ${KDE4_KDECORE_LIBS} kabc_xmlrpc)

install(TARGETS kabc_xmlrpc_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcal_xmlrpc_plugin_PART_SRCS kcal_resourcexmlrpcplugin.cpp )

kde4_automoc(${kcal_xmlrpc_plugin_PART_SRCS})

kde4_add_plugin(kcal_xmlrpc_plugin ${kcal_xmlrpc_plugin_PART_SRCS})
set_target_properties(kcal_xmlrpc_plugin PROPERTIES OUTPUT_NAME kcal_xmlrpc)



target_link_libraries(kcal_xmlrpc_plugin ${KDE4_KDECORE_LIBS} kcal_xmlrpc)

install(TARGETS kcal_xmlrpc_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(knotes_xmlrpc_plugin_PART_SRCS knotes_resourcexmlrpcplugin.cpp )

kde4_automoc(${knotes_xmlrpc_plugin_PART_SRCS})

kde4_add_plugin(knotes_xmlrpc_plugin ${knotes_xmlrpc_plugin_PART_SRCS})
set_target_properties(knotes_xmlrpc_plugin PROPERTIES OUTPUT_NAME knotes_xmlrpc)



target_link_libraries(knotes_xmlrpc_plugin ${KDE4_KDECORE_LIBS} knotes_xmlrpc )

install(TARGETS knotes_xmlrpc_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES knotes_xmlrpc.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/knotes)
install( FILES kabc_xmlrpc.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kabc_resourcexmlrpc.h egroupware_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/kabc)
install( FILES kcal_xmlrpc.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)


