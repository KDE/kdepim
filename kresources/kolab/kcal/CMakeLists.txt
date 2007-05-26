
include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared  )


########### next target ###############

set(kcalkolab_LIB_SRCS ${libresourcekolabshared_SRCS}
   incidence.cpp 
   event.cpp 
   task.cpp 
   journal.cpp 
   resourcekolab.cpp )

#kde4_add_dcop_skels(kcalkolab_LIB_SRCS
#    ${CMAKE_SOURCE_DIR}/kresources/kolab/shared/kmailconnection.h
#    )

#kde4_add_dcop_stubs(kcalkolab_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/kmailicalIface.h )

qt4_generate_dbus_interface( ${CMAKE_SOURCE_DIR}/kmail/kmailicalIface.h  org.kde.kmail.groupware.xml )
QT4_ADD_DBUS_INTERFACE( kcalkolab_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.groupware.xml kmail_groupwareinterface )



kde4_automoc(${kcalkolab_LIB_SRCS})

kde4_add_library(kcalkolab SHARED ${kcalkolab_LIB_SRCS})

target_link_libraries(kcalkolab  ${KDE4_KDECORE_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KCAL_LIBS} kdepim )

set_target_properties(kcalkolab PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kcalkolab  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_kolab_PART_SRCS resourcekolab_plugin.cpp )

kde4_automoc(${kcal_kolab_PART_SRCS})

kde4_add_plugin(kcal_kolab ${kcal_kolab_PART_SRCS})



target_link_libraries(kcal_kolab  ${KDE4_KDECORE_LIBS} kcalkolab )

install(TARGETS kcal_kolab  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kolab.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)


