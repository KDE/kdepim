include_directories(
  ${CMAKE_SOURCE_DIR}/akregator2/interfaces
  ${CMAKE_SOURCE_DIR}/akregator2/src
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
)

set(akregator_sharemicroblog_plugin_SRCS sharemicroblogplugin.cpp)

kde4_add_plugin(akregator2_sharemicroblog_plugin ${akregator_sharemicroblog_plugin_SRCS})

target_link_libraries(akregator2_sharemicroblog_plugin
  akregator2private
  akregator2interfaces
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDEPIMLIBS_KRSS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
)

install(FILES akregator2_sharemicroblog_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator2_sharemicroblog_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES akregator2_sharemicroblog_plugin.rc DESTINATION ${DATA_INSTALL_DIR}/akregator_sharemicroblog_plugin)

########### settings target ###############

set(kcm_akrshareconfig_SRCS
  akregator_config_sharemicroblog.cpp
)

kde4_add_kcfg_files(kcm_akrshareconfig_SRCS sharesettings.kcfgc)
kde4_add_ui_files(kcm_akrshareconfig_SRCS configurationwidget.ui)
kde4_add_plugin(akregator2_config_sharemicroblog ${kcm_akrshareconfig_SRCS})

target_link_libraries(akregator2_config_sharemicroblog
  akregator2interfaces
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator2_config_sharemicroblog.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator2_config_sharemicroblog DESTINATION ${PLUGIN_INSTALL_DIR})
