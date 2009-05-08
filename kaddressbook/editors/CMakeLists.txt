
add_subdirectory( protocols ) 
add_subdirectory( icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook ${KDE4_INCLUDE_DIR} ${GPGME_INCLUDES} ${CMAKE_SOURCE_DIR}/libkleo ${QT_INCLUDES} ${Boost_INCLUDE_DIR} )


########### next target ###############

set(kaddrbk_cryptosettings_PART_SRCS cryptowidget.cpp )


kde4_add_plugin(kaddrbk_cryptosettings ${kaddrbk_cryptosettings_PART_SRCS})



target_link_libraries(kaddrbk_cryptosettings  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} kleo kaddressbookprivate kabinterfaces )

install(TARGETS kaddrbk_cryptosettings  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES cryptosettings.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbookimprotocol.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

