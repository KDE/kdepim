project(backupmail)

include_directories(${AKONADI_INCLUDE_DIR} 
                    ${SOPRANO_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${NEPOMUK_CORE_INCLUDE_DIR}
)


########### next target ###############

set(backupmail_SRCS
  main.cpp
  abstractimportexportjob.cpp
  backupmailwindow.cpp
  backupmailwidget.cpp
  exportmailjob.cpp
  importmailjob.cpp
  backupmailkernel.cpp
  backupmailutil.cpp
  selecttypewidget.cpp
  selectiontypedialog.cpp
  akonadidatabase.cpp
  exportcalendarjob.cpp
  importcalendarjob.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(backupmail_SRCS ui/selecttypewidget.ui )

kde4_add_executable(backupmail ${backupmail_SRCS})

target_link_libraries(backupmail
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${AKONADI_COMMON_LIBRARIES}
  mailimporter
  mailcommon
  kdepim
  messagecomposer
)

install(TARGETS backupmail ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    backupmail.rc
    DESTINATION ${DATA_INSTALL_DIR}/backupmail
)


########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )
