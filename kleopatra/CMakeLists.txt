project(kleopatra)

set( kleopatra_version 1.9.0 )
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
  if ( NOT KdeSubversion_FOUND )
    find_package( KdeSubversion )
  endif ( NOT KdeSubversion_FOUND )
  if ( KdeSubversion_FOUND )
    KdeSubversion_WC_INFO( ${PROJECT_SOURCE_DIR} Kleopatra )
    string( SUBSTRING "${Kleopatra_WC_LAST_CHANGED_DATE}" 0 10 Kleopatra_WC_LAST_CHANGED_DATE )
    set( kleopatra_version "${kleopatra_version} (svn-${Kleopatra_WC_REVISION}, ${Kleopatra_WC_LAST_CHANGED_DATE})" )
  endif ( KdeSubversion_FOUND )
endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kleopatra.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${QGPGME_INCLUDES} ${GPGME_INCLUDES} )
if (USABLE_ASSUAN_FOUND)
   include_directories(${ASSUAN_INCLUDES})
endif(USABLE_ASSUAN_FOUND)
add_definitions ( -DQT_STL -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS ${KDE4_ENABLE_EXCEPTIONS} -D_ASSUAN_ONLY_GPG_ERRORS -UQT_NO_STL )

add_subdirectory( pics )
add_subdirectory( kgpgconf )
add_subdirectory( tests )

if ( WIN32 )
  set( _kleopatra_extra_uiserver_SRCS
    utils/gnupg-registry.c
    uiserver/uiserver_win.cpp
    )
else ( WIN32 )
  set( _kleopatra_extra_uiserver_SRCS
    uiserver/uiserver_unix.cpp
    )
endif ( WIN32 )


if ( USABLE_ASSUAN_FOUND )
  set(_kleopatra_uiserver_SRCS
    uiserver/uiserver.cpp
    ${_kleopatra_extra_uiserver_SRCS}
    uiserver/assuanserverconnection.cpp
    uiserver/echocommand.cpp
    uiserver/decryptverifycommand.cpp
    uiserver/signemailcontroller.cpp
    uiserver/signcommand.cpp
    uiserver/signencryptfilescommand.cpp
    uiserver/encryptemailcontroller.cpp
    uiserver/prepencryptcommand.cpp
    uiserver/encryptcommand.cpp
    )

  if ( WIN32 )
    set( _kleopatra_uiserver_extra_libs ${ASSUAN_VANILLA_LIBRARIES} )
  else ( WIN32 )
    set( _kleopatra_uiserver_extra_libs ${ASSUAN_PTHREAD_LIBRARIES} )
  endif( WIN32 )

  if ( HAVE_GPG_ERR_SOURCE_KLEO )
    add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_KLEO )
  else( HAVE_GPG_ERR_SOURCE_KLEO )
    add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_1 )
  endif( HAVE_GPG_ERR_SOURCE_KLEO )

else ( USABLE_ASSUAN_FOUND )
  set(_kleopatra_uiserver_SRCS)
endif ( USABLE_ASSUAN_FOUND )

kde4_add_ui_files( _kleopatra_uiserver_SRCS 
                   crypto/gui/certificateselectionwidget.ui 
                   crypto/gui/signingcertificateselectionwidget.ui)


