include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_SOURCE_DIR}/mailcommon/search/
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/messagecore/
  ${CMAKE_SOURCE_DIR}/messagecomposer/
  ${CMAKE_SOURCE_DIR}/templateparser
  ${CMAKE_BINARY_DIR}/templateparser
  ${CMAKE_BINARY_DIR}/pimcommon
  ${CMAKE_SOURCE_DIR}/pimcommon
  ${CMAKE_SOURCE_DIR}/pimcommon/util/
  ${CMAKE_CURRENT_BINARY_DIR}/../ontologies
  ${CMAKE_CURRENT_SOURCE_DIR}/filter
  ${CMAKE_CURRENT_SOURCE_DIR}/tag/
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/snippets
  ${CMAKE_CURRENT_SOURCE_DIR}/folder
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
)

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libmailcommon_SRCS
  collectionpage/attributeregistrar.cpp
  job/backupjob.cpp
  collectionpage/collectionannotationsattribute.cpp
  collectionpage/collectiongeneralpage.cpp
  collectionpage/collectionexpirypage.cpp
  collectionpage/expirecollectionattribute.cpp
  job/expirejob.cpp
  widgets/favoritecollectionwidget.cpp
  filter/filteraction.cpp
  filter/filteractionaddheader.cpp
  filter/filteractionaddtag.cpp
  filter/filteractionaddtoaddressbook.cpp
  filter/filteractionbeep.cpp
  filter/filteractioncopy.cpp
  filter/filteractiondelete.cpp
  filter/filteractiondict.cpp
  filter/filteractionexec.cpp
  filter/filteractionforward.cpp
  filter/filteractionmissingargumentdialog.cpp
  filter/filteractionmove.cpp
  filter/filteractionpipethrough.cpp
  filter/filteractionplaysound.cpp
  filter/filteractionredirect.cpp
  filter/filteractionremoveheader.cpp
  filter/filteractionreplyto.cpp
  filter/filteractionrewriteheader.cpp
  filter/filteractionsendfakedisposition.cpp
  filter/filteractionsendreceipt.cpp
  filter/filteractionsetidentity.cpp
  filter/filteractionsetstatus.cpp
  filter/filteractionstatus.cpp
  filter/filteractionunsetstatus.cpp
  filter/filteractionsettransport.cpp
  filter/filteractionwidget.cpp
  filter/filteractionwithaddress.cpp
  filter/filteractionwithcommand.cpp
  filter/filteractionwithfolder.cpp
  filter/filteractionwithnone.cpp
  filter/filteractionwithstring.cpp
  filter/filteractionwithstringlist.cpp
  filter/filteractionwithtest.cpp
  filter/filteractionwithuoid.cpp
  filter/filteractionwithurl.cpp
  filter/filtercontroller.cpp
  filter/filtereditdialog.cpp
  filter/filterimporterexporter.cpp
  filter/filterimporter/filterimporterabstract.cpp
  filter/filterimporter/filterimporterevolution.cpp
  filter/filterimporter/filterimportersylpheed.cpp
  filter/filterimporter/filterimporterthunderbird.cpp
  filter/filterimporter/filterimporterprocmail.cpp
  filter/filterimporter/filterimporterbalsa.cpp
  filter/filterimporter/filterimporterclawsmail.cpp
  filter/filterlog.cpp
  filter/filtermanager.cpp
  filter/filtermodel.cpp
  filter/itemcontext.cpp
  filter/kmfilterdialog.cpp
  filter/mailfilter.cpp
  filter/selectthunderbirdfilterfilesdialog.cpp
  filter/selectthunderbirdfilterfileswidget.cpp
  filter/mdnadvicedialog.cpp
  folder/foldercollection.cpp
  folder/foldercollectionmonitor.cpp
  job/folderjob.cpp
  folder/folderrequester.cpp
  folder/folderselectiondialog.cpp
  folder/foldertreeview.cpp
  folder/foldertreewidget.cpp
  folder/foldertreewidgetproxymodel.cpp
  folder/entitycollectionorderproxymodel.cpp
  job/jobscheduler.cpp
  kernel/mailkernel.cpp
  util/mailutil.cpp
  widgets/redirectdialog.cpp
  widgets/regexplineedit.cpp
  search/rulewidgethandlermanager.cpp
  search/searchpattern.cpp
  search/searchpatternedit.cpp
  mdn/sendmdnhandler.cpp
  snippets/snippetdialog.cpp
  snippets/snippetsmanager.cpp
  snippets/snippetsmodel.cpp
  snippets/snippetvariabledialog.cpp
  tag/tagwidget.cpp
  tag/tag.cpp
  tag/addtagdialog.cpp
)

soprano_add_ontology(libmailcommon_SRCS
  ${CMAKE_SOURCE_DIR}/messagecore/ontology/messagetag.trig
  "MessageTag"
  "Vocabulary"
  "trig"
)


set_source_files_properties(${CMAKE_SOURCE_DIR}/mailfilteragent/org.freedesktop.Akonadi.MailFilterAgent.xml PROPERTIES INCLUDE "../mailfilteragent/dbusoperators.h")

qt4_add_dbus_interfaces(libmailcommon_SRCS
  ${CMAKE_SOURCE_DIR}/mailfilteragent/org.freedesktop.Akonadi.MailFilterAgent.xml
)

kde4_add_ui_files(libmailcommon_SRCS filter/ui/filterconfigwidget.ui snippets/ui/snippetdialog.ui filter/ui/selectthunderbirdfilterfileswidget.ui)

if(NOT WINCE)
  set(libmailcommon_SRCS ${libmailcommon_SRCS} filter/soundtestwidget.cpp)
endif()

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(libmailcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface(settings.kcfg org.kde.Akonadi.POP3.Settings Pop3Settings)

qt4_add_dbus_interfaces(libmailcommon_SRCS
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

kde4_add_library(mailcommon ${LIBRARY_TYPE} ${libmailcommon_SRCS})

target_link_libraries(mailcommon messageviewer messagecore messagecomposer mailimporter
  templateparser
  kdepim
  akonadi_next
  pimcommon
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KIMAP_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
)

target_link_libraries(mailcommon
  ${NEPOMUK_QUERY_LIBRARIES}
  ${SOPRANO_LIBRARIES}
  niefast_apps
)

if(NOT WINCE)
  target_link_libraries(mailcommon incidenceeditorsng)
else()
  target_link_libraries(mailcommon incidenceeditorsngmobile)
endif()

set_target_properties(mailcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS mailcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(filter/filterimporter/tests)

#install(FILES viewer/viewer.h attachmentstrategy.h header/headerstrategy.h messageviewer_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)

if (QT_QTDESIGNER_FOUND)
  set(mailcommonwidgets_PART_SRCS)
  kde4_add_widget_files(mailcommonwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/mailcommon.widgets)

  kde4_add_plugin(mailcommonwidgets ${mailcommonwidgets_PART_SRCS})


  target_link_libraries(mailcommonwidgets  ${KDE4_KIO_LIBS} mailcommon)

  install(TARGETS mailcommonwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()


