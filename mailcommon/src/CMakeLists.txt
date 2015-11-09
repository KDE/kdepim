
add_definitions(-DTRANSLATION_DOMAIN=\"libmailcommon\")

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


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
  filter/filterimporterexporter.cpp
  filter/filterimporter/filterimporterabstract.cpp
  filter/filterimporter/filterimporterevolution.cpp
  filter/filterimporter/filterimportersylpheed.cpp
  filter/filterimporter/filterimporterthunderbird.cpp
  filter/filterimporter/filterimporterprocmail.cpp
  filter/filterimporter/filterimporterbalsa.cpp
  filter/filterimporter/filterimporterclawsmail.cpp
  filter/filterimporter/filterimportergmail.cpp
  filter/filterlog.cpp
  filter/filtermanager.cpp
  filter/itemcontext.cpp
  filter/kmfilterdialog.cpp
  filter/mailfilter.cpp
  filter/dialog/selectthunderbirdfilterfilesdialog.cpp
  filter/dialog/selectthunderbirdfilterfileswidget.cpp
  filter/mdnadvicedialog.cpp
)

set(libmailcommon_invalidfilters 
  filter/invalidfilters/invalidfilterdialog.cpp
  filter/invalidfilters/invalidfilterlistview.cpp
  filter/invalidfilters/invalidfilterwidget.cpp
  filter/invalidfilters/invalidfilterinfo.cpp
  filter/invalidfilters/invalidfilterlistitemdelegate.cpp
  filter/invalidfilters/invalidfilterlistmodel.cpp
  filter/invalidfilters/invalidfilterinfowidget.cpp
)

set(libmailcommon_collection_SRCS
  collectionpage/collectiontypeutil.cpp
  collectionpage/attributeregistrar.cpp
  collectionpage/attributes/collectionannotationsattribute.cpp
  collectionpage/collectiongeneralpage.cpp
  collectionpage/collectionexpirypage.cpp
  collectionpage/attributes/expirecollectionattribute.cpp
  collectionpage/incidencesforwidget.cpp
  collectionpage/contenttypewidget.cpp
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
  search/widgethandler/rulewidgethandlermanager.cpp
  search/searchpattern.cpp
  search/searchpatternedit.cpp
  search/widgethandler/textrulerwidgethandler.cpp
  search/widgethandler/statusrulewidgethandler.cpp
  search/widgethandler/messagerulewidgethandler.cpp
  search/widgethandler/tagrulewidgethandler.cpp
  search/widgethandler/numericrulewidgethandler.cpp
  search/widgethandler/daterulewidgethandler.cpp
  search/widgethandler/numericdoublerulewidgethandler.cpp
  search/widgethandler/headersrulerwidgethandler.cpp
  search/searchrule/searchrulenumerical.cpp
  search/searchrule/searchruledate.cpp
  search/searchrule/searchrulestring.cpp 
  search/searchrule/searchrulestatus.cpp
  search/searchrule/searchrule.cpp
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
  widgets/redirectwidget.cpp
  widgets/favoritecollectionwidget.cpp
)

set(libmailcommon_mdn_SRCS
  mdn/sendmdnhandler.cpp
  mdn/mdnstateattribute.cpp
)

set(libmailcommon_util_SRCS
  util/mailutil.cpp
  util/resourcereadconfigfile.cpp
)

set(libmailcommon_SRCS
  kernel/mailkernel.cpp
  ${libmailcommon_util_SRCS}
  ${libmailcommon_mdn_SRCS}
  ${libmailcommon_invalidfilters}
  ${libmailcommon_widget_SRCS}
  ${libmailcommon_tag_SRCS}
  ${libmailcommon_snippets_SRCS}
  ${libmailcommon_search_SRCS}
  ${libmailcommon_job_SRCS}
  ${libmailcommon_collection_SRCS}
  ${libmailcommon_folder_SRCS}
  ${libmailcommon_filter_SRCS}
)
kconfig_add_kcfg_files(libmailcommon_SRCS
    settings/mailcommonsettings_base.kcfgc
)

ecm_qt_declare_logging_category(libmailcommon_SRCS HEADER mailcommon_debug.h IDENTIFIER MAILCOMMON_LOG CATEGORY_NAME log_mailcommon)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/dbusinterfaces/org.freedesktop.Akonadi.MailFilterAgent.xml PROPERTIES INCLUDE "dbusoperators.h")

qt5_add_dbus_interfaces(libmailcommon_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/dbusinterfaces/org.freedesktop.Akonadi.MailFilterAgent.xml
)

ki18n_wrap_ui(libmailcommon_SRCS filter/ui/filterconfigwidget.ui snippets/ui/snippetdialog.ui filter/ui/selectthunderbirdfilterfileswidget.ui)

