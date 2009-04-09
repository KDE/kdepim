project(kleopatra)

include(MacroOptionalAddSubdirectory)

set( kleopatra_version 2.0.8 )
set( kleopatra_release FALSE )

if (NOT kleopatra_release)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
  if ( NOT KdeSubversion_FOUND )
    find_package( KdeSubversion )
  endif ( NOT KdeSubversion_FOUND )
  if ( KdeSubversion_FOUND )
    KdeSubversion_WC_INFO( ${PROJECT_SOURCE_DIR} Kleopatra )
    string( SUBSTRING "${Kleopatra_WC_LAST_CHANGED_DATE}" 0 10 Kleopatra_WC_LAST_CHANGED_DATE )
    set( kleopatra_version "${kleopatra_version}-svn${Kleopatra_WC_REVISION} (${Kleopatra_WC_LAST_CHANGED_DATE})" )
  endif ( KdeSubversion_FOUND )
endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
endif (NOT kleopatra_release)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kleopatra.h )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${Boost_INCLUDE_DIR}
  ${QGPGME_INCLUDES}
  ${GPGME_INCLUDES} )
if (USABLE_ASSUAN_FOUND)
   include_directories(${ASSUAN_INCLUDES})
endif(USABLE_ASSUAN_FOUND)
add_definitions ( -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS -D_ASSUAN_ONLY_GPG_ERRORS )
remove_definitions ( -DQT_NO_STL )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( pics )
add_subdirectory( conf )
if ( USABLE_ASSUAN_FOUND AND BUILD_libkleopatraclient )
  # can't use macro_optional_add_subdirectory here, since it's split
  # off into ConfigureChecks.cmake..
  add_subdirectory( libkleopatraclient )
endif ( USABLE_ASSUAN_FOUND AND BUILD_libkleopatraclient )
add_subdirectory( kgpgconf )
add_subdirectory( kwatchgnupg )
add_subdirectory( tests )

if ( WIN32 )
  set( _kleopatra_extra_uiserver_SRCS
    uiserver/uiserver_win.cpp
    )
  set( _kleopatra_extra_SRCS
    utils/gnupg-registry.c
    selftest/registrycheck.cpp
    )
else ( WIN32 )
  set( _kleopatra_extra_uiserver_SRCS
    uiserver/uiserver_unix.cpp
    )
  set( _kleopatra_extra_SRCS
    )
endif ( WIN32 )


if ( USABLE_ASSUAN_FOUND )
  add_definitions ( -DKLEOPATRACLIENT_NAMESPACE=KleopatraClientCopy -DKLEOPATRACLIENTCORE_EXPORT= )
  set(_kleopatra_uiserver_SRCS
    uiserver/uiserver.cpp
    ${_kleopatra_extra_uiserver_SRCS}
    uiserver/assuanserverconnection.cpp
    uiserver/echocommand.cpp
    uiserver/decryptverifycommandemailbase.cpp
    uiserver/decryptverifycommandfilesbase.cpp
    uiserver/signcommand.cpp
    uiserver/signencryptfilescommand.cpp
    uiserver/prepencryptcommand.cpp
    uiserver/encryptcommand.cpp
    uiserver/selectcertificatecommand.cpp
    uiserver/importfilescommand.cpp

    libkleopatraclient/core/command.cpp
    selftest/uiservercheck.cpp
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
                   crypto/gui/signingcertificateselectionwidget.ui)


