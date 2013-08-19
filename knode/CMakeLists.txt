project(knode)

add_subdirectory( tests )

add_subdirectory( pics )
add_subdirectory( filters )
add_subdirectory( kconf_update )

include_directories(
  ${KDE4_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${CMAKE_SOURCE_DIR}/messagecomposer
)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5003 )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )


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
   composer/attachment_view.cpp
   composer/followupto_combobox.cpp
   configuration/identity_widget.cpp
   configuration/identity_edition_dialog.cpp
   utils/locale.cpp
   utils/startup.cpp
   kscoring.cpp
   kscoringeditor.cpp
   treewidget.cpp
   foldertreewidget.cpp
)

qt4_add_dbus_adaptor( knodecommon_LIB_SRCS org.kde.knode.xml knmainwidget.h KNMainWidget)



kde4_add_ui_files(knodecommon_LIB_SRCS
   nntpaccountdialog_base.ui
   nntpaccountlistwidget_base.ui
   readnewsgeneralwidget_base.ui
   readnewsnavigationwidget_base.ui
   readnewsviewerwidget_base.ui
   postnewstechnicalwidget_base.ui
   postnewscomposerwidget_base.ui
   composer/composer_view.ui
   configuration/identity_widget.ui
   configuration/identity_edition_dialog.ui
)

kde4_add_kcfg_files(knodecommon_LIB_SRCS settings_base.kcfgc )

kde4_add_library(knodecommon ${LIBRARY_TYPE} ${knodecommon_LIB_SRCS})

target_link_libraries(knodecommon
  ${KDEPIMLIBS_KMIME_LIBS} ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS}
  ${KDE4_KDE3SUPPORT_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KHTML_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
  kleo
  kdepim kpgp
  messagecomposer
  messageviewer
)

target_link_libraries(knodecommon LINK_INTERFACE_LIBRARIES ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

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

kde4_add_app_icon(knode_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-knode.png")

kde4_add_executable(knode ${knode_SRCS})

target_link_libraries(knode ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} kdepim knodecommon)

install(TARGETS knode  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS KNode.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES knode_config_identity.desktop knode_config_accounts.desktop knode_config_appearance.desktop knode_config_read_news.desktop knode_config_post_news.desktop knode_config_privacy.desktop knode_config_cleanup.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES headers.rc knodeui.rc knreaderui.rc kncomposerui.rc  DESTINATION ${DATA_INSTALL_DIR}/knode)
install(FILES org.kde.knode.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

add_subdirectory(icons)
