
include_directories( ${CMAKE_SOURCE_DIR}/kresources/scalix/shared  )


########### next target ###############

set(kcalscalix_LIB_SRCS ${libresourcescalixshared_SRCS}
   resourcescalix.cpp )

#kde4_add_dcop_skels(kcalscalix_LIB_SRCS
#    ${CMAKE_SOURCE_DIR}/kresources/scalix/shared/kmailconnection.h
#    )


qt4_generate_dbus_interface( ${CMAKE_SOURCE_DIR}/kmail/groupwareadaptor.h  org.kde.kmail.groupware.xml )
QT4_ADD_DBUS_INTERFACE( kcalscalix_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.groupware.xml kmail_groupwareinterface )




kde4_add_library(kcalscalix SHARED ${kcalscalix_LIB_SRCS})

target_link_libraries(kcalscalix  ${KDE4_KDECORE_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KCAL_LIBS} kdepim )

set_target_properties(kcalscalix PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcalscalix  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_scalix_PART_SRCS resourcescalix_plugin.cpp )


kde4_add_plugin(kcal_scalix ${kcal_scalix_PART_SRCS})



target_link_libraries(kcal_scalix  ${KDE4_KDECORE_LIBS} kcalscalix )

install(TARGETS kcal_scalix  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES scalix.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)


