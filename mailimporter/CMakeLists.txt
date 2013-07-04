project(mailimporter)

set(libmailimporter_SRCS
  importmailswidget.cpp
  filterinfogui.cpp
  filterinfo.cpp
  filters.cpp
  filter_oe.cpp
  filter_pmail.cpp
  filter_plain.cpp
  filter_mbox.cpp
  filter_evolution.cpp
  filter_mailapp.cpp
  filter_evolution_v2.cpp
  filter_evolution_v3.cpp
  filter_opera.cpp
  filter_thunderbird.cpp
  filter_kmail_maildir.cpp
  filter_sylpheed.cpp
  filter_clawsmail.cpp
  filter_thebat.cpp
  filter_lnotes.cpp
  filter_kmail_archive.cpp
  filterbalsa.cpp 
  selectthunderbirdprofilewidget.cpp 
  othermailerutil.cpp
)

include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(libmailimporter_SRCS ui/importmailswidget.ui ui/selectthunderbirdprofilewidget.ui)

kde4_add_library(mailimporter ${LIBRARY_TYPE} ${libmailimporter_SRCS})
target_link_libraries(mailimporter
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  kdepim
  messagecore
)

set_target_properties(mailimporter PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS mailimporter ${INSTALL_TARGETS_DEFAULT_ARGS})

