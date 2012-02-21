include_directories( ${CMAKE_SOURCE_DIR}/akregator2/interfaces )
include_directories( ${CMAKE_SOURCE_DIR}/akregator2/src )
include_directories( ${CMAKE_SOURCE_DIR} ) 
include_directories( ${KDE4_INCLUDE_DIR} )
include_directories( ${QT_INCLUDES} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${Boost_INCLUDE_DIRS} )


########### next target ###############

set(kcm_akronlinesyncconfig_SRCS 
    akregator2_config_onlinesync.cpp
    ui/configurationdialog.cpp
    ui/configurationwidget.cpp
    ui/configurationdialogadd.cpp )

kde4_add_kcfg_files(kcm_akronlinesyncconfig_SRCS onlinesyncsettings.kcfgc)

kde4_add_ui_files(kcm_akronlinesyncconfig_SRCS ui/configurationwidget.ui)
kde4_add_ui_files(kcm_akronlinesyncconfig_SRCS ui/configurationdialogadd.ui)

kde4_add_plugin(akregator2_config_onlinesync ${kcm_akronlinesyncconfig_SRCS})

target_link_libraries( akregator2_config_onlinesync akregator2interfaces ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

install( FILES akregator2_config_onlinesync.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator2_config_onlinesync  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(akregator2_onlinesync_plugin_SRCS 
    sync/feedsync.cpp
    sync/subscriptionlist.cpp 
    sync/akregator2.cpp 
    sync/googlereader.cpp 
    sync/opml.cpp 
    ui/configurationdialog.cpp 
    ui/configurationwidget.cpp 
    ui/configurationdialogadd.cpp 
    onlinesyncplugin.cpp )

kde4_add_ui_files(akregator2_onlinesync_plugin_SRCS 
    ui/configurationwidget.ui
    ui/configurationdialogadd.ui)

kde4_add_kcfg_files(akregator2_onlinesync_plugin_SRCS onlinesyncsettings.kcfgc)

kde4_add_plugin(akregator2_onlinesync_plugin ${akregator2_onlinesync_plugin_SRCS})

target_link_libraries(akregator2_onlinesync_plugin akregator2interfaces ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

install(FILES akregator2_onlinesync_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator2_onlinesync_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES akregator2_onlinesync_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator2_onlinesync_plugin)
