
include_directories( ${CMAKE_SOURCE_DIR}/kresources/scalix/shared ${CMAKE_SOURCE_DIR}/knotes  )


########### next target ###############

set(knotesscalix_LIB_SRCS resourcescalix.cpp ${libresourcescalixshared_SRCS} ${libknotesresources_SRCS})


QT4_ADD_DBUS_INTERFACE2( knotesscalix_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.groupware.xml kmail_groupwareinterface groupware_types.h )



kde4_add_library(knotesscalix SHARED ${knotesscalix_LIB_SRCS})

target_link_libraries(knotesscalix  ${KDE4_KRESOURCES_LIBS} ${KDE4_KCAL_LIBS} kdepim kgroupwarebase)

set_target_properties(knotesscalix PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS knotesscalix ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(knotes_scalix_PART_SRCS resourcescalix_plugin.cpp )


kde4_add_plugin(knotes_scalix ${knotes_scalix_PART_SRCS})



target_link_libraries(knotes_scalix  ${KDE4_KDECORE_LIBS} knotesscalix ${KDE4_KCAL_LIBS})

install(TARGETS knotes_scalix  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES scalix.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/knotes)


