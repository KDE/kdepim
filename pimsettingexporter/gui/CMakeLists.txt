
include_directories(${AKONADI_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${CMAKE_SOURCE_DIR}/messageviewer
                    ${CMAKE_SOURCE_DIR}/pimcommon
                    ${CMAKE_SOURCE_DIR}/messagecomposer/
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
    main.cpp)

kde4_add_executable(pimsettingexporter ${pimsettingexporter_SRCS})

target_link_libraries(pimsettingexporter
  ${KDE4_KFILE_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${AKONADI_COMMON_LIBRARIES}
  ${KDEPIMLIBS_KALARMCAL_LIBS}
  ${QT_QTXML_LIBRARY}
  mailimporter
  mailcommon
  pimcommon
  kdepim
  messagecomposer
  pimsettingexporterprivate
)


install(TARGETS pimsettingexporter ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    pimsettingexporter.rc
    DESTINATION ${DATA_INSTALL_DIR}/pimsettingexporter
)

install(PROGRAMS pimsettingexporter.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

