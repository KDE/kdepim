set( sieveeditor_kcfg_SRCS settings/sieveeditorglobalconfig.kcfgc )

kconfig_add_kcfg_files(sieveeditor_kcfg_SRCS ${sieveeditor_kcfg_SRCS})


set(sieveeditor_SRCS
    ${sieveeditor_kcfg_SRCS}
    main.cpp
    sieveeditormainwindow.cpp
    serversievesettingsdialog.cpp
    serversievesettings.cpp
    sieveeditormainwidget.cpp
    sieveeditorscriptwidget.cpp
    sieveeditorconfiguredialog.cpp
    sieveeditorscriptmanagerwidget.cpp
    sieveeditorutil.cpp
    serversievelistwidget.cpp
    sieveeditorconfigureserverwidget.cpp
    sieveeditormanagesievewidget.cpp
    sieveeditorpagewidget.cpp
    sieveserversettings.cpp
    sieveeditortabwidget.cpp
    sieveeditorcentralwidget.cpp
    sieveeditorconfigureserverpage.cpp
    sieveeditorbookmarks.cpp
    sieveeditoremptytabwidgetlabel.cpp
    )

ki18n_wrap_ui(sieveeditor_SRCS
    ui/serversievesettings.ui
    ui/sieveeditorconfigureserverwidget.ui
    )

qt5_add_resources(sieveeditor_SRCS sieveeditor.qrc)

ecm_qt_declare_logging_category(sieveeditor_SRCS HEADER sieveeditor_debug.h IDENTIFIER SIEVEEDITOR_LOG CATEGORY_NAME log_sieveeditor)

add_executable(sieveeditor ${sieveeditor_SRCS} )
target_link_libraries(sieveeditor
    KF5::MailTransport
    KF5::Bookmarks
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::XmlGui
    KF5::PimCommon
    KF5::KSieveUi
    KF5::KManageSieve
    )

install(TARGETS sieveeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.sieveeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})

add_subdirectory(kconf_update)

install(FILES settings/sieveeditorglobalconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

