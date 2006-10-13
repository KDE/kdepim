
include_directories(
	${CMAKE_SOURCE_DIR}/kaddressbook
	${CMAKE_BINARY_DIR}/kaddressbook
	${CMAKE_SOURCE_DIR}/kaddressbook/common
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces
	
)


########### next target ###############

set(kcm_kabconfig_PART_SRCS 
   kcmkabconfig.cpp 
   kabconfigwidget.cpp 
   extensionconfigdialog.cpp 
   addresseewidget.cpp )

kde4_automoc(kcm_kabconfig ${kcm_kabconfig_PART_SRCS})

kde4_add_plugin(kcm_kabconfig ${kcm_kabconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabconfig )

target_link_libraries(kcm_kabconfig  ${KDE4_KIO_LIBS} kdepim kaddressbook )

install(TARGETS kcm_kabconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kabldapconfig_PART_SRCS addhostdialog.cpp kcmkabldapconfig.cpp ldapoptionswidget.cpp )

kde4_automoc(kcm_kabldapconfig ${kcm_kabldapconfig_PART_SRCS})

kde4_add_plugin(kcm_kabldapconfig ${kcm_kabldapconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabldapconfig )

target_link_libraries(kcm_kabldapconfig  ${KDE4_KIO_LIBS} kdepim )

install(TARGETS kcm_kabldapconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kabcustomfields_PART_SRCS kcmkabcustomfields.cpp )

kde4_automoc(kcm_kabcustomfields ${kcm_kabcustomfields_PART_SRCS})

kde4_add_plugin(kcm_kabcustomfields ${kcm_kabcustomfields_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabcustomfields )

target_link_libraries(kcm_kabcustomfields  ${KDE4_KDEUI_LIBS} kaddressbook )

install(TARGETS kcm_kabcustomfields  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabconfig.desktop kabldapconfig.desktop                      kabcustomfields.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


