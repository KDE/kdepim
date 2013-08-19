



########### next target ###############
include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim/
)


set(kcal_resourceremote_LIB_SRCS resourceremote.cpp resourceremoteconfig.cpp )


kde4_add_library(kcal_resourceremote ${LIBRARY_TYPE} ${kcal_resourceremote_LIB_SRCS})

target_link_libraries(kcal_resourceremote  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kdepim )

set_target_properties(kcal_resourceremote PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kcal_resourceremote ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcal_remote_PART_SRCS resourceremote_plugin.cpp )


kde4_add_plugin(kcal_remote ${kcal_remote_PART_SRCS})



target_link_libraries(kcal_remote
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  kcal_resourceremote
  kdepim
)

install(TARGETS kcal_remote  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES remote.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)

