project(importwizard)

set(importwizard_SRCS
importwizardkernel.cpp
checkprogrampage.cpp
importaddressbookpage.cpp
importfilterpage.cpp
importmailpage.cpp
importsettingpage.cpp
importwizard.cpp
main.cpp
pimimportabstract.cpp
selectcomponentpage.cpp
thunderbird/thunderbirdimportdata.cpp
sylpheed/sylpheedimportdata.cpp
evolutionv3/evolutionv3importdata.cpp
evolutionv2/evolutionv2importdata.cpp
evolutionv1/evolutionv1importdata.cpp
importfilterinfogui.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/mailimporter ${CMAKE_SOURCE_DIR}/mailcommon )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(importwizard_SRCS 
                  ui/checkprogrampage.ui
                  ui/importaddressbookpage.ui
                  ui/importfilterpage.ui
                  ui/importmailpage.ui
                  ui/importsettingpage.ui
                  ui/selectcomponentpage.ui )

kde4_add_executable(importwizard ${importwizard_SRCS})

target_link_libraries(importwizard
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  mailimporter
  mailcommon
  kdepim
  messagecomposer
)

install(TARGETS importwizard  ${INSTALL_TARGETS_DEFAULT_ARGS} )

