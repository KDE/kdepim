project(pimsettingexporter)

include_directories(${AKONADI_INCLUDE_DIR} 
                    ${SOPRANO_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${NEPOMUK_CORE_INCLUDE_DIR}
)


########### next target ###############

set(pimsettingexporter_SRCS
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
  archivestorage.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(pimsettingexporter_SRCS ui/selecttypewidget.ui )

kde4_add_executable(pimsettingexporter ${pimsettingexporter_SRCS})

target_link_libraries(pimsettingexporter
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${AKONADI_COMMON_LIBRARIES}
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


########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )
