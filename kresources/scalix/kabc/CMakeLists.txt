
include_directories( ${CMAKE_SOURCE_DIR}/kresources/scalix/shared  )


########### next target ###############

set(kabcscalix_LIB_SRCS resourcescalix.cpp contact.cpp ${libresourcescalixshared_SRCS})

qt4_generate_dbus_interface( ${CMAKE_SOURCE_DIR}/kmail/groupwareadaptor.h  org.kde.kmail.groupware.xml )
QT4_ADD_DBUS_INTERFACE( kabcscalix_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.groupware.xml kmail_groupwareinterface )


kde4_add_library(kabcscalix SHARED ${kabcscalix_LIB_SRCS})

target_link_libraries(kabcscalix  ${KDE4_KDECORE_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KCAL_LIBS} kdepim)

set_target_properties(kabcscalix PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabcscalix  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kabc_scalix_PART_SRCS resourcescalix_plugin.cpp )


kde4_add_plugin(kabc_scalix ${kabc_scalix_PART_SRCS})



target_link_libraries(kabc_scalix  ${KDE4_KDECORE_LIBS} kabcscalix ${KDE4_KCAL_LIBS} )

install(TARGETS kabc_scalix  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES scalix.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)


