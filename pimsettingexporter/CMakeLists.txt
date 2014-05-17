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
)

kde_enable_exceptions()

add_executable(pimsettingexporter ${pimsettingexporter_SRCS})

target_link_libraries(pimsettingexporter
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::PimUtils
  KF5::MailTransport
  KF5::AlarmCalendar
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

########### install files ###############

add_subdirectory(tests)
