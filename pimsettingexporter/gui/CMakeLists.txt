
include_directories( 
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/ 
)


set(pimsettingexporter_widget_SRCS
  widgets/selectiontypetreewidget.cpp
  widgets/logwidget.cpp
)

set(pimsettingexporter_dialog_SRCS
  dialog/selectiontypedialog.cpp
  dialog/backupfilestructureinfodialog.cpp
  dialog/showarchivestructuredialog.cpp
)


set(pimsettingexporter_SRCS
    ${pimsettingexporter_widget_SRCS}
    ${pimsettingexporter_dialog_SRCS}
    pimsettingsbackuprestoreui.cpp
    importexportprogressindicatorgui.cpp
    pimsettingexporterwindow.cpp
    pimsettingcommandlineoption.cpp
    ../pimsettingexport_debug.cpp
    main.cpp)

add_executable(pimsettingexporter ${pimsettingexporter_SRCS})

target_link_libraries(pimsettingexporter
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::MailTransport
  KF5::AlarmCalendar
  KF5::DBusAddons
  mailimporter
  mailcommon
  pimcommon
  kdepim
  messagecomposer
  pimsettingexporterprivate
)

install(TARGETS pimsettingexporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    pimsettingexporter.rc
    DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/pimsettingexporter
)

install(PROGRAMS org.kde.pimsettingexporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})


