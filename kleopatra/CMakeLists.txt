kde4_no_enable_final(kleopatra)
project(kleopatra)

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


option(KLEO_STATIC_KCMODULES "Build Kleopatra's KCMs into Kleopatra; don't install plugins" WINCE)

include(MacroOptionalAddSubdirectory)

set(kleopatra_version 2.1.1)
set(kleopatra_release FALSE)

if(NOT kleopatra_release)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --oneline --format=%h ${CMAKE_CURRENT_SOURCE_DIR}
                    WORKING_DIRECTORY ${kdepim_SOURCE_DIR}/kleopatra
                    OUTPUT_VARIABLE Kleopatra_WC_REVISION)
    string(REGEX REPLACE "\n" "" Kleopatra_WC_REVISION "${Kleopatra_WC_REVISION}")

    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --oneline --format=%ci ${CMAKE_CURRENT_SOURCE_DIR}
                    WORKING_DIRECTORY ${kdepim_SOURCE_DIR}/kleopatra
                    OUTPUT_VARIABLE Kleopatra_WC_LAST_CHANGED_DATE)
    string(REGEX REPLACE " [-0-9:+ ]*\n" "" Kleopatra_WC_LAST_CHANGED_DATE "${Kleopatra_WC_LAST_CHANGED_DATE}")

    set(kleopatra_version "${kleopatra_version}-git${Kleopatra_WC_REVISION} (${Kleopatra_WC_LAST_CHANGED_DATE})")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version-kleopatra.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kleopatra.h)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_SOURCE_DIR}/mobile/lib
  ${Boost_INCLUDE_DIR}
  ${QGPGME_INCLUDES}
  ${GPGME_INCLUDES}
)

if(USABLE_ASSUAN_FOUND)
  if(ASSUAN2_FOUND)
    include_directories(${ASSUAN2_INCLUDES})
  else()
    include_directories(${ASSUAN_INCLUDES})
  endif()
endif()

add_definitions(-D_ASSUAN_ONLY_GPG_ERRORS)

if(NOT WINCE)
  add_definitions(-DQT_STL)
  remove_definitions(-DQT_NO_STL)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
if(GCC_IS_NEWER_THAN_4_3)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces -Wno-parentheses -Wno-ignored-qualifiers")
  endif()
endif()

add_subdirectory(pics)
add_subdirectory(conf)

if(USABLE_ASSUAN_FOUND AND BUILD_libkleopatraclient)
  # can't use macro_optional_add_subdirectory here, since it's split
  # off into ConfigureChecks.cmake..
  add_subdirectory(libkleopatraclient)
endif()

if(NOT WINCE)
  add_subdirectory(kgpgconf)
  add_subdirectory(kwatchgnupg)
  add_subdirectory(tests)
endif()

if(WIN32)
  set(_kleopatra_extra_uiserver_SRCS uiserver/uiserver_win.cpp)
  set(_kleopatra_extra_SRCS utils/gnupg-registry.c)
  if(NOT WINCE)
    set(_kleopatra_extra_SRCS ${_kleopatra_extra_SRCS} selftest/registrycheck.cpp)
  endif()
else()
  set(_kleopatra_extra_uiserver_SRCS uiserver/uiserver_unix.cpp)
  set(_kleopatra_extra_SRCS)
endif()

if(USABLE_ASSUAN_FOUND)
  add_definitions(-DKLEOPATRACLIENT_NAMESPACE=KleopatraClientCopy -DKLEOPATRACLIENTCORE_EXPORT=)
  set(_kleopatra_uiserver_SRCS
    uiserver/sessiondata.cpp
    uiserver/uiserver.cpp
    ${_kleopatra_extra_uiserver_SRCS}
    uiserver/assuanserverconnection.cpp
    uiserver/echocommand.cpp
    uiserver/decryptverifycommandemailbase.cpp
    uiserver/decryptverifycommandfilesbase.cpp
    uiserver/signcommand.cpp
    uiserver/signencryptfilescommand.cpp
    uiserver/prepencryptcommand.cpp
    uiserver/prepsigncommand.cpp
    uiserver/encryptcommand.cpp
    uiserver/selectcertificatecommand.cpp
    uiserver/importfilescommand.cpp
    uiserver/createchecksumscommand.cpp
    uiserver/verifychecksumscommand.cpp

    libkleopatraclient/core/command.cpp
    selftest/uiservercheck.cpp
  )

  if(ASSUAN2_FOUND)
    set(_kleopatra_uiserver_extra_libs ${ASSUAN2_LIBRARIES})
  else()
    if(WIN32)
      set(_kleopatra_uiserver_extra_libs ${ASSUAN_VANILLA_LIBRARIES})
    else()
      set(_kleopatra_uiserver_extra_libs ${ASSUAN_PTHREAD_LIBRARIES})
    endif()
  endif()

  if(HAVE_GPG_ERR_SOURCE_KLEO)
    add_definitions(-DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_KLEO)
  else()
    add_definitions(-DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_1)
  endif()

