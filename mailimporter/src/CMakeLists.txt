set(libmailimporter_SRCS
  importmailswidget.cpp
  filterinfogui.cpp
  filterinfo.cpp
  filters.cpp
  filteroe.cpp
  filterpmail.cpp
  filterplain.cpp
  filtermbox.cpp
  filterevolution.cpp
  filtermailapp.cpp
  filterevolution_v2.cpp
  filterevolution_v3.cpp
  filteropera.cpp
  filterthunderbird.cpp
  filterkmail_maildir.cpp
  filtersylpheed.cpp
  filterclawsmail.cpp
  filterthebat.cpp
  filterlnotes.cpp
  filterkmailarchive.cpp
  filtermailmangzip.cpp
  filtericedove.cpp
  filterbalsa.cpp 
  selectthunderbirdprofilewidget.cpp 
  othermailerutil.cpp
)

ki18n_wrap_ui(libmailimporter_SRCS ui/importmailswidget.ui ui/selectthunderbirdprofilewidget.ui)

ecm_qt_declare_logging_category(libmailimporter_SRCS HEADER mailimporter_debug.h IDENTIFIER MAILIMPORTER_LOG CATEGORY_NAME log_mailimporter)

add_library(KF5MailImporter ${libmailimporter_SRCS})

generate_export_header( KF5MailImporter BASE_NAME mailimporter)

add_library(KF5::MailImporter ALIAS KF5MailImporter)

target_include_directories(KF5MailImporter INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/MailImporter/;${KDE_INSTALL_INCLUDEDIR_KF5}/mailimporter>")
target_include_directories(KF5MailImporter PUBLIC "$<BUILD_INTERFACE:${MailImporter_SOURCE_DIR}/src;${MailImporter_BINARY_DIR}/src;>")


target_link_libraries(KF5MailImporter
PUBLIC
  KF5::Mime
PRIVATE
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Libkdepim
  KF5::Archive
  KF5::I18n
  KF5::ConfigCore
  Qt5::Xml
  Qt5::Widgets
)

set_target_properties(KF5MailImporter PROPERTIES
    VERSION ${MAILIMPORTER_VERSION_STRING}
    SOVERSION ${MAILIMPORTER_SOVERSION}
    EXPORT_NAME MailImporter
)

install(TARGETS
    KF5MailImporter
    EXPORT KF5MailImporterTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(MailImporter_CamelCase_HEADERS
  HEADER_NAMES
  FilterClawsMail
  FilterBalsa
  FilterSylpheed
  FilterTheBat
  FilterEvolution
  FilterInfoGui
  FilterThunderbird
  FilterEvolution_v2
  FilterEvolution_v3
  FilterMailApp
  FilterMailmanGzip
  FilterPlain
  FilterPMail
  FilterMBox
  FilterOpera
  FilterLNotes
  FilterOE
  FilterKMail_maildir
  FilterKMailArchive
  FilterIcedove
  OtherMailerUtil
  ImportMailsWidget
  MailImporter_Utils
  Filters
  FilterInfo
  REQUIRED_HEADERS MailImporter_HEADERS
  PREFIX MailImporter
)
ecm_generate_pri_file(BASE_NAME MailImporter
    LIB_NAME KF5MailImpoter
    DEPS "Mime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/MailImporter
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${MailImporter_SOURCE_DIR})
install(FILES
    ${MailImporter_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/MailImporter
    COMPONENT Devel
)

install(FILES
    ${MailImporter_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/mailimporter_export.h
    ${MailImporter_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/mailimporter
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()
