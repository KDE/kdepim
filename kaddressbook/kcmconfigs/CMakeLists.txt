
include_directories(
	${CMAKE_SOURCE_DIR}/kaddressbook
	${CMAKE_BINARY_DIR}/kaddressbook
	${CMAKE_SOURCE_DIR}/kaddressbook/common
	${CMAKE_BINARY_DIR}/kaddressbook/common
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces
	
)


########### next target ###############

set(kcm_kabconfig_PART_SRCS 
   kcmkabconfig.cpp 
   kabconfigwidget.cpp 
   extensionconfigdialog.cpp 
   addresseewidget.cpp )


kde4_add_plugin(kcm_kabconfig ${kcm_kabconfig_PART_SRCS})



target_link_libraries(kcm_kabconfig kaddressbookprivate kabcommon ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_kabconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kabldapconfig_PART_SRCS addhostdialog.cpp kcmkabldapconfig.cpp ldapoptionswidget.cpp )


kde4_add_plugin(kcm_kabldapconfig ${kcm_kabldapconfig_PART_SRCS})



target_link_libraries(kcm_kabldapconfig kabcommon ${KDE4_KIO_LIBS} kdepim ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KLDAP_LIBS})

install(TARGETS kcm_kabldapconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kabcustomfields_PART_SRCS kcmkabcustomfields.cpp )


kde4_add_plugin(kcm_kabcustomfields ${kcm_kabcustomfields_PART_SRCS})



target_link_libraries(kcm_kabcustomfields kaddressbookprivate kabcommon ${KDE4_KDEUI_LIBS} kdepim)

install(TARGETS kcm_kabcustomfields  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabconfig.desktop kabldapconfig.desktop                      kabcustomfields.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


