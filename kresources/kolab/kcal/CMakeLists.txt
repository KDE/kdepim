
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


QT4_ADD_DBUS_INTERFACE2( kcalkolab_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.groupware.xml kmail_groupwareinterface groupware_types.h)




kde4_add_library(kcalkolab SHARED ${kcalkolab_LIB_SRCS})

target_link_libraries(kcalkolab  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kdepim kgroupwarebase)

set_target_properties(kcalkolab PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcalkolab ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcal_kolab_PART_SRCS resourcekolab_plugin.cpp )


kde4_add_plugin(kcal_kolab ${kcal_kolab_PART_SRCS})



target_link_libraries(kcal_kolab  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kcalkolab )

install(TARGETS kcal_kolab  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kolab.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)


