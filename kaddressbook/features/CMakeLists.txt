
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook  )


########### next target ###############

set(kaddrbk_distributionlist_PART_SRCS distributionlistwidget.cpp )

kde4_automoc(kaddrbk_distributionlist ${kaddrbk_distributionlist_PART_SRCS})

kde4_add_plugin(kaddrbk_distributionlist WITH_PREFIX ${kaddrbk_distributionlist_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_distributionlist )

target_link_libraries(kaddrbk_distributionlist  ${KDE4_KABC_LIBS} kaddressbook ${LIB_KDE3SUPPORT})

install(TARGETS kaddrbk_distributionlist  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_resourceselection_PART_SRCS resourceselection.cpp )

kde4_automoc(kaddrbk_resourceselection ${kaddrbk_resourceselection_PART_SRCS})

kde4_add_plugin(kaddrbk_resourceselection WITH_PREFIX ${kaddrbk_resourceselection_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_resourceselection )

target_link_libraries(kaddrbk_resourceselection  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_resourceselection  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES distributionlist.desktop resourceselection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)

