project(accountwizard)

# Qt5 package
find_package(Qt5UiTools ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

find_package(KF5Kross ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)

set( SHARED_MIME_INFO_MINIMUM_VERSION "0.40" )
find_package( SharedMimeInfo REQUIRED )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

# allow to disable GHNS support on resource-constrained systems
option( ACCOUNTWIZARD_NO_GHNS "Disable GHNS support in the account wizard" FALSE )

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )

if ( ACCOUNTWIZARD_NO_GHNS )
  add_definitions( -DACCOUNTWIZARD_NO_GHNS )
endif ()


set(accountwizard_srcs
  dialog.cpp
  typepage.cpp
  loadpage.cpp
  global.cpp
  page.cpp
  dynamicpage.cpp
  setupmanager.cpp
  setuppage.cpp
  resource.cpp
  setupobject.cpp
  transport.cpp
  configfile.cpp
  ldap.cpp
  identity.cpp
  setupispdb.cpp
  setupautoconfigkolabmail.cpp
  setupautoconfigkolabldap.cpp
  setupautoconfigkolabfreebusy.cpp
  servertest.cpp
  personaldatapage.cpp
  ispdb/ispdb.cpp
  ispdb/autoconfigkolabmail.cpp
  ispdb/autoconfigkolabldap.cpp
  ispdb/autoconfigkolabfreebusy.cpp
)

ki18n_wrap_ui(accountwizard_srcs
  ui/typepage.ui
  ui/loadpage.ui
  ui/setuppage.ui
  ui/personaldatapage.ui
)
ecm_qt_declare_logging_category(accountwizard_srcs HEADER accountwizard_debug.h IDENTIFIER ACCOUNTWIZARD_LOG CATEGORY_NAME log_accountwizard)

set(accountwizard_libs
  KF5::AkonadiCore
  KF5::MailTransport
  KF5::KCMUtils
  KF5::KrossCore
  KF5::KIOCore
  KF5::Mime
  KF5::IdentityManagement
  KF5::WidgetsAddons
  KF5::NewStuff
  Qt5::UiTools
  KF5::Libkdepim
  KF5::ItemViews
  KF5::I18n
  KF5::Ldap
  KF5::AkonadiWidgets
)

if ( NOT ACCOUNTWIZARD_NO_GHNS )
  set(accountwizard_srcs ${accountwizard_srcs} providerpage.cpp)
  ki18n_wrap_ui(accountwizard_srcs ui/providerpage.ui)
  set(accountwizard_libs ${accountwizard_libs} KF5::NewStuff)
endif ()

add_executable(accountwizard ${accountwizard_srcs} main.cpp)
target_link_libraries(accountwizard ${accountwizard_libs} KF5::DBusAddons)

add_library(accountwizard_plugin MODULE ${accountwizard_srcs} inprocess-main.cpp)
target_link_libraries(accountwizard_plugin ${accountwizard_libs})

if ( NOT ACCOUNTWIZARD_NO_GHNS )
  install(FILES accountwizard.knsrc DESTINATION ${KDE_INSTALL_CONFDIR})
endif ()
install(TARGETS accountwizard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS accountwizard_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(PROGRAMS org.kde.accountwizard.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES accountwizard-mime.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})

add_subdirectory(wizards)
add_subdirectory(ispdb)
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
