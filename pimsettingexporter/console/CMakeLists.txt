
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

add_executable(pimsettingexporterconsole ${pimsettingexporterconsole_SRCS})

target_link_libraries(pimsettingexporterconsole
  
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  
  KF5::MailTransport
  ${AKONADI_COMMON_LIBRARIES}
  KF5::AlarmCalendar
  Qt5::Xml
  mailimporter
  mailcommon
  pimcommon
  kdepim
  messagecomposer
  pimsettingexporterprivate
)

#Install when finished
#install(TARGETS pimsettingexporterconsole ${INSTALL_TARGETS_DEFAULT_ARGS} )

