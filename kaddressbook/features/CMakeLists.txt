
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook  )


########### next target ###############

set(kaddrbk_distributionlist_PART_SRCS distributionlistwidget.cpp )


kde4_add_plugin(kaddrbk_distributionlist ${kaddrbk_distributionlist_PART_SRCS})



target_link_libraries(kaddrbk_distributionlist kaddressbook kabinterfaces kabcommon ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_distributionlist  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kaddrbk_distributionlistng_PART_SRCS distributionlistngwidget.cpp )


kde4_add_plugin(kaddrbk_distributionlistng ${kaddrbk_distributionlistng_PART_SRCS})

target_link_libraries(kaddrbk_distributionlistng kaddressbook kabinterfaces kabcommon ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_distributionlistng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_resourceselection_PART_SRCS resourceselection.cpp )


kde4_add_plugin(kaddrbk_resourceselection ${kaddrbk_resourceselection_PART_SRCS})



target_link_libraries(kaddrbk_resourceselection kaddressbook kabinterfaces ${KDE4_KDECORE_LIBS})

install(TARGETS kaddrbk_resourceselection  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES distributionlist.desktop distributionlistng.desktop resourceselection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)

