
include_directories( 
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/core/ 
)


set(pimsettingexporter_widget_SRCS
  widgets/selectiontypetreewidget.cpp
  widgets/logwidget.cpp
)

set(pimsettingexporter_dialog_SRCS
  dialog/selectiontypedialog.cpp
  dialog/backupfilestructureinfodialog.cpp
  dialog/showarchivestructuredialog.cpp
  dialog/synchronizeresourcedialog.cpp
)


set(pimsettingexporter_SRCS
    ${pimsettingexporter_widget_SRCS}
    ${pimsettingexporter_dialog_SRCS}
    pimsettingsbackuprestoreui.cpp
    importexportprogressindicatorgui.cpp
    pimsettingexporterwindow.cpp
    pimsettingcommandlineoption.cpp
    fullsynchronizeresourcesjob.cpp
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
)

add_subdirectory(kconf_update)

install(TARGETS pimsettingexporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(PROGRAMS org.kde.pimsettingexporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

