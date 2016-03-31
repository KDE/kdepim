
include_directories( 
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/core/ 
)


set( pimsettingexporter_kcfg_SRCS settings/pimsettingexporterglobalconfig.kcfgc )

kconfig_add_kcfg_files(pimsettingexporter_kcfg_SRCS ${pimsettingexporter_kcfg_SRCS})


set(pimsettingexporter_widget_SRCS
  widgets/selectiontypetreewidget.cpp
  widgets/logwidget.cpp
  widgets/pimsettingexporterconfigurewidget.cpp
)

set(pimsettingexporter_dialog_SRCS
  dialog/selectiontypedialog.cpp
  dialog/backupfilestructureinfodialog.cpp
  dialog/showarchivestructuredialog.cpp
  dialog/synchronizeresourcedialog.cpp
  dialog/pimsettingexporterconfiguredialog.cpp
)

set(pimsettingexporter_job_SRCS
   job/fullsynchronizeresourcesjob.cpp
)

set(pimsettingexporter_trayicon_SRCS
   trayicon/pimsettingstrayicon.cpp
)

set(pimsettingexporter_SRCS
    ${pimsettingexporter_kcfg_SRCS}
    ${pimsettingexporter_widget_SRCS}
    ${pimsettingexporter_dialog_SRCS}
    ${pimsettingexporter_job_SRCS}
    ${pimsettingexporter_trayicon_SRCS}
    pimsettingsbackuprestoreui.cpp
    importexportprogressindicatorgui.cpp
    pimsettingexporterwindow.cpp
    pimsettingcommandlineoption.cpp
    main.cpp)

qt5_add_resources(pimsettingexporter_SRCS pimsettingexporter.qrc)

ecm_qt_declare_logging_category(pimsettingexporter_SRCS HEADER pimsettingexportgui_debug.h IDENTIFIER PIMSETTINGEXPORTERGUI_LOG CATEGORY_NAME log_pimsettingexportergui)

add_executable(pimsettingexporter ${pimsettingexporter_SRCS})

target_link_libraries(pimsettingexporter
 PRIVATE
  KF5::XmlGui
  KF5::DBusAddons
  KF5::MailCommon
  KF5::PimCommon
  KF5::Libkdepim
  pimsettingexporterprivate
  KF5::Archive
  KF5::ItemViews
  KF5::KIOFileWidgets
  KF5::Notifications
)

add_subdirectory(kconf_update)

install(TARGETS pimsettingexporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(PROGRAMS org.kde.pimsettingexporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES settings/pimsettingexporterglobalconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

