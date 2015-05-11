
include_directories(${AKONADI_INCLUDE_DIR} 
                    ${CMAKE_SOURCE_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/mailcommon
                    ${CMAKE_SOURCE_DIR}/messageviewer
                    ${CMAKE_SOURCE_DIR}/pimcommon
                    ${CMAKE_SOURCE_DIR}/messagecomposer/
                    ${CMAKE_SOURCE_DIR}/pimsettingexporter/ 
)


set(pimsettingexporterconsole_SRCS
    importexportprogressindicatorconsole.cpp
    main.cpp)

kde4_add_executable(pimsettingexporterconsole ${pimsettingexporterconsole_SRCS})

target_link_libraries(pimsettingexporterconsole
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

#Install when finished
#install(TARGETS pimsettingexporterconsole ${INSTALL_TARGETS_DEFAULT_ARGS} )

