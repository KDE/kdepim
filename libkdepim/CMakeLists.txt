

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(tests)
add_subdirectory(pics)


add_definitions(-DTRANSLATION_DOMAIN=\"libkdepim\")

option(KDEPIM_INPROCESS_LDAP "Use in-process rather than KIO slave LDAP" FALSE)
if (KDEPIM_INPROCESS_LDAP)
  add_definitions( -DKDEPIM_INPROCESS_LDAP )
endif ()

########### next target ###############

set(kdepim_proxy_LIB_SRCS
   proxy/quotacolorproxymodel.cpp
   proxy/kcolumnfilterproxymodel.cpp
)

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
   widgets/spellchecklineedit.cpp
   widgets/selectedcollectiondialog.cpp
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

set(kdepim_freebusy_LIB_SRCS
   freebusymodel/freeperiodmodel.cpp
   freebusymodel/freebusyitem.cpp
   freebusymodel/freebusyitemmodel.cpp
   freebusymodel/freebusycalendar.cpp
)

set(kdepim_ldap_LIB_SRCS
   ldap/addhostdialog.cpp
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
   ldap/ldapclient_debug.cpp
   ldap/addhostdialog.cpp
)

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
) 

set(kdepim_multiplyingline_LIB_SRCS
   multiplyingline/multiplyingline.cpp
   multiplyingline/multiplyinglineeditor.cpp
   multiplyingline/multiplyinglineview_p.cpp
)

set(kdepim_LIB_SRCS
   ${kdepim_proxy_LIB_SRCS}
   ${kdepim_addressline_blacklist_baloo_LIB_SRCS}
   ${kdepim_progresswidget_LIB_SRCS}
   ${kdepim_job_LIB_SRCS}
   ${kdepim_widgets_LIB_SRCS}
   ${kdepim_ldap_LIB_SRCS}
   ${kdepim_misc_LIB_SRCS}
   ${kdepim_addressline_LIB_SRCS}
   ${kdepim_freebusy_LIB_SRCS}
   ${kdepim_multiplyingline_LIB_SRCS}
   libkdepim_debug.cpp
   prefs/kprefsdialog.cpp
)

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

add_library(kdepim ${kdepim_LIB_SRCS})
generate_export_header(kdepim BASE_NAME kdepim)

target_link_libraries(kdepim
  KF5::Contacts
  KF5::Codecs
  KF5::Ldap
  KF5::KCMUtils
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiContact
  KF5::AkonadiWidgets
  KF5::AkonadiCalendar
  KF5::KCMUtils
  KF5::AkonadiSearchPIM
  KF5::SonnetCore
  KF5::Wallet
)


target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  KF5::Contacts
  KF5::Codecs
  KF5::Ldap
  KF5::AkonadiCore
  KF5::AkonadiContact
  KF5::AkonadiCalendar
)

if(MINGW)
   target_link_libraries(kdepim oleaut32)
endif()

set_target_properties(kdepim PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kdepim ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (Qt5Designer_FOUND)
    kf5designerplugin_add_plugin(kdepimwidgets ${CMAKE_CURRENT_SOURCE_DIR}/designer/kdepim.widgets)

    target_link_libraries(kdepimwidgets
        kdepim
    )
    if(NOT WIN32)
        # Since there are no libraries provided by this module,
        # there is no point including the build tree in RPath,
        # and then having to edit it at install time.
        set_target_properties(kdepimwidgets PROPERTIES
            SKIP_BUILD_RPATH TRUE
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()

    install(TARGETS
        kdepimwidgets
        DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/designer
    )
endif()


########### next target ###############


set( kcm_ldap_SRCS
     ldap/addhostdialog.cpp
     ldap/kcmldap.cpp
)

add_library(kcm_ldap MODULE ${kcm_ldap_SRCS})


target_link_libraries(kcm_ldap   kdepim)

kcoreaddons_desktop_to_json(kcm_ldap ldap/kcmldap.desktop)

install(TARGETS kcm_ldap DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES ldap/kcmldap.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})

if (BUILD_TESTING)
add_subdirectory(addressline/blacklistbaloocompletion/autotests)
add_subdirectory(addressline/autotests/)
add_subdirectory(freebusymodel/autotests)
endif()
