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
  filtericedove.cpp
  filterbalsa.cpp 
  selectthunderbirdprofilewidget.cpp 
  othermailerutil.cpp
)

ki18n_wrap_ui(libmailimporter_SRCS ui/importmailswidget.ui ui/selectthunderbirdprofilewidget.ui)

ecm_qt_declare_logging_category(libmailimporter_SRCS HEADER mailimporter_debug.h IDENTIFIER MAILIMPORTER_LOG CATEGORY_NAME log_mailimporter)

add_library(mailimporter ${libmailimporter_SRCS})
generate_export_header(mailimporter BASE_NAME mailimporter)
target_link_libraries(mailimporter
PUBLIC
  KF5::Mime
PRIVATE
  KF5::AkonadiCore
  KF5::AkonadiMime
  kdepim
  KF5::Archive
  KF5::I18n
  KF5::ConfigCore
  Qt5::Xml
)

target_include_directories(mailimporter PUBLIC "$<BUILD_INTERFACE:${mailimporter_SOURCE_DIR};${mailimporter_BINARY_DIR}>")

set_target_properties(mailimporter PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS mailimporter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
