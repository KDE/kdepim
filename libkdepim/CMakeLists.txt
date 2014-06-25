kde_enable_exceptions()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

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
)

set(kdepim_ldap_LIB_SRCS
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   ldap/ldapclientsearch.cpp
   ldap/ldapclientsearchconfig.cpp
)

set(kdepim_misc_LIB_SRCS
   misc/broadcaststatus.cpp
   misc/maillistdrag.cpp
   misc/uistatesaver.cpp
)

set(kdepim_addressline_LIB_SRCS
   addressline/completionordereditor.cpp
   addressline/addresseelineedit.cpp
   addressline/recentaddresses.cpp
   addressline/kmailcompletion.cpp
)

set(kdepim_LIB_SRCS
   ${kdepim_progresswidget_LIB_SRCS}
   ${kdepim_job_LIB_SRCS}
   ${kdepim_widgets_LIB_SRCS}
   ${kdepim_ldap_LIB_SRCS}
   ${kdepim_misc_LIB_SRCS}
   ${kdepim_addressline_LIB_SRCS}
   prefs/kprefsdialog.cpp
   multiplyingline/multiplyingline.cpp
   multiplyingline/multiplyinglineeditor.cpp
   multiplyingline/multiplyinglineview_p.cpp
)

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

add_library(kdepim ${LIBRARY_TYPE} ${kdepim_LIB_SRCS})

target_link_libraries(kdepim
  KF5::Abc
  KF5::PimUtils
  KF5::Ldap
  KF5::PimIdentities
  KF5::KCMUtils
  KF5::Mime
  KF5::PimTextEdit
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiContact
  KF5::AkonadiAgentBase
  KF5::AkonadiWidgets
  KF5::SonnetCore
  KF5::KCMUtils
  KF5::Wallet
  baloopim
)


target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  KF5::Abc
  KF5::PimUtils
  KF5::Ldap
  KF5::PimIdentities
  KF5::PimTextEdit
  KF5::AkonadiCore
  KF5::AkonadiContact
)

if(MINGW)
   target_link_libraries(kdepim oleaut32)
endif()

set_target_properties(kdepim PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kdepim ${INSTALL_TARGETS_DEFAULT_ARGS})


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
        DESTINATION ${QT_PLUGIN_INSTALL_DIR}/designer
    )
endif()


########### next target ###############

set( kcm_ldap_SRCS
     ldap/addhostdialog.cpp
     ldap/kcmldap.cpp
)

add_library(kcm_ldap MODULE ${kcm_ldap_SRCS})

target_link_libraries(kcm_ldap   kdepim)

install(TARGETS kcm_ldap DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ldap/kcmldap.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
