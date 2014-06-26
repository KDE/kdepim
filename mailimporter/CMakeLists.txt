project(mailimporter)



add_definitions(-DTRANSLATION_DOMAIN=\"libmailimporter\")

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
  filter_mailmangzip.cpp
  filterbalsa.cpp 
  selectthunderbirdprofilewidget.cpp 
  othermailerutil.cpp
)

include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)

ki18n_wrap_ui(libmailimporter_SRCS ui/importmailswidget.ui ui/selectthunderbirdprofilewidget.ui)

add_library(mailimporter ${LIBRARY_TYPE} ${libmailimporter_SRCS})
target_link_libraries(mailimporter
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::PimUtils
  kdepim
  messagecore
)

set_target_properties(mailimporter PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS mailimporter ${INSTALL_TARGETS_DEFAULT_ARGS})