else()
  set(_kleopatra_uiserver_SRCS)
endif()

kde4_add_ui_files(_kleopatra_uiserver_SRCS crypto/gui/signingcertificateselectionwidget.ui)

set(_kleopatra_SRCS
  utils/gnupg-helper.cpp
  utils/filesystemwatcher.cpp
  utils/filedialog.cpp
  utils/kdpipeiodevice.cpp
  utils/kdlogtextwidget.cpp
  utils/kdsignalblocker.cpp
  utils/headerview.cpp
  utils/scrollarea.cpp
  utils/dragqueen.cpp
  utils/multivalidator.cpp
  utils/progressmanager.cpp
  utils/systemtrayicon.cpp

  utils/hex.cpp
  utils/path-helper.cpp
  utils/input.cpp
  utils/output.cpp
  utils/formatting.cpp
  utils/validation.cpp
  utils/wsastarter.cpp
  utils/classify.cpp
  utils/iodevicelogger.cpp
  utils/log.cpp
  utils/action_data.cpp
  utils/types.cpp
  utils/archivedefinition.cpp
  utils/auditlog.cpp

  selftest/selftest.cpp
  selftest/enginecheck.cpp
  selftest/gpgconfcheck.cpp
  selftest/gpgagentcheck.cpp
  selftest/libkleopatrarccheck.cpp

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
  dialogs/deletecertificatesdialog.cpp
  dialogs/setinitialpindialog.cpp

  crypto/controller.cpp
  crypto/certificateresolver.cpp
  crypto/sender.cpp
  crypto/recipient.cpp
  crypto/task.cpp
  crypto/taskcollection.cpp
  crypto/decryptverifytask.cpp
  crypto/decryptverifyemailcontroller.cpp
  crypto/decryptverifyfilescontroller.cpp
  crypto/encryptemailtask.cpp
  crypto/encryptemailcontroller.cpp
  crypto/newsignencryptemailcontroller.cpp
  crypto/signencryptfilestask.cpp
  crypto/signencryptfilescontroller.cpp
  crypto/signemailtask.cpp
  crypto/signemailcontroller.cpp
  crypto/createchecksumscontroller.cpp
  crypto/verifychecksumscontroller.cpp

  crypto/gui/wizard.cpp
  crypto/gui/wizardpage.cpp

  crypto/gui/signingcertificateselectionwidget.cpp
  crypto/gui/signingcertificateselectiondialog.cpp

  crypto/gui/resultitemwidget.cpp
  crypto/gui/resultlistwidget.cpp
  crypto/gui/resultpage.cpp

  crypto/gui/newresultpage.cpp
  crypto/gui/newsignencryptfileswizard.cpp

  crypto/gui/signencryptemailconflictdialog.cpp

  crypto/gui/decryptverifyoperationwidget.cpp
  crypto/gui/decryptverifyfileswizard.cpp

  crypto/gui/objectspage.cpp
  crypto/gui/resolverecipientspage.cpp
  crypto/gui/signerresolvepage.cpp
  crypto/gui/encryptemailwizard.cpp
  crypto/gui/signemailwizard.cpp
  crypto/gui/signencryptfileswizard.cpp
  crypto/gui/signencryptwizard.cpp

  crypto/gui/verifychecksumsdialog.cpp

  commands/command.cpp
  commands/gnupgprocesscommand.cpp
  commands/detailscommand.cpp
  commands/exportcertificatecommand.cpp
  commands/importcertificatescommand.cpp
  commands/importcertificatefromfilecommand.cpp
  commands/importcertificatefromclipboardcommand.cpp
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
  commands/changeroottrustcommand.cpp
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
  systrayicon.cpp
  kleopatraapplication.cpp
  main.cpp
)

if(NOT WINCE)
  set(_kleopatra_SRCS
    ${_kleopatra_SRCS}
    smartcard/readerstatus.cpp

    commands/setinitialpincommand.cpp
    commands/learncardkeyscommand.cpp
    commands/checksumcreatefilescommand.cpp
    commands/checksumverifyfilescommand.cpp
  )
endif()

