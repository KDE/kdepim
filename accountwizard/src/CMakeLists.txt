
set(accountwizard_srcs
    cryptopage.cpp
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
    ui/cryptopage.ui
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
    KF5::Crash
    KF5::Libkleo
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

