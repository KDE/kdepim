kde_enable_exceptions()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
#find_package(KF5Ldap ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Abc ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimIdentities ${KF5_VERSION} CONFIG REQUIRED)
find_package(Baloo ${KF5_VERSION} CONFIG REQUIRED)

add_subdirectory(tests)
add_subdirectory(pics)


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
   misc/statisticsproxymodel.cpp
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
  ${KDE4_SOLID_LIBS}
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
  ${BALOO_PIM_LIBRARY}
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

if (QT_QTDESIGNER_FOUND)
  set(kdepimwidgets_PART_SRCS)
  kde4_add_widget_files(kdepimwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/kdepim.widgets)

  add_library(kdepimwidgets MODULE ${kdepimwidgets_PART_SRCS})


  target_link_libraries(kdepimwidgets  KF5::KIOCore kdepim)

  install(TARGETS kdepimwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()

########### next target ###############

#REACTIVATE
#set( kcm_ldap_SRCS
#     ldap/addhostdialog.cpp
#     ldap/kcmldap.cpp
#)

#add_library(kcm_ldap MODULE ${kcm_ldap_SRCS})

#target_link_libraries(kcm_ldap   kdepim)

#install(TARGETS kcm_ldap DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES ldap/kcmldap.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
