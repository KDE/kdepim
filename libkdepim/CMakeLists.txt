KDE4_NO_ENABLE_FINAL(libkdepim)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

#add_definitions(-DQT3_SUPPORT_WARNINGS)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_subdirectory(tests)
add_subdirectory(pics)

add_definitions(${QT_QTDBUS_DEFINITIONS})

include_directories( ${Boost_INCLUDE_DIR} )

########### next target ###############

set(kdepim_LIB_SRCS
   addemailaddressjob.cpp
   addcontactjob.cpp
   openemailaddressjob.cpp
   kdescendantsproxymodel.cpp
   ktimeedit.cpp
   kprefsdialog.cpp
   kdateedit.cpp
   maillistdrag.cpp
   addresseelineedit.cpp
   kdatepickerpopup.cpp
   ktimezonecombobox.cpp
   ldap/ldapclient.cpp
   ldap/ldapsearchdialog.cpp
   overlaywidget.cpp
   progressmanager.cpp
   agentprogressmonitor.cpp
   progressdialog.cpp
   statusbarprogresswidget.cpp
   ssllabel.cpp
   completionordereditor.cpp
   broadcaststatus.cpp
   kwidgetlister.cpp
   recentaddresses.cpp
   sendsmsdialog.cpp
   kmailcompletion.cpp
   uistatesaver.cpp
   pimmessagebox.cpp
)

if (KDEPIMLIBS_KRESOURCES_LIBRARY)
  set(kdepim_LIB_SRCS ${kdepim_LIB_SRCS}
    kresourceprefs.cpp
  )
endif (KDEPIMLIBS_KRESOURCES_LIBRARY)

kde4_add_library(kdepim SHARED ${kdepim_LIB_SRCS})

target_link_libraries(kdepim
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KMIME_LIBRARY}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
)

target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
)

if(MINGW)
   target_link_libraries(kdepim oleaut32)
endif(MINGW)


set_target_properties(kdepim PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepim ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (QT_QTDESIGNER_FOUND)
  set(kdepimwidgets_PART_SRCS)
  kde4_add_widget_files(kdepimwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kdepim.widgets)

  kde4_add_plugin(kdepimwidgets ${kdepimwidgets_PART_SRCS})


  target_link_libraries(kdepimwidgets  ${KDE4_KIO_LIBS} kdepim)

  install(TARGETS kdepimwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif (QT_QTDESIGNER_FOUND)

########### next target ###############

set( kcm_ldap_SRCS
     ldap/addhostdialog.cpp
     ldap/kcmldap.cpp
)

kde4_add_plugin(kcm_ldap ${kcm_ldap_SRCS})

target_link_libraries(kcm_ldap ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdepim)

install(TARGETS kcm_ldap DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ldap/kcmldap.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
