project(kleopatra)

cmake_minimum_required(VERSION 2.8.12)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kleopatra_SOURCE_DIR})
   # Standalone build. Find / include everything neccessary.
  set(KF5_VERSION "5.17.0")
  set(GPGMEPP_VERSION "5.1.40")
  set(KMIME_VERSION "5.1.40")
  set(LIBKLEO_VERSION "5.1.41")
  set(QT_REQUIRED_VERSION "5.4.0")

  find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

  include(ECMOptionalAddSubdirectory)
  include(ECMInstallIcons)
  include(ECMSetupVersion)
  include(ECMAddTests)
  include(ECMMarkNonGuiExecutable)
  include(GenerateExportHeader)
  include(ECMGenerateHeaders)
  include(CMakePackageConfigHelpers)
  include(FeatureSummary)
  include(CheckFunctionExists)
  include(ECMGeneratePriFile)

  include(KDEInstallDirs)
  include(KDECMakeSettings)
  include(KDECompilerSettings NO_POLICY_SCOPE)
  include(ECMAddAppIcon)
  include(ECMQtDeclareLoggingCategory)

# Find KF5 packages
  find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5Notifications ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5WindowSystem ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5TextWidgets ${KF5_VERSION} CONFIG REQUIRED)

# Kdepimlibs packages
  find_package(KF5Libkleo ${LIBKLEO_VERSION} CONFIG REQUIRED)
  find_package(KF5Mime ${KMIME_VERSION} CONFIG REQUIRED)
  find_package(KF5Gpgmepp ${GPGMEPP_VERSION} CONFIG REQUIRED)

  find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test Network)
endif() # Standalone build

find_package(Boost 1.34.0 REQUIRED)

find_path(Boost_TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
if(NOT Boost_TOPOLOGICAL_SORT_DIR)
  message(FATAL_ERROR "The Boost Topological_sort header was NOT found. Should be part of Boost graph module.")
endif()

set(kleopatra_version 2.2.0)
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

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include (ConfigureChecks.cmake)

find_package(Gpgme REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kleopatra.h)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/libkleopatraclient/core/
  ${Boost_INCLUDE_DIR}
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

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces -Wno-parentheses -Wno-ignored-qualifiers")
endif()

kde_enable_exceptions()

add_subdirectory(pics)
add_subdirectory(conf)

if(USABLE_ASSUAN_FOUND)
  # can't use ecm_optional_add_subdirectory here, since it's split
  # off into ConfigureChecks.cmake..
  add_subdirectory(libkleopatraclient)
endif()

add_subdirectory(kwatchgnupg)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(kconf_update)

if(WIN32)
  set(_kleopatra_extra_uiserver_SRCS uiserver/uiserver_win.cpp)
  set(_kleopatra_extra_SRCS utils/gnupg-registry.c selftest/registrycheck.cpp)
else()
  set(_kleopatra_extra_uiserver_SRCS uiserver/uiserver_unix.cpp)
  set(_kleopatra_extra_SRCS)
endif()

if(USABLE_ASSUAN_FOUND)
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

ki18n_wrap_ui(_kleopatra_uiserver_SRCS crypto/gui/signingcertificateselectionwidget.ui)

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
  utils/clipboardmenu.cpp

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
  commands/setinitialpincommand.cpp
  commands/learncardkeyscommand.cpp
  commands/checksumcreatefilescommand.cpp
  commands/checksumverifyfilescommand.cpp

  ${_kleopatra_uiserver_files}

  conf/configuredialog.cpp

  newcertificatewizard/listwidget.cpp
  newcertificatewizard/newcertificatewizard.cpp

  smartcard/readerstatus.cpp

  aboutdata.cpp
  systrayicon.cpp
  kleopatraapplication.cpp
  mainwindow.cpp
  main.cpp
)

ecm_qt_declare_logging_category(_kleopatra_SRCS HEADER kleopatra_debug.h IDENTIFIER KLEOPATRA_LOG CATEGORY_NAME log_kleopatra)

if(KLEO_MODEL_TEST)
  add_definitions(-DKLEO_MODEL_TEST)
  set(_kleopatra_SRCS ${_kleopatra_SRCS} models/modeltest.cpp)
endif()

ki18n_wrap_ui(_kleopatra_SRCS
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

kconfig_add_kcfg_files(_kleopatra_SRCS
  kcfg/tooltippreferences.kcfgc
  kcfg/emailoperationspreferences.kcfgc
  kcfg/fileoperationspreferences.kcfgc
  kcfg/smimevalidationpreferences.kcfgc
)


file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kleopatra.png")
ecm_add_app_icon(_kleopatra_SRCS ICONS ${ICONS_SRCS})

qt5_add_resources(_kleopatra_SRCS kleopatra.qrc)

add_executable(kleopatra_bin ${_kleopatra_SRCS} ${_kleopatra_uiserver_SRCS})
set_target_properties(kleopatra_bin PROPERTIES OUTPUT_NAME kleopatra)

target_link_libraries(kleopatra_bin
  ${_kleopatra_extra_libs}
  KF5::Libkleo
  KF5::Mime
  KF5::KCMUtils
  KF5::I18n
  KF5::XmlGui
  KF5::IconThemes
  KF5::WindowSystem
  KF5::DBusAddons
  Qt5::Network
  ${_kleopatra_uiserver_extra_libs}
  kleopatraclientcore
)

if(USABLE_ASSUAN_FOUND)
  target_link_libraries(kleopatra_bin )
endif()

install(TARGETS kleopatra_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
  PROGRAMS data/org.kde.kleopatra.desktop data/kleopatra_import.desktop
  DESTINATION ${KDE_INSTALL_APPDIR}
)
install(FILES data/kleopatra.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(
  FILES data/kleopatra_signencryptfiles.desktop
        data/kleopatra_signencryptfolders.desktop
        data/kleopatra_decryptverifyfiles.desktop
        data/kleopatra_decryptverifyfolders.desktop
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

add_subdirectory(icons)
