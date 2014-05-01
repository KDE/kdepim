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

set(pimsettingexporter_SRCS
  main.cpp
  widgets/selectiontypetreewidget.cpp
  abstractimportexportjob.cpp
  pimsettingexporterwindow.cpp
  widgets/logwidget.cpp
  mail/exportmailjob.cpp
  mail/importmailjob.cpp
  pimsettingexporterkernel.cpp
  utils.cpp
  dialog/selectiontypedialog.cpp
  akonadidatabase.cpp
  calendar/exportcalendarjob.cpp
  calendar/importcalendarjob.cpp
  addressbook/exportaddressbookjob.cpp
  addressbook/importaddressbookjob.cpp
  alarm/exportalarmjob.cpp
  alarm/importalarmjob.cpp
  jot/exportjotjob.cpp
  jot/importjotjob.cpp
  notes/exportnotesjob.cpp
  notes/importnotesjob.cpp
  akregator/exportakregatorjob.cpp
  akregator/importakregatorjob.cpp
  blogilo/exportblogilojob.cpp
  blogilo/importblogilojob.cpp
  archivestorage.cpp
  knode/exportknodejob.cpp
  knode/importknodejob.cpp
  dialog/backupfilestructureinfodialog.cpp
  dialog/showarchivestructuredialog.cpp
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
  ${AKONADI_COMMON_LIBRARIES}
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
