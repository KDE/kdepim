project(importwizard)

set(importwizard_SRCS
  main.cpp
  importwizard.cpp
  pimimportabstract.cpp
  thunderbirdimportdata.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/mailimporter ${CMAKE_SOURCE_DIR}/mailcommon )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_executable(importwizard ${importwizard_SRCS})

target_link_libraries(importwizard
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  mailimporter
  mailcommon
)

install(TARGETS importwizard  ${INSTALL_TARGETS_DEFAULT_ARGS} )