set( _kleopatra_SRCS
  utils/gnupg-helper.cpp
  utils/filesystemwatcher.cpp
  utils/kdpipeiodevice.cpp
  utils/kdlogtextwidget.cpp
  utils/headerview.cpp
  utils/scrollarea.cpp
  utils/dragqueen.cpp
  utils/multivalidator.cpp
  utils/progressmanager.cpp
  utils/filedialog.cpp

  utils/hex.cpp
  utils/input.cpp
  utils/output.cpp
  utils/exception.cpp
  utils/formatting.cpp
  utils/validation.cpp
  utils/wsastarter.cpp
  utils/classify.cpp
  utils/iodevicelogger.cpp
  utils/log.cpp
  utils/action_data.cpp
  utils/types.cpp

  selftest/selftest.cpp
  selftest/enginecheck.cpp
  selftest/gpgconfcheck.cpp

  ${_kleopatra_extra_SRCS}

  models/keycache.cpp
  models/keylistmodel.cpp
  models/keylistsortfilterproxymodel.cpp
  models/subkeylistmodel.cpp
  models/useridlistmodel.cpp

  view/keylistcontroller.cpp
  view/keytreeview.cpp
  view/searchbar.cpp
  view/tabwidget.cpp

  dialogs/certificateselectiondialog.cpp
  dialogs/expirydialog.cpp
  dialogs/lookupcertificatesdialog.cpp
  dialogs/ownertrustdialog.cpp
  dialogs/selftestdialog.cpp
  dialogs/certifycertificatedialog.cpp
  dialogs/exportsecretkeydialog.cpp
  dialogs/adduseriddialog.cpp
  dialogs/certificatedetailsdialog.cpp
  dialogs/exportcertificatesdialog.cpp

  crypto/certificateresolver.cpp
  crypto/task.cpp
  crypto/taskcollection.cpp
  crypto/decryptverifytask.cpp
  crypto/decryptverifyemailcontroller.cpp
  crypto/decryptverifyfilescontroller.cpp
  crypto/encryptemailtask.cpp
  crypto/encryptemailcontroller.cpp
  crypto/signencryptfilestask.cpp
  crypto/signemailtask.cpp
  crypto/signemailcontroller.cpp

  crypto/gui/wizard.cpp
  crypto/gui/wizardpage.cpp
  crypto/gui/signingcertificateselectiondialog.cpp

  crypto/gui/resultitemwidget.cpp
  crypto/gui/resultlistwidget.cpp
  crypto/gui/resultpage.cpp

  crypto/gui/decryptverifyoperationwidget.cpp
  crypto/gui/decryptverifyfileswizard.cpp

  crypto/gui/objectspage.cpp
  crypto/gui/resolverecipientspage.cpp
  crypto/gui/signerresolvepage.cpp
  crypto/gui/encryptemailwizard.cpp
  crypto/gui/signemailwizard.cpp
  crypto/gui/signencryptfileswizard.cpp
  crypto/gui/signencryptwizard.cpp

  crypto/controller.cpp
  crypto/signencryptfilescontroller.cpp

  commands/command.cpp
  commands/gnupgprocesscommand.cpp
  commands/detailscommand.cpp
  commands/exportcertificatecommand.cpp
  commands/importcertificatescommand.cpp
  commands/importcertificatefromfilecommand.cpp
  commands/lookupcertificatescommand.cpp
  commands/reloadkeyscommand.cpp
  commands/refreshx509certscommand.cpp
  commands/refreshopenpgpcertscommand.cpp
  commands/deletecertificatescommand.cpp
  commands/decryptverifyfilescommand.cpp
  commands/signencryptfilescommand.cpp
  commands/encryptclipboardcommand.cpp
  commands/signclipboardcommand.cpp
  commands/decryptverifyclipboardcommand.cpp
  commands/clearcrlcachecommand.cpp
  commands/dumpcrlcachecommand.cpp
  commands/dumpcertificatecommand.cpp
  commands/importcrlcommand.cpp
  commands/changeexpirycommand.cpp
  commands/changeownertrustcommand.cpp
  commands/changepassphrasecommand.cpp
  commands/certifycertificatecommand.cpp
  commands/selftestcommand.cpp
  commands/exportsecretkeycommand.cpp
  commands/exportopenpgpcertstoservercommand.cpp
  commands/adduseridcommand.cpp
  commands/newcertificatecommand.cpp
  
  ${_kleopatra_uiserver_files}

  conf/configuredialog.cpp

  newcertificatewizard/listwidget.cpp
  newcertificatewizard/newcertificatewizard.cpp

  aboutdata.cpp
  systemtrayicon.cpp
  kleopatraapplication.cpp
  main.cpp )

