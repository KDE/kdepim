
add_subdirectory( soap )
add_subdirectory( kioslave )

########### next target ###############
### kcal groupwise shared library

set(kcal_groupwise_LIB_SRCS
   kcal_resourcegroupwise.cpp
   kcal_resourcegroupwiseconfig.cpp
   kcal_groupwiseprefs.cpp
   groupwisesettingswidget.cpp
)

kde4_add_ui_files(kcal_groupwise_LIB_SRCS groupwisesettingswidgetbase.ui)

kde4_add_kcfg_files(kcal_groupwise_LIB_SRCS kcal_groupwiseprefsbase.kcfgc)

kde4_add_library (kcalgroupwise SHARED ${kcal_groupwise_LIB_SRCS})

target_link_libraries(kcalgroupwise  ${KDE4_KDECORE_LIBS} gwsoap ${KDEPIMLIBS_KCAL_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kdepim)

set_target_properties(kcalgroupwise PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kcalgroupwise ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############
### groupwise kcal plugin

set( kcal_groupwise_plugin_SRCS kcal_resourcegroupwise_plugin.cpp)

kde4_add_plugin(kcal_groupwise ${kcal_groupwise_plugin_SRCS})

target_link_libraries(kcal_groupwise ${KDE4_KDECORE_LIBS} kcalgroupwise ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS})

install(TARGETS kcal_groupwise DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############
### kabc_groupwise shared lib

set(kabc_groupwise_LIB_SRCS kabc_resourcegroupwise.cpp kabc_resourcegroupwiseconfig.cpp)

kde4_add_kcfg_files(kabc_groupwise_LIB_SRCS kabc_groupwiseprefs.kcfgc)

kde4_add_library(kabcgroupwise SHARED ${kabc_groupwise_LIB_SRCS})

target_link_libraries(kabcgroupwise ${KDE4_KDECORE_LIBS} gwsoap ${KDEPIMLIBS_KCAL_LIBS} kdepim ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

set_target_properties(kabcgroupwise PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kabcgroupwise ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############
### kabc groupwise plugin

set( kabc_groupwise_plugin_SRCS kabc_resourcegroupwise_plugin.cpp)

kde4_add_plugin(kabc_groupwise ${kabc_groupwise_plugin_SRCS})

target_link_libraries(kabc_groupwise ${KDE4_KDECORE_LIBS} kabcgroupwise ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS})

install(TARGETS kabc_groupwise DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kabc_groupwise.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kcal_groupwise.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
