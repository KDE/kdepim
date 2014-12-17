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
  ${CMAKE_CURRENT_SOURCE_DIR}/filter
  ${CMAKE_CURRENT_SOURCE_DIR}/tag/
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/snippets
  ${CMAKE_CURRENT_SOURCE_DIR}/folder
  ${CMAKE_CURRENT_SOURCE_DIR}/util
)

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libmailcommon_filter_SRCS
  filter/kmfilteraccountlist.cpp
  filter/kmfilterlistbox.cpp
  filter/filterselectiondialog.cpp
  filter/filterconverter/filterconverttosieve.cpp
  filter/filterconverter/filterconverttosieveresultdialog.cpp
  filter/filteractions/filteraction.cpp
  filter/filteractions/filteractionaddheader.cpp
  filter/filteractions/filteractionaddtag.cpp
  filter/filteractions/filteractionaddtoaddressbook.cpp
  filter/filteractions/filteractionbeep.cpp
  filter/filteractions/filteractioncopy.cpp
  filter/filteractions/filteractiondelete.cpp
  filter/filteractions/filteractiondict.cpp
  filter/filteractions/filteractionexec.cpp
  filter/filteractions/filteractionforward.cpp
  filter/dialog/filteractionmissingargumentdialog.cpp
  filter/filteractions/filteractionmove.cpp
  filter/filteractions/filteractionpipethrough.cpp
  filter/filteractions/filteractionplaysound.cpp
  filter/filteractions/filteractionredirect.cpp
  filter/filteractions/filteractionremoveheader.cpp
  filter/filteractions/filteractionreplyto.cpp
  filter/filteractions/filteractionrewriteheader.cpp
  filter/filteractions/filteractionsendfakedisposition.cpp
  filter/filteractions/filteractionsendreceipt.cpp
  filter/filteractions/filteractionsetidentity.cpp
  filter/filteractions/filteractionsetstatus.cpp
  filter/filteractions/filteractionstatus.cpp
  filter/filteractions/filteractionunsetstatus.cpp
  filter/filteractions/filteractionsettransport.cpp
  filter/filteractions/filteractionwidget.cpp
  filter/filteractions/filteractionwithaddress.cpp
  filter/filteractions/filteractionwithcommand.cpp
  filter/filteractions/filteractionwithfolder.cpp
  filter/filteractions/filteractionwithnone.cpp
  filter/filteractions/filteractionwithstring.cpp
  filter/filteractions/filteractionwithstringlist.cpp
  filter/filteractions/filteractionwithtest.cpp
  filter/filteractions/filteractionwithuoid.cpp
  filter/filteractions/filteractionwithurl.cpp
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
  filter/dialog/selectthunderbirdfilterfilesdialog.cpp
  filter/dialog/selectthunderbirdfilterfileswidget.cpp
  filter/mdnadvicedialog.cpp
)

set(libmailcommon_collection_SRCS
  collectionpage/attributeregistrar.cpp
  collectionpage/collectionannotationsattribute.cpp
  collectionpage/collectiongeneralpage.cpp
  collectionpage/collectionexpirypage.cpp
  collectionpage/expirecollectionattribute.cpp
  collectionpage/newmailnotifierattribute.cpp
)

set(libmailcommon_folder_SRCS
  folder/foldercollection.cpp
  folder/foldercollectionmonitor.cpp
  folder/folderrequester.cpp
  folder/folderselectiondialog.cpp
  folder/foldertreeview.cpp
  folder/foldertreewidget.cpp
  folder/foldertreewidgetproxymodel.cpp
  folder/entitycollectionorderproxymodel.cpp
  folder/accountconfigorderdialog.cpp
)

set(libmailcommon_job_SRCS
  job/jobscheduler.cpp
  job/folderjob.cpp
  job/expirejob.cpp
  job/backupjob.cpp
)

set(libmailcommon_search_SRCS
  search/rulewidgethandlermanager.cpp
  search/searchpattern.cpp
  search/searchpatternedit.cpp
  search/textrulerwidgethandler.cpp
  search/statusrulewidgethandler.cpp
  search/messagerulewidgethandler.cpp
  search/tagrulewidgethandler.cpp
  search/numericrulewidgethandler.cpp
  search/daterulewidgethandler.cpp
  search/numericdoublerulewidgethandler.cpp
  search/headersrulerwidgethandler.cpp
)

set(libmailcommon_snippets_SRCS
  snippets/snippetdialog.cpp
  snippets/snippetsmanager.cpp
  snippets/snippetsmodel.cpp
  snippets/snippetvariabledialog.cpp
)

set(libmailcommon_tag_SRCS
  tag/tagwidget.cpp
  tag/tag.cpp
  tag/addtagdialog.cpp
)

set(libmailcommon_widget_SRCS
  widgets/redirectdialog.cpp
  widgets/regexplineedit.cpp
  widgets/favoritecollectionwidget.cpp
)

set(libmailcommon_SRCS
  kernel/mailkernel.cpp
  util/mailutil.cpp
  mdn/sendmdnhandler.cpp
  ${libmailcommon_widget_SRCS}
  ${libmailcommon_tag_SRCS}
  ${libmailcommon_snippets_SRCS}
  ${libmailcommon_search_SRCS}
  ${libmailcommon_job_SRCS}
  ${libmailcommon_collection_SRCS}
  ${libmailcommon_folder_SRCS}   
  ${libmailcommon_filter_SRCS}
)

kde4_add_kcfg_files(libmailcommon_SRCS
    settings/mailcommonsettings_base.kcfgc
)



set_source_files_properties(${CMAKE_SOURCE_DIR}/agents/mailfilteragent/org.freedesktop.Akonadi.MailFilterAgent.xml PROPERTIES INCLUDE "dbusoperators.h")

qt4_add_dbus_interfaces(libmailcommon_SRCS
  ${CMAKE_SOURCE_DIR}/agents/mailfilteragent/org.freedesktop.Akonadi.MailFilterAgent.xml
)

kde4_add_ui_files(libmailcommon_SRCS filter/ui/filterconfigwidget.ui snippets/ui/snippetdialog.ui filter/ui/selectthunderbirdfilterfileswidget.ui)

set(libmailcommon_SRCS ${libmailcommon_SRCS} filter/soundtestwidget.cpp)

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
)

target_link_libraries(mailcommon
  incidenceeditorsng
)


set_target_properties(mailcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS mailcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(filter/filterimporter/tests)
add_subdirectory(collectionpage/autotests)
#install(FILES viewer/viewer.h viewer/attachmentstrategy.h header/headerstrategy.h messageviewer_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)

if (QT_QTDESIGNER_FOUND)
  set(mailcommonwidgets_PART_SRCS)
  kde4_add_widget_files(mailcommonwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/mailcommon.widgets)

  kde4_add_plugin(mailcommonwidgets ${mailcommonwidgets_PART_SRCS})


  target_link_libraries(mailcommonwidgets  ${KDE4_KIO_LIBS} mailcommon)

  install(TARGETS mailcommonwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()


