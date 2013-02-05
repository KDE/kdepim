KDE4_NO_ENABLE_FINAL(libkdepim)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_subdirectory(tests)
add_subdirectory(pics)

add_definitions(${QT_QTDBUS_DEFINITIONS})

include_directories( ${Boost_INCLUDE_DIR} )
include_directories( ${SOPRANO_INCLUDE_DIR} ${NEPOMUK_CORE_INCLUDE_DIR} )

option(KDEPIM_INPROCESS_LDAP "Use in-process rather than KIO slave LDAP" FALSE)
if (KDEPIM_INPROCESS_LDAP)
  add_definitions( -DKDEPIM_INPROCESS_LDAP )
endif ()

########### next target ###############

set(kdepim_LIB_SRCS
   addemailaddressjob.cpp
   addcontactjob.cpp
   completionordereditor.cpp
   openemailaddressjob.cpp
   kprefsdialog.cpp
   maillistdrag.cpp
   addresseelineedit.cpp
   kdatepickerpopup.cpp
   kcheckcombobox.cpp
   kweekdaycheckcombo.cpp
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   overlaywidget.cpp
   progressmanager.cpp
   progressmanager_akonadiagent.cpp
   agentprogressmonitor.cpp
   progressdialog.cpp
   statusbarprogresswidget.cpp
   ssllabel.cpp
   broadcaststatus.cpp
   kwidgetlister.cpp
   recentaddresses.cpp
   sendsmsdialog.cpp
   kmailcompletion.cpp
   pimmessagebox.cpp
   multiplyingline.cpp
   multiplyinglineeditor.cpp
   multiplyinglineview_p.cpp
   uistatesaver.cpp
   nepomukwarning.cpp
   statisticsproxymodel.cpp
   customlogwidget.cpp
   kmsubjectlineedit.cpp
)

if (KDEPIMLIBS_KRESOURCES_LIBS)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    kresourceprefs.cpp
  )
endif ()

if (KDEPIM_INPROCESS_LDAP)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    ldap/ldapsession.cpp
    ldap/ldapqueryjob.cpp
  )
endif ()

kde4_add_library(kdepim ${LIBRARY_TYPE} ${kdepim_LIB_SRCS})

target_link_libraries(kdepim
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_SOLID_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
)


target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
)

if(MINGW)
   target_link_libraries(kdepim oleaut32)
endif()

set_target_properties(kdepim PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepim ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (QT_QTDESIGNER_FOUND)
  set(kdepimwidgets_PART_SRCS)
  kde4_add_widget_files(kdepimwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kdepim.widgets)

  kde4_add_plugin(kdepimwidgets ${kdepimwidgets_PART_SRCS})


  target_link_libraries(kdepimwidgets  ${KDE4_KIO_LIBS} kdepim)

  install(TARGETS kdepimwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()

########### next target ###############

set( kcm_ldap_SRCS
     ldap/addhostdialog.cpp
     ldap/kcmldap.cpp
)

if (NOT WINCE)
  kde4_add_plugin(kcm_ldap ${kcm_ldap_SRCS})
else ()
  kde4_add_plugin(kcm_ldap STATIC ${kcm_ldap_SRCS})
endif ()

target_link_libraries(kcm_ldap ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdepim)

install(TARGETS kcm_ldap DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ldap/kcmldap.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
