

set( storageservicemanager_kcfg_SRCS settings/storageservicemanagerglobalconfig.kcfgc )

kconfig_add_kcfg_files(storageservicemanager_kcfg_SRCS ${storageservicemanager_kcfg_SRCS})

set(storageservicemanager_SRCS
    ${storageservicemanager_kcfg_SRCS}
    main.cpp
    storageservicemanagermainwindow.cpp
    storageservicepage.cpp
    storageservicetabwidget.cpp
    storageserviceconfiguredialog.cpp
    storageservicemanagersettingsjob.cpp
    storageservicetreewidget.cpp
    storageservicewarning.cpp
    storageservicelogdialog.cpp
    storageserviceaccountinfodialog.cpp
    storageservicenavigationbuttons.cpp
    storageservicemanagerutil.cpp
    storageservicemanagermainwidget.cpp
    storageserviceconfigurestoragewidget.cpp
    )
qt5_add_resources(storageservicemanager_SRCS storageservicemanager.qrc)

ecm_qt_declare_logging_category(storageservicemanager_SRCS HEADER storageservicemanager_debug.h IDENTIFIER STORAGESERVICEMANAGER_LOG CATEGORY_NAME log_storageservicemanager)

add_executable(storageservicemanager ${storageservicemanager_SRCS} )
target_link_libraries(storageservicemanager 
    KF5::NotifyConfig
    KF5::PimCommon
    KF5::Libkdepim
    KF5::DBusAddons
    KF5::XmlGui
    KF5::Notifications
    KF5::PimTextEdit
    KF5::I18n
    )

install(TARGETS storageservicemanager ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.storageservicemanager.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES data/storageservicemanager.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})

install(FILES settings/storageservicemanagerglobalconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

add_subdirectory(kconf_update)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
