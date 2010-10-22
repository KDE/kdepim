include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/templateparser
  ${CMAKE_BINARY_DIR}/templateparser
  ${CMAKE_CURRENT_BINARY_DIR}/../ontologies
 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libmailcommon_SRCS
  entitycollectionorderproxymodel.cpp
  expirejob.cpp
  expirypropertiesdialog.cpp
  foldercollection.cpp
  foldercollectionmonitor.cpp
  folderjob.cpp
  folderrequester.cpp
  folderselectiondialog.cpp
  foldertreeview.cpp
  foldertreewidget.cpp
  imapaclattribute.cpp
  jobscheduler.cpp
  mailfilter.cpp
  mailkernel.cpp
  mailutil.cpp
  mdnadvicedialog.cpp
  messageproperty.cpp
  readablecollectionproxymodel.cpp
  regexplineedit.cpp
)

if ( NOT WINCE )
set(libmailcommon_SRCS ${libmailcommon_SRCS}
  filteraction.cpp
  filterimporterexporter.cpp
  filterlog.cpp
  filtermanager.cpp
  searchpattern.cpp
  soundtestwidget.cpp
)
endif ( NOT WINCE )

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile) 
  set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(libmailcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro(add_resource_iface)

add_resource_iface( imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings )

qt4_add_dbus_interfaces(libmailcommon_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

kde4_add_library(mailcommon ${LIBRARY_TYPE} ${libmailcommon_SRCS})

target_link_libraries(mailcommon messageviewer messagecore messagecomposer
  templateparser
  kdepim
  akonadi_next
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KIMAP_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
)

if ( NOT WINCE )
target_link_libraries(mailcommon 
  ${NEPOMUK_QUERY_LIBRARIES}
  ${SOPRANO_LIBRARIES}
  niefast_apps
  incidenceeditorsng
)
endif (NOT WINCE)

set_target_properties( mailcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS mailcommon ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer.h attachmentstrategy.h headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)