set( _kleopatra_common_SRCS

  utils/kdpipeiodevice.cpp
  utils/kdlogtextwidget.cpp

  utils/hex.cpp
  utils/input.cpp
  utils/output.cpp
  utils/exception.cpp
  utils/formatting.cpp
  utils/wsastarter.cpp
  utils/classify.cpp
  utils/iodevicelogger.cpp
  utils/log.cpp

  models/keycache.cpp
  models/keylistmodel.cpp
  models/keylistsortfilterproxymodel.cpp
  models/subkeylistmodel.cpp
  models/useridlistmodel.cpp

  controllers/keylistcontroller.cpp

  crypto/certificateresolver.cpp
  crypto/task.cpp
  crypto/encryptemailtask.cpp
  crypto/signencryptfilestask.cpp
  crypto/signemailtask.cpp

  crypto/gui/wizard.cpp
  crypto/gui/wizardpage.cpp
  crypto/gui/scrollarea.cpp
  crypto/gui/signingcertificateselectiondialog.cpp
  crypto/gui/certificateselectiondialog.cpp

  crypto/gui/resultdisplaywidget.cpp

  crypto/gui/decryptverifyoperationwidget.cpp
  crypto/gui/decryptverifyresultwidget.cpp
  crypto/gui/decryptverifywizard.cpp

  crypto/gui/objectspage.cpp
  crypto/gui/resolverecipientspage.cpp
  crypto/gui/wizardresultpage.cpp
  crypto/gui/signerresolvepage.cpp
  crypto/gui/signemailwizard.cpp
  crypto/gui/signencryptfileswizard.cpp
  crypto/gui/signencryptwizard.cpp

  crypto/controller.cpp
  crypto/signencryptfilescontroller.cpp

  commands/command.cpp
  commands/detailscommand.cpp
  commands/exportcertificatecommand.cpp
  commands/importcertificatecommand.cpp
  commands/refreshkeyscommand.cpp
  commands/deletecertificatescommand.cpp
  commands/signencryptfilescommand.cpp
  commands/clearcrlcachecommand.cpp
  commands/dumpcrlcachecommand.cpp
  commands/importcrlcommand.cpp

  ${_kleopatra_uiserver_files}

  certificatewizardimpl.cpp
  aboutdata.cpp
  systemtrayicon.cpp
  exportcertificatesdialog.cpp
  certificateinfowidgetimpl.cpp
  main.cpp )

kde4_add_ui_files( _kleopatra_common_SRCS certificateinfowidget.ui certificatewizard.ui )

if (ONLY_KLEO)
   add_definitions( -DONLY_KLEO )
endif(ONLY_KLEO)

set( _kleopatra_mainwindow_SRCS
     action_data.cpp
     mainwindow.cpp 
     searchbar.cpp
     tabwidget.cpp
)

if ( KLEO_BUILD_OLD_MAINWINDOW )
  add_definitions( -DKLEO_BUILD_OLD_MAINWINDOW )
  set(libconf_SRCS  conf/configuredialog.cpp )
  add_subdirectory( conf )
  add_subdirectory( kwatchgnupg )

  set( _kleopatra_mainwindow_SRCS
    ${libconf_SRCS}
    customactions.cpp
    certmanager.cpp
    hierarchyanalyser.cpp
    crlview.cpp
    certlistview.cpp )

endif ( KLEO_BUILD_OLD_MAINWINDOW )

add_definitions ( -DKDE_DEFAULT_DEBUG_AREA=5151 )

kde4_add_executable(kleopatra_bin ${_kleopatra_common_SRCS} ${_kleopatra_mainwindow_SRCS} ${_kleopatra_uiserver_SRCS} )
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

if ( KLEO_BUILD_OLD_MAINWINDOW )
  if ( ONLY_KLEO )
    set( _kleopatra_extra_libs ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )
  else ( ONLY_KLEO )
    set( _kleopatra_extra_libs ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} )
  endif ( ONLY_KLEO )
else ( KLEO_BUILD_OLD_MAINWINDOW )
  set( _kleopatra_extra_libs ${QT_QT3SUPPORT_LIBRARY} )
endif ( KLEO_BUILD_OLD_MAINWINDOW )

target_link_libraries(kleopatra_bin ${_kleopatra_extra_libs} kleo ${QGPGME_LIBRARIES} ${KDE4_KMIME_LIBRARY} ${_kleopatra_uiserver_extra_libs} )

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kleopatraui.rc kleopatra_newui.rc  DESTINATION ${DATA_INSTALL_DIR}/kleopatra)

kde4_install_icons( ${ICON_INSTALL_DIR} )
