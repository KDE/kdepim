
include_directories(
	${CMAKE_SOURCE_DIR}/kresources/lib
	${CMAKE_BINARY_DIR}/kresources/lib
	
)


set(libkgroupdav_SRCS groupdavglobals.cpp)

########### next target ###############

set(kcal_groupdav_LIB_SRCS groupdavcalendaradaptor.cpp kcal_resourcegroupdav.cpp ${libkgroupdav_SRCS})


kde4_add_library(kcal_groupdav SHARED ${kcal_groupdav_LIB_SRCS})

target_link_libraries(kcal_groupdav  ${KDE4_KDECORE_LIBS} kgroupwaredav ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS} kdepim)

set_target_properties(kcal_groupdav PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_groupdav ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kabc_groupdav_LIB_SRCS kabc_resourcegroupdav.cpp groupdavaddressbookadaptor.cpp ${libkgroupdav_SRCS})


kde4_add_library(kabc_groupdav SHARED ${kabc_groupdav_LIB_SRCS})

target_link_libraries(kabc_groupdav  ${KDE4_KDECORE_LIBS} kgroupwaredav ${KDEPIMLIBS_KCAL_LIBS} kdepim ${KDEPIMLIBS_KABC_LIBS})

set_target_properties(kabc_groupdav PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabc_groupdav ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############


########### next target ###############

set(kcal_groupdav_plugin_PART_SRCS kcal_resourcegroupdav_plugin.cpp)


kde4_add_plugin(kcal_groupdav_plugin ${kcal_groupdav_plugin_PART_SRCS})
# TODO: plugin and shared lib could not have the same name
if (NOT MSVC) 
set_target_properties(kcal_groupdav_plugin PROPERTIES OUTPUT_NAME kcal_groupdav)
endif (NOT MSVC) 

target_link_libraries(kcal_groupdav_plugin  ${KDE4_KDECORE_LIBS} kcal_groupdav ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kgroupwarebase)

install(TARGETS kcal_groupdav_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kabc_groupdav_plugin_PART_SRCS kabc_resourcegroupdav_plugin.cpp )


kde4_add_plugin(kabc_groupdav_plugin ${kabc_groupdav_plugin_PART_SRCS})
# TODO: plugin and shared lib could not have the same name
if (NOT MSVC) 
set_target_properties(kabc_groupdav_plugin PROPERTIES OUTPUT_NAME kabc_groupdav)
endif (NOT MSVC) 

target_link_libraries(kabc_groupdav_plugin  ${KDE4_KDECORE_LIBS} kabc_groupdav ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kgroupwarebase)

install(TARGETS kabc_groupdav_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabc_groupdav.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kcal_groupdav.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)

install(FILES uninstall.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc/ RENAME kabc_opengroupware.desktop)
install(FILES uninstall.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal/ RENAME kcal_opengroupware.desktop)


