project(pimsettingexporter)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(${AKONADI_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${CMAKE_SOURCE_DIR}/messageviewer
                    ${CMAKE_SOURCE_DIR}/pimcommon
                    ${CMAKE_SOURCE_DIR}/messagecomposer/
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/ 
)


########### next target ###############

set(pimsettingexporter_calendar_SRCS
  calendar/exportcalendarjob.cpp
  calendar/importcalendarjob.cpp
)

set(pimsettingexporter_addressbook_SRCS
  addressbook/exportaddressbookjob.cpp
  addressbook/importaddressbookjob.cpp
)

set(pimsettingexporter_mail_SRCS
  mail/exportmailjob.cpp
  mail/importmailjob.cpp
)

set(pimsettingexporter_knode_SRCS
  knode/exportknodejob.cpp
  knode/importknodejob.cpp
)

set(pimsettingexporter_alarm_SRCS
  alarm/exportalarmjob.cpp
  alarm/importalarmjob.cpp
)

set(pimsettingexporter_notes_SRCS
  notes/exportnotesjob.cpp
  notes/importnotesjob.cpp
)

set(pimsettingexporter_dialog_SRCS
  dialog/selectiontypedialog.cpp
  dialog/backupfilestructureinfodialog.cpp
  dialog/showarchivestructuredialog.cpp
)

set(pimsettingexporter_widget_SRCS
  widgets/selectiontypetreewidget.cpp
  widgets/logwidget.cpp
)

set(pimsettingexporter_xml_SRCS
  xml/templateselection.cpp
)

set(pimsettingexporter_jot_SRCS
  jot/exportjotjob.cpp
  jot/importjotjob.cpp
)

set(pimsettingexporter_akregator_SRCS
  akregator/exportakregatorjob.cpp
  akregator/importakregatorjob.cpp
)

set(pimsettingexporter_blogilo_SRCS
  blogilo/exportblogilojob.cpp
  blogilo/importblogilojob.cpp
)

set(pimsettingexporter_SRCS
  ${pimsettingexporter_widget_SRCS}
  ${pimsettingexporter_xml_SRCS}
  ${pimsettingexporter_notes_SRCS}
  ${pimsettingexporter_alarm_SRCS}
  ${pimsettingexporter_knode_SRCS}
  ${pimsettingexporter_mail_SRCS}
  ${pimsettingexporter_calendar_SRCS}
  ${pimsettingexporter_addressbook_SRCS}
  ${pimsettingexporter_dialog_SRCS}
  ${pimsettingexporter_jot_SRCS}
  ${pimsettingexporter_akregator_SRCS}
  ${pimsettingexporter_blogilo_SRCS}
  main.cpp
  abstractimportexportjob.cpp
  pimsettingexporterwindow.cpp
  pimsettingexporterkernel.cpp
  utils.cpp
  akonadidatabase.cpp
  archivestorage.cpp
  synchronizeresourcejob.cpp
  pimsettingsbackuprestore.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

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
)

install(TARGETS pimsettingexporter ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    pimsettingexporter.rc
    DESTINATION ${DATA_INSTALL_DIR}/pimsettingexporter
)

install(FILES backup-structure.txt DESTINATION ${DATA_INSTALL_DIR}/pimsettingexporter)

install(PROGRAMS pimsettingexporter.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})


########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )
add_subdirectory(tests)
