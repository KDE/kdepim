
include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared  )


########### next target ###############

set(kabckolab_LIB_SRCS resourcekolab.cpp contact.cpp ${libresourcekolabshared_SRCS})

#kde4_add_dcop_stubs(kabckolab_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/kmailicalIface.h )

kde4_automoc(${kabckolab_LIB_SRCS})

kde4_add_library(kabckolab SHARED ${kabckolab_LIB_SRCS})

target_link_libraries(kabckolab  ${KDE4_KDECORE_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KCAL_LIBS} kdepim)

set_target_properties(kabckolab PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kabckolab  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kabc_kolab_PART_SRCS resourcekolab_plugin.cpp )

kde4_automoc(${kabc_kolab_PART_SRCS})

kde4_add_plugin(kabc_kolab ${kabc_kolab_PART_SRCS})



target_link_libraries(kabc_kolab  ${KDE4_KDECORE_LIBS} kabckolab ${KDE4_KCAL_LIBS} )

install(TARGETS kabc_kolab  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kolab.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)


