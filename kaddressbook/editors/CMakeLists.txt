
add_subdirectory( protocols ) 
add_subdirectory( icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${GPGME_INCLUDES} 
${QT_INCLUDES} )


########### next target ###############

set(kaddrbk_cryptosettings_PART_SRCS cryptowidget.cpp )

kde4_automoc(kaddrbk_cryptosettings ${kaddrbk_cryptosettings_PART_SRCS})

kde4_add_plugin(kaddrbk_cryptosettings WITH_PREFIX ${kaddrbk_cryptosettings_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_cryptosettings )

target_link_libraries(kaddrbk_cryptosettings  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kleopatra kaddressbook)

install(TARGETS kaddrbk_cryptosettings  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES cryptosettings.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbookimprotocol.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

