
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if(BUILD_TESTING)
   add_subdirectory(addressline/blacklistbaloocompletion/autotests)
   add_subdirectory(addressline/autotests/)
endif()

add_subdirectory(pics)


add_definitions(-DTRANSLATION_DOMAIN=\"libkdepim\")

########### next target ###############
set(kdepim_progresswidget_LIB_SRCS
   progresswidget/progressmanager.cpp
   progresswidget/progressmanager_akonadiagent.cpp
   progresswidget/agentprogressmonitor.cpp
   progresswidget/progressdialog.cpp
   progresswidget/statusbarprogresswidget.cpp
   progresswidget/ssllabel.cpp
   progresswidget/progressstatusbarwidget.cpp
)

set(kdepim_job_LIB_SRCS
   job/addemailaddressjob.cpp
   job/addcontactjob.cpp
   job/openemailaddressjob.cpp
   job/addemaildisplayjob.cpp
   job/collectionsearchjob.cpp
   job/personsearchjob.cpp
)

set(kdepim_widgets_LIB_SRCS
   widgets/customlogwidget.cpp
   widgets/kdatepickerpopup.cpp
   widgets/kcheckcombobox.cpp
   widgets/kweekdaycheckcombo.cpp
   widgets/kwidgetlister.cpp
   widgets/pimmessagebox.cpp
   widgets/overlaywidget.cpp
   widgets/tagwidgets.cpp
   widgets/progressindicatorlabel.cpp
   widgets/progressindicatorwidget.cpp
   widgets/tagselectioncombo.cpp
)

set(kdepim_ldap_LIB_SRCS
   ldap/addhostdialog.cpp
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
   ldap/addhostdialog.cpp
)
ecm_qt_declare_logging_category(kdepim_ldap_LIB_SRCS HEADER ldapclient_debug.h IDENTIFIER LDAPCLIENT_LOG CATEGORY_NAME log_ldapclient)

set(kdepim_misc_LIB_SRCS
   misc/broadcaststatus.cpp
   misc/emailvalidator.cpp
   misc/maillistdrag.cpp
   misc/uistatesaver.cpp
)

set(kdepim_addressline_LIB_SRCS
   addressline/completionorder/completionordereditor.cpp
   addressline/completionorder/completionorderwidget.cpp
   addressline/addresslineedit/addresseelineedit.cpp
   addressline/addresslineedit/addresseelineedit_p.cpp
   addressline/addresslineedit/addresseelineeditstatic.cpp
   addressline/addresslineedit/kmailcompletion.cpp
   addressline/addresslineedit/baloocompletionemail.cpp
   addressline/completionconfiguredialog/completionconfiguredialog.cpp
   addressline/recentaddress/recentaddressdialog.cpp
   addressline/recentaddress/recentaddresswidget.cpp
   addressline/recentaddress/recentaddresses.cpp
)

set(kdepim_addressline_blacklist_baloo_LIB_SRCS
   addressline/blacklistbaloocompletion/blacklistbalooemailcompletiondialog.cpp
   addressline/blacklistbaloocompletion/blacklistbalooemaillist.cpp
   addressline/blacklistbaloocompletion/blacklistbalooemailsearchjob.cpp
   addressline/blacklistbaloocompletion/blacklistbalooemailutil.cpp
   addressline/blacklistbaloocompletion/blacklistbalooemailcompletionwidget.cpp
   addressline/blacklistbaloocompletion/blacklistbalooemailwarning.cpp
) 

set(kdepim_multiplyingline_LIB_SRCS
   multiplyingline/multiplyingline.cpp
   multiplyingline/multiplyinglineeditor.cpp
   multiplyingline/multiplyinglineview_p.cpp
)

set(kdepim_LIB_SRCS
   ${kdepim_addressline_blacklist_baloo_LIB_SRCS}
   ${kdepim_progresswidget_LIB_SRCS}
   ${kdepim_job_LIB_SRCS}
   ${kdepim_widgets_LIB_SRCS}
   ${kdepim_ldap_LIB_SRCS}
   ${kdepim_misc_LIB_SRCS}
   ${kdepim_addressline_LIB_SRCS}
   ${kdepim_multiplyingline_LIB_SRCS}
   prefs/kprefsdialog.cpp
)
ecm_qt_declare_logging_category(kdepim_LIB_SRCS HEADER libkdepim_debug.h IDENTIFIER LIBKDEPIM_LOG CATEGORY_NAME log_libkdepim)


add_library(KF5Libkdepim ${kdepim_LIB_SRCS})
generate_export_header(KF5Libkdepim BASE_NAME kdepim)
add_library(KF5::Libkdepim ALIAS KF5Libkdepim)
target_include_directories(KF5Libkdepim INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/Libkdepim/;${KDE_INSTALL_INCLUDEDIR_KF5}/libkdepim>")
target_include_directories(KF5Libkdepim PUBLIC "$<BUILD_INTERFACE:${libkdepim_SOURCE_DIR}/src;${libkdepim_BINARY_DIR}/src;>")



target_link_libraries(KF5Libkdepim
 PUBLIC
  KF5::AkonadiCore
  KF5::AkonadiContact
 PRIVATE
  KF5::AkonadiWidgets
  KF5::Ldap
  KF5::Contacts
  KF5::Codecs
  KF5::Completion
  KF5::I18n
  KF5::KCMUtils
  KF5::Wallet
  KF5::ItemViews
  KF5::AkonadiSearchPIM
  KF5::IconThemes
  KF5::KIOCore
  KF5::KIOWidgets
  Qt5::DBus
  Qt5::Network
)


if(MINGW)
   target_link_libraries(KF5Libkdepim oleaut32)
endif()


set_target_properties(KF5Libkdepim PROPERTIES
    VERSION ${LIBKDEPIM_VERSION_STRING}
    SOVERSION ${LIBKDEPIM_SOVERSION}
    EXPORT_NAME Libkdepim
)

install(TARGETS
    KF5Libkdepim
    EXPORT KF5LibkdepimTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(libkdepim_Camelcaseldap_HEADERS
  HEADER_NAMES
  LdapClientSearchConfig
  LdapClientSearch
  AddHostDialog
  LdapSearchDialog
  LdapClient 
  REQUIRED_HEADERS libkdepim_ldap_HEADERS
  PREFIX Libkdepim
  RELATIVE ldap
)

ecm_generate_headers(libkdepim_Camelcasewidgets_HEADERS
  HEADER_NAMES
  CustomLogWidget
  ProgressIndicatorWidget
  TagWidgets
  KCheckComboBox
  TagSelectionCombo
  ProgressIndicatorLabel
  KDatePickerPopup
  KWidgetLister
  KWeekdayCheckCombo
  PIMMessageBox
  TagSelectionCombo
  OverlayWidget
  REQUIRED_HEADERS libkdepim_widgets_HEADERS
  PREFIX Libkdepim
  RELATIVE widgets
)

ecm_generate_headers(libkdepim_Camelcasemisc_HEADERS
  HEADER_NAMES
  EmailValidator
  BroadcastStatus
  MaillistDrag
  UiStateSaver
  KCursorSaver
  REQUIRED_HEADERS libkdepim_misc_HEADERS
  PREFIX Libkdepim
  RELATIVE misc
)

ecm_generate_headers(libkdepim_Camelcaseprogresswidget_HEADERS
  HEADER_NAMES
  ProgressStatusBarWidget
  StatusbarProgressWidget
  ProgressDialog
  ProgressManager	
  REQUIRED_HEADERS libkdepim_progresswidget_HEADERS
  PREFIX Libkdepim
  RELATIVE progresswidget
)

ecm_generate_headers(libkdepim_Camelcasejob_HEADERS
  HEADER_NAMES
  AddContactJob
  AddEmailAddressJob
  AddEmailDisplayJob
  CollectionSearchJob
  OpenEmailAddressJob
  PersonSearchJob
  Person
 
  REQUIRED_HEADERS libkdepim_job_HEADERS
  PREFIX Libkdepim
  RELATIVE job
)

ecm_generate_headers(libkdepim_Camelcasepref_HEADERS
  HEADER_NAMES
  KPrefsDialog 
  REQUIRED_HEADERS libkdepim_pref_HEADERS
  PREFIX Libkdepim
  RELATIVE prefs
)

ecm_generate_headers(libkdepim_Camelcasemultiline_HEADERS
  HEADER_NAMES
  MultiplyingLine  
  MultiplyingLineEditor
  REQUIRED_HEADERS libkdepim_multiline_HEADERS
  PREFIX Libkdepim
  RELATIVE  multiplyingline
)


ecm_generate_headers(libkdepim_Camelcaseaddressline_HEADERS
  HEADER_NAMES
  AddresseeLineEdit 
  REQUIRED_HEADERS libkdepim_addressline_HEADERS
  PREFIX Libkdepim
  RELATIVE  addressline/addresslineedit/
)

ecm_generate_headers(libkdepim_Camelcaserecentaddress_HEADERS
  HEADER_NAMES
  RecentAddresses  
  REQUIRED_HEADERS libkdepim_recentaddress_HEADERS
  PREFIX Libkdepim
  RELATIVE addressline/recentaddress/
)

ecm_generate_headers(libkdepim_Camelcasecompletionorder_HEADERS
  HEADER_NAMES
  CompletionOrderEditor
  REQUIRED_HEADERS libkdepim_completionorder_HEADERS
  PREFIX Libkdepim
  RELATIVE addressline/completionorder
)

ecm_generate_headers(libkdepim_Camelcasecompletionconfiguredialog_HEADERS
  HEADER_NAMES
  CompletionConfigureDialog
  REQUIRED_HEADERS libkdepim_completionconfiguredialog_HEADERS
  PREFIX Libkdepim
  RELATIVE addressline/completionconfiguredialog/
)

ecm_generate_headers(libkdepim_Camelcaseblacklistbaloocompletion_HEADERS
  HEADER_NAMES
  BlackListBalooEmailCompletionDialog
  REQUIRED_HEADERS libkdepim_blacklistbaloocompletion_HEADERS
  PREFIX Libkdepim
  RELATIVE addressline/blacklistbaloocompletion/
)

ecm_generate_pri_file(BASE_NAME Libkdepim
    LIB_NAME KF5Libkdepim
    DEPS "AkonadiCore AkonadiContact" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/Libkdepim
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${libkdepim_SOURCE_DIR})
install(FILES
    ${libkdepim_Camelcaseblacklistbaloocompletion_HEADERS}
    ${libkdepim_Camelcasecompletionconfiguredialog_HEADERS}
    ${libkdepim_Camelcasecompletionorder_HEADERS}
    ${libkdepim_CamelcaseCamelCase_HEADERS}
    ${libkdepim_Camelcaseldap_HEADERS}
    ${libkdepim_Camelcasewidgets_HEADERS}
    ${libkdepim_Camelcaseprogresswidget_HEADERS}
    ${libkdepim_Camelcasemisc_HEADERS}
    ${libkdepim_Camelcasejob_HEADERS}
    ${libkdepim_Camelcasepref_HEADERS}
    ${libkdepim_Camelcasemultiline_HEADERS}
    ${libkdepim_Camelcaseaddressline_HEADERS}
    ${libkdepim_Camelcaserecentaddress_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/Libkdepim
    COMPONENT Devel
)

install(FILES
    ${libkdepim_blacklistbaloocompletion_HEADERS}
    ${libkdepim_completionconfiguredialog_HEADERS}
    ${libkdepim_completionorder_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kdepim_export.h
    ${libkdepim_CamelCase_HEADERS}
    ${libkdepim_ldap_HEADERS}
    ${libkdepim_progresswidget_HEADERS}
    ${libkdepim_misc_HEADERS}
    ${libkdepim_widgets_HEADERS}
    ${libkdepim_job_HEADERS}
    ${libkdepim_pref_HEADERS}
    ${libkdepim_multiline_HEADERS}
    ${libkdepim_addressline_HEADERS}
    ${libkdepim_recentaddress_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/libkdepim
    COMPONENT Devel
)

install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


endif()
add_subdirectory(designer)
########### next target ###############


set( kcm_ldap_SRCS
     ldap/kcmldap.cpp
)

add_library(kcm_ldap MODULE ${kcm_ldap_SRCS})


target_link_libraries(kcm_ldap   KF5::Libkdepim KF5::IconThemes KF5::KCMUtils KF5::I18n KF5::Ldap Qt5::Core)

kcoreaddons_desktop_to_json(kcm_ldap ldap/kcmldap.desktop)

install(TARGETS kcm_ldap DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES ldap/kcmldap.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