if(WINCE)
  set(_kleopatra_SRCS
    ${_kleopatra_SRCS}
    utils/qwizard.cpp
  )
endif()

if(KLEO_STATIC_KCMODULES)
  add_definitions(-DKLEO_STATIC_KCMODULES)
  set(_kleopatra_extra_libs kcm_kleopatra)
else()
  set(_kleopatra_extra_libs)
endif()

if(KLEO_MODEL_TEST)
  add_definitions(-DKLEO_MODEL_TEST)
  set(_kleopatra_SRCS ${_kleopatra_SRCS} models/modeltest.cpp)
endif()

kde4_add_ui_files(_kleopatra_SRCS
  dialogs/certificationoptionswidget.ui
  dialogs/expirydialog.ui
  dialogs/lookupcertificatesdialog.ui
  dialogs/ownertrustdialog.ui
  dialogs/selectchecklevelwidget.ui
  dialogs/selftestdialog.ui
  dialogs/exportsecretkeydialog.ui
  dialogs/adduseriddialog.ui
  dialogs/certificatedetailsdialog.ui
  dialogs/setinitialpindialog.ui
  newcertificatewizard/listwidget.ui
  newcertificatewizard/chooseprotocolpage.ui
  newcertificatewizard/enterdetailspage.ui
  newcertificatewizard/overviewpage.ui
  newcertificatewizard/keycreationpage.ui
  newcertificatewizard/resultpage.ui
  newcertificatewizard/advancedsettingsdialog.ui
)

kde4_add_kcfg_files(_kleopatra_SRCS
  kcfg/tooltippreferences.kcfgc
  kcfg/emailoperationspreferences.kcfgc
  kcfg/fileoperationspreferences.kcfgc
  kcfg/smimevalidationpreferences.kcfgc
)

if(KDEPIM_ONLY_KLEO)
  add_definitions(-DKDEPIM_ONLY_KLEO -DKDEPIM_EXPORT=)
  set(_kleopatra_libkdepim_SRCS
    ../libkdepim/progressmanager.cpp
    ../libkdepim/progressdialog.cpp
    ../libkdepim/overlaywidget.cpp
    ../libkdepim/ssllabel.cpp
    ../libkdepim/statusbarprogresswidget.cpp
  )
  set(_kleopatra_libkdepim_LIBS)
else()
  set(_kleopatra_libkdepim_SRCS)
  set(_kleopatra_libkdepim_LIBS kdepim)
endif()

if(KDEPIM_MOBILE_UI)
  set(_kleopatra_uiserver_extra_libs ${_kleopatra_uiserver_extra_libs} kdepimmobileui)
  set(_kleopatra_mainwindow_SRCS mainwindow_mobile.cpp)
else()
  set(_kleopatra_mainwindow_SRCS mainwindow_desktop.cpp)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5151)

kde4_add_app_icon(_kleopatra_mainwindow_SRCS "hi*-app-kleopatra.png")

kde4_add_executable(kleopatra_bin ${_kleopatra_SRCS} ${_kleopatra_mainwindow_SRCS} ${_kleopatra_uiserver_SRCS} ${_kleopatra_libkdepim_SRCS})
set_target_properties(kleopatra_bin PROPERTIES OUTPUT_NAME kleopatra)

target_link_libraries(kleopatra_bin
  ${_kleopatra_extra_libs}
  kleo
  ${QGPGME_LIBRARIES}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${_kleopatra_uiserver_extra_libs}
  ${_kleopatra_libkdepim_LIBS}
)

if(KDEPIM_MOBILE_UI)
  target_link_libraries(kleopatra_bin ${QT_QTDECLARATIVE_LIBRARY})
endif()

if(USABLE_ASSUAN_FOUND)
  target_link_libraries(kleopatra_bin ${QT_QTNETWORK_LIBRARY})
endif()

install(TARGETS kleopatra_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
  PROGRAMS kleopatra.desktop kleopatra_import.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)
install(
  FILES kleopatra_signencryptfiles.desktop
        kleopatra_signencryptfolders.desktop
        kleopatra_decryptverifyfiles.desktop
        kleopatra_decryptverifyfolders.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

install(FILES kleopatra.rc DESTINATION ${DATA_INSTALL_DIR}/kleopatra)

if(KDEPIM_MOBILE_UI)
  install(
    FILES qml/kleopatra-mobile.qml qml/KleopatraActions.qml
    DESTINATION ${DATA_INSTALL_DIR}/kleopatra
  )
endif()

kde4_install_icons(${ICON_INSTALL_DIR})