set(libmailcommon_SRCS ${libmailcommon_SRCS} filter/soundtestwidget.cpp)

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "pimcommon/metatype.h")
  qt5_add_dbus_interface(libmailcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface(settings.kcfg org.kde.Akonadi.POP3.Settings Pop3Settings)

qt5_add_dbus_interfaces(libmailcommon_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dbusinterfaces/org.kde.Korganizer.Calendar.xml
)

add_library(KF5MailCommon ${libmailcommon_SRCS})
generate_export_header(KF5MailCommon BASE_NAME mailcommon)


add_library(KF5::MailCommon ALIAS KF5MailCommon)

target_link_libraries(KF5MailCommon
PUBLIC
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::MessageComposer
  KF5::PimCommon
PRIVATE
  KF5::Libkdepim
  KF5::MessageViewer
  KF5::MailImporter
  KF5::MessageCore
  KF5::TemplateParser
  KF5::Mime
  KF5::Codecs
  KF5::MailTransport
  Phonon::phonon4qt5
  KF5::XmlGui
  KF5::KIOWidgets
  KF5::WindowSystem
  KF5::IconThemes
  KF5::Archive
  KF5::ItemViews
  KF5::Ldap
)
target_include_directories(KF5MailCommon INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/MailCommon/;${KDE_INSTALL_INCLUDEDIR_KF5}/mailcommon>")
target_include_directories(KF5MailCommon PUBLIC "$<BUILD_INTERFACE:${mailcommon_SOURCE_DIR}/src;${mailcommon_BINARY_DIR}/src;${mailcommon_SOURCE_DIR}/src/search>")


set_target_properties(KF5MailCommon PROPERTIES
    VERSION ${MAILCOMMON_VERSION_STRING}
    SOVERSION ${MAILCOMMON_SOVERSION}
    EXPORT_NAME MailCommon
)

install(TARGETS
    KF5MailCommon
    EXPORT KF5MailCommonTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

if (BUILD_TESTING)
   add_subdirectory(filter/filterimporter/autotests)
   add_subdirectory(collectionpage/autotests)
   add_subdirectory(filter/autotests)
   add_subdirectory(snippets/autotests)
   add_subdirectory(filter/tests) 
   add_subdirectory(search/autotests)
   add_subdirectory(mdn/autotests/)
endif()

ecm_generate_headers(MailCommon_CamelCase_HEADERS
  HEADER_NAMES
  MailUtil
  MetaType
  ResourceReadConfigFile
  REQUIRED_HEADERS MailCommon_HEADERS
  PREFIX MailCommon
  RELATIVE util
)

ecm_generate_headers(MailCommon_Camelcasetop_HEADERS
  HEADER_NAMES
  DBusOperators
  REQUIRED_HEADERS MailCommon_top_HEADERS
  PREFIX MailCommon
)

ecm_generate_headers(MailCommon_Camelcasetag_HEADERS
  HEADER_NAMES
  TagWidget
  Tag
  AddTagDialog
  REQUIRED_HEADERS MailCommon_tag_HEADERS
  PREFIX MailCommon
  RELATIVE tag
)

ecm_generate_headers(MailCommon_Camelcasefolder_HEADERS
  HEADER_NAMES
  FolderCollection
  FolderRequester
  FolderCollectionMonitor
  AccountConfigOrderDialog
  FolderTreeView
  FolderTreeWidget
  FolderSelectionDialog
  FolderTreeWidgetProxyModel
  REQUIRED_HEADERS MailCommon_folder_HEADERS
  PREFIX MailCommon
  RELATIVE folder
)

ecm_generate_headers(MailCommon_Camelcasekernel_HEADERS
  HEADER_NAMES
  MailKernel
  REQUIRED_HEADERS MailCommon_kernel_HEADERS
  PREFIX MailCommon
  RELATIVE kernel
)

ecm_generate_headers(MailCommon_Camelcaseinterfaces_HEADERS
  HEADER_NAMES
  MailInterfaces
  REQUIRED_HEADERS MailCommon_interfaces_HEADERS
  PREFIX MailCommon
  RELATIVE interfaces
)


ecm_generate_headers(MailCommon_Camelcasefilter_HEADERS
  HEADER_NAMES
  FilterLog
  MailFilter
  FilterImporterExporter
  FilterManager
  KMFilterDialog
  ItemContext
  REQUIRED_HEADERS MailCommon_filter_HEADERS
  PREFIX MailCommon
  RELATIVE filter
)

ecm_generate_headers(MailCommon_Camelcasejob_HEADERS
  HEADER_NAMES
  BackupJob
  JobScheduler
  FolderJob
  REQUIRED_HEADERS MailCommon_job_HEADERS
  PREFIX MailCommon
  RELATIVE job
)

ecm_generate_headers(MailCommon_Camelcasefilteraction_HEADERS
  HEADER_NAMES
  FilterAction
  FilterActionDict
  REQUIRED_HEADERS MailCommon_filteraction_HEADERS
  PREFIX MailCommon
  RELATIVE filter/filteractions/
)

ecm_generate_headers(MailCommon_Camelcasemdn_HEADERS
  HEADER_NAMES
  SendMdnHandler
  MDNStateAttribute
  REQUIRED_HEADERS MailCommon_mdn_HEADERS
  PREFIX MailCommon
  RELATIVE mdn
)


ecm_generate_headers(MailCommon_Camelcasecollectionpage_HEADERS
  HEADER_NAMES
  IncidencesForWidget
  CollectionTypeUtil
  CollectionGeneralPage
  CollectionExpiryPage
  REQUIRED_HEADERS MailCommon_collectionpage_HEADERS
  PREFIX MailCommon
  RELATIVE collectionpage
)

ecm_generate_headers(MailCommon_Camelcasecollectionpageattributes_HEADERS
  HEADER_NAMES
  CollectionAnnotationsAttribute
  ExpireCollectionAttribute
  REQUIRED_HEADERS MailCommon_collectionpageattributes_HEADERS
  PREFIX MailCommon
  RELATIVE collectionpage/attributes
)

ecm_generate_headers(MailCommon_Camelcasefilterimporter_HEADERS
  HEADER_NAMES
  FilterImporterBalsa
  FilterImporterClawsMail
  FilterImporterAbstract
  REQUIRED_HEADERS MailCommon_filterimporter_HEADERS
  PREFIX MailCommon
  RELATIVE filter/filterimporter/
)

ecm_generate_headers(MailCommon_Camelcasesnippets_HEADERS
  HEADER_NAMES
  SnippetsManager
  REQUIRED_HEADERS MailCommon_snippets_HEADERS
  PREFIX MailCommon
  RELATIVE snippets
)

ecm_generate_headers(MailCommon_Camelcasekernel_HEADERS
  HEADER_NAMES
  SearchPattern
  SearchPatternEdit
  REQUIRED_HEADERS MailCommon_kernel_HEADERS
  PREFIX MailCommon
  RELATIVE search
)

ecm_generate_headers(MailCommon_Camelcasewidgets_HEADERS
  HEADER_NAMES
  RedirectDialog
  FavoriteCollectionWidget
  REQUIRED_HEADERS MailCommon_widgets_HEADERS
  PREFIX MailCommon
  RELATIVE widgets
)

ecm_generate_headers(MailCommon_Camelcasesearchrule_HEADERS
  HEADER_NAMES
  SearchRule
  SearchRuleStatus
  REQUIRED_HEADERS MailCommon_searchrule_HEADERS
  PREFIX MailCommon
  RELATIVE search/searchrule
)

ecm_generate_pri_file(BASE_NAME MailCommon
    LIB_NAME KF5MailCommon
    DEPS "AkonadiCore AkonadiMime MessageComposer PimCommon" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/MailCommon
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${mailcommon_SOURCE_DIR})
install(FILES
    ${MailCommon_Camelcasesearchrule_HEADERS}
    ${MailCommon_CamelCase_HEADERS}
    ${MailCommon_Camelcasetop_HEADERS}
    ${MailCommon_Camelcasetag_HEADERS}
    ${MailCommon_Camelcasefolder_HEADERS}
    ${MailCommon_Camelcasekernel_HEADERS}
    ${MailCommon_Camelcaseinterfaces_HEADERS}
    ${MailCommon_Camelcasefilter_HEADERS}
    ${MailCommon_Camelcasejob_HEADERS}
    ${MailCommon_Camelcasefilteraction_HEADERS}
    ${MailCommon_Camelcasemdn_HEADERS}
    ${MailCommon_Camelcasecollectionpage_HEADERS}
    ${MailCommon_Camelcasecollectionpageattributes_HEADERS}
    ${MailCommon_Camelcasefilterimporter_HEADERS}
    ${MailCommon_Camelcasesnippets_HEADERS}
    ${MailCommon_Camelcasekernel_HEADERS}
    ${MailCommon_Camelcasewidgets_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/MailCommon
    COMPONENT Devel
)

install(FILES
    ${MailCommon_searchrule_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/mailcommon_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/pop3settings.h
    ${CMAKE_CURRENT_BINARY_DIR}/mailcommonsettings_base.h
    ${MailCommon_HEADERS}
    ${MailCommon_top_HEADERS}
    ${MailCommon_tag_HEADERS}
    ${MailCommon_folder_HEADERS}
    ${MailCommon_kernel_HEADERS}
    ${MailCommon_interfaces_HEADERS}
    ${MailCommon_filter_HEADERS}
    ${MailCommon_job_HEADERS}
    ${MailCommon_filteraction_HEADERS}
    ${MailCommon_mdn_HEADERS}
    ${MailCommon_collectionpage_HEADERS}
    ${MailCommon_collectionpageattributes_HEADERS}
    ${MailCommon_filterimporter_HEADERS}
    ${MailCommon_snippets_HEADERS}
    ${MailCommon_kernel_HEADERS}
    ${MailCommon_widgets_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/mailcommon
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


endif()



add_subdirectory(designer)