if ( KLEO_MODEL_TEST )
 add_definitions( -DKLEO_MODEL_TEST )
 set( _kleopatra_SRCS ${_kleopatra_SRCS} models/modeltest.cpp )
endif ( KLEO_MODEL_TEST )

kde4_add_ui_files( _kleopatra_SRCS
  dialogs/certificationoptionswidget.ui
  dialogs/expirydialog.ui
  dialogs/lookupcertificatesdialog.ui
  dialogs/ownertrustdialog.ui
  dialogs/selectchecklevelwidget.ui
  dialogs/selftestdialog.ui
  dialogs/exportsecretkeydialog.ui
  dialogs/adduseriddialog.ui
  dialogs/certificatedetailsdialog.ui
  newcertificatewizard/listwidget.ui
  newcertificatewizard/chooseprotocolpage.ui
  newcertificatewizard/enterdetailspage.ui
  newcertificatewizard/overviewpage.ui
  newcertificatewizard/keycreationpage.ui
  newcertificatewizard/resultpage.ui
  newcertificatewizard/advancedsettingsdialog.ui
  )
kde4_add_kcfg_files( _kleopatra_SRCS
  kcfg/tooltippreferences.kcfgc
  kcfg/emailoperationspreferences.kcfgc
  kcfg/smimevalidationpreferences.kcfgc
  )


if (ONLY_KLEO)
  add_definitions( -DONLY_KLEO -DKDEPIM_EXPORT= )
  set( _kleopatra_libkdepim_SRCS
    ../libkdepim/progressmanager.cpp
    ../libkdepim/progressdialog.cpp
    ../libkdepim/overlaywidget.cpp
    ../libkdepim/ssllabel.cpp
    ../libkdepim/statusbarprogresswidget.cpp
    )
  set( _kleopatra_libkdepim_LIBS )
else(ONLY_KLEO)
  set( _kleopatra_libkdepim_SRCS )
  set( _kleopatra_libkdepim_LIBS kdepim )
endif(ONLY_KLEO)

set( _kleopatra_mainwindow_SRCS
     mainwindow.cpp 
)

add_definitions ( -DKDE_DEFAULT_DEBUG_AREA=5151 )


kde4_add_app_icon(_kleopatra_mainwindow_SRCS "ox*-app-kleopatra.png")
kde4_add_executable(kleopatra_bin ${_kleopatra_SRCS} ${_kleopatra_mainwindow_SRCS} ${_kleopatra_uiserver_SRCS} ${_kleopatra_libkdepim_SRCS} )
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

set( _kleopatra_extra_libs ${QT_QT3SUPPORT_LIBRARY} )

if ( NOT KDE4_KCMUTILS_LIBS )
  set( KDE4_KCMUTILS_LIBS ${KDE4_KUTILS_LIBS} )
endif ( NOT KDE4_KCMUTILS_LIBS )

target_link_libraries(kleopatra_bin ${_kleopatra_extra_libs} kleo ${_kleopatra_libkdepim_LIBS} ${QGPGME_LIBRARIES} ${KDE4_KMIME_LIBRARY} ${KDE4_KCMUTILS_LIBS} ${_kleopatra_uiserver_extra_libs} )
if ( USABLE_ASSUAN_FOUND )
  target_link_libraries(kleopatra_bin ${QT_QTNETWORK_LIBRARY}) 
ENDIF ( USABLE_ASSUAN_FOUND )

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kleopatra_signencryptfiles.desktop
               kleopatra_signencryptfolders.desktop
               kleopatra_decryptverifyfiles.desktop
               kleopatra_decryptverifyfolders.desktop
                                         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kleopatra.rc              DESTINATION ${DATA_INSTALL_DIR}/kleopatra)

kde4_install_icons( ${ICON_INSTALL_DIR} )

