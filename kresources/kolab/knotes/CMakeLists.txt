
include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${CMAKE_SOURCE_DIR}/knotes  )


########### next target ###############

set(knoteskolab_LIB_SRCS resourcekolab.cpp note.cpp ${libresourcekolabshared_SRCS} ${libknotesresources_SRCS})


QT4_ADD_DBUS_INTERFACE2( knoteskolab_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.groupware.xml kmail_groupwareinterface groupware_types.h)



kde4_add_library(knoteskolab SHARED ${knoteskolab_LIB_SRCS})

target_link_libraries(knoteskolab  ${KDE4_KRESOURCES_LIBS} ${KDE4_KCAL_LIBS} kdepim kgroupwarebase)

set_target_properties(knoteskolab PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS knoteskolab ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(knotes_kolab_PART_SRCS resourcekolab_plugin.cpp )


kde4_add_plugin(knotes_kolab ${knotes_kolab_PART_SRCS})



target_link_libraries(knotes_kolab  ${KDE4_KDECORE_LIBS} knoteskolab ${KDE4_KRESOURCES_LIBS})

install(TARGETS knotes_kolab  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kolabresource.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/knotes)


