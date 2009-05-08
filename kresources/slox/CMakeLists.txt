check_include_files(sys/limits.h HAVE_SYS_LIMITS_H)
check_include_files(values.h HAVE_VALUES_H)

configure_file (config-slox.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-slox.h)

add_subdirectory( tests )


########### next target ###############

set(kslox_LIB_SRCS 
   sloxaccounts.cpp 
   webdavhandler.cpp 
   sloxfolder.cpp 
   sloxfoldermanager.cpp 
   sloxfolderdialog.cpp 
   sloxbase.cpp )


kde4_add_library(kslox SHARED ${kslox_LIB_SRCS})

target_link_libraries(kslox  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kdepim )

set_target_properties(kslox PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kslox ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kabc_slox_LIB_SRCS kabcresourceslox.cpp kabcresourcesloxconfig.cpp )


kde4_add_kcfg_files(kabc_slox_LIB_SRCS kabcsloxprefs.kcfgc )

kde4_add_library(kabc_slox SHARED ${kabc_slox_LIB_SRCS})

target_link_libraries(kabc_slox  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} kslox kdepim)

set_target_properties(kabc_slox PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabc_slox ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcal_slox_LIB_SRCS kcalresourceslox.cpp kcalresourcesloxconfig.cpp )


kde4_add_kcfg_files(kcal_slox_LIB_SRCS kcalsloxprefs.kcfgc )

kde4_add_library(kcal_slox SHARED ${kcal_slox_LIB_SRCS})

target_link_libraries(kcal_slox  ${KDE4_KDECORE_LIBS} kslox ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS} kdepim )

set_target_properties(kcal_slox PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_slox ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcal_slox_plugin_PART_SRCS kcalresourceslox_plugin.cpp )


kde4_add_plugin(kcal_slox_plugin ${kcal_slox_plugin_PART_SRCS})
# TODO: plugin and shared lib could not have the same name
if (NOT MSVC) 
set_target_properties(kcal_slox_plugin PROPERTIES OUTPUT_NAME kcal_slox)
endif (NOT MSVC) 


target_link_libraries(kcal_slox_plugin  ${KDE4_KDECORE_LIBS} kcal_slox ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS})

install(TARGETS kcal_slox_plugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kabc_slox_plugin_PART_SRCS kabcresourceslox_plugin.cpp)


kde4_add_plugin(kabc_slox_plugin ${kabc_slox_plugin_PART_SRCS})
# TODO: plugin and shared lib could not have the same name
if (NOT MSVC) 
set_target_properties(kabc_slox_plugin PROPERTIES OUTPUT_NAME kabc_slox)
endif (NOT MSVC) 



target_link_libraries(kabc_slox_plugin  ${KDE4_KDECORE_LIBS} kabc_slox ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KABC_LIBS})

install(TARGETS kabc_slox_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})



########### install files ###############

install( FILES kabc_slox.desktop kabc_ox.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kcal_slox.desktop kcal_ox.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)


