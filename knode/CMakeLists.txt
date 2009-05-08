project(knode)

add_subdirectory( tests )

add_subdirectory( pics )
add_subdirectory( filters )

include_directories( ${KDE4_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkdepim )

add_definitions(-DI_KNOW_KSOCKS_ISNT_PUBLIC )
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(knodecommon_LIB_SRCS
   knconfigmanager.cpp
   knconfig.cpp
   knconfigwidgets.cpp
   settings.cpp
   scheduler.cpp
   knjobdata.cpp
   mailsendjob.cpp
   nntpjobs.cpp
   knaccountmanager.cpp
   kncollection.cpp
   kncollectionviewitem.cpp
   knserverinfo.cpp
   knnntpaccount.cpp
   kngroupmanager.cpp
   knarticlecollection.cpp
   kngroup.cpp
   kngroupbrowser.cpp
   kngroupselectdialog.cpp
   kngroupdialog.cpp
   kngrouppropdlg.cpp
   knfoldermanager.cpp
   knfolder.cpp
   knmemorymanager.cpp
   kncleanup.cpp
   knconvert.cpp
   knarticlemanager.cpp
   knarticle.cpp
   kndisplayedheader.cpp
   knsourceviewwindow.cpp
   knarticlewindow.cpp
   knhdrviewitem.cpp
   kncomposer.cpp
   kncomposerview.cpp
   kncomposereditor.cpp
   knarticlefactory.cpp
   knfiltermanager.cpp
   knstatusfilter.cpp
   knstringfilter.cpp
   knrangefilter.cpp
   knarticlefilter.cpp
   knfilterconfigwidget.cpp
   knfilterdialog.cpp
   knsearchdialog.cpp
   utilities.cpp
   knscoring.cpp
   headerview.cpp
   knmainwidget.cpp
   aboutdata.cpp
   knglobals.cpp
   kncollectionview.cpp
   articlewidget.cpp
   csshelper.cpp
   utils/locale.cpp
)

qt4_add_dbus_adaptor( knodecommon_LIB_SRCS org.kde.knode.xml knmainwidget.h KNMainWidget)



kde4_add_ui_files(knodecommon_LIB_SRCS
   nntpaccountdialog_base.ui
   nntpaccountlistwidget_base.ui
   readnewsgeneralwidget_base.ui
   readnewsnavigationwidget_base.ui
   readnewsviewerwidget_base.ui
   postnewstechnicalwidget_base.ui
   postnewscomposerwidget_base.ui )

kde4_add_kcfg_files(knodecommon_LIB_SRCS settings_base.kcfgc )

kde4_add_library(knodecommon SHARED ${knodecommon_LIB_SRCS})

target_link_libraries(knodecommon ${KDE4_KMIME_LIBRARY} ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS}
                              ${KDE4_KDE3SUPPORT_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KHTML_LIBS}
                              ${KDE4_KPIMUTILS_LIBRARY} ${KDE4_MAILTRANSPORT_LIBRARY}
                              kdepim kpgp)

target_link_libraries(knodecommon LINK_INTERFACE_LIBRARIES ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

set_target_properties(knodecommon
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knodecommon ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcm_knode_PART_SRCS knconfigpages.cpp )


kde4_add_plugin(kcm_knode ${kcm_knode_PART_SRCS})



target_link_libraries(kcm_knode  ${KDE4_KDECORE_LIBS} knodecommon )

install(TARGETS kcm_knode  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(knodepart_PART_SRCS knode_part.cpp )


kde4_add_plugin(knodepart ${knodepart_PART_SRCS})



target_link_libraries(knodepart  ${KDE4_KPARTS_LIBS} knodecommon )

install(TARGETS knodepart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(knode_SRCS knode.cpp knapplication.cpp main.cpp )


# todo: move to Oxygen icon
kde4_add_app_icon(knode_SRCS "hi*-app-knode2.png")

kde4_add_executable(knode ${knode_SRCS})

target_link_libraries(knode ${KDE4_KDECORE_LIBS} kdepim knodecommon)

install(TARGETS knode  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES knewsservice.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES KNode.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES knode_config_identity.desktop knode_config_accounts.desktop knode_config_appearance.desktop knode_config_read_news.desktop knode_config_post_news.desktop knode_config_privacy.desktop knode_config_cleanup.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES headers.rc knodeui.rc knreaderui.rc kncomposerui.rc  DESTINATION ${DATA_INSTALL_DIR}/knode)
install(FILES org.kde.knode.xml DESTINATION ${KDE4_DBUS_INTERFACES_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

