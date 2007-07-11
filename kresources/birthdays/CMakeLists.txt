
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/common ${CMAKE_SOURCE_DIR}/kaddressbook/ ${CMAKE_BINARY_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkdepim )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kcal_kabc_PART_SRCS ${libkabcommon_SRCS} resourcekabc.cpp resourcekabcconfig.cpp )

kde4_add_kcfg_files(kcal_kabc_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )


kde4_add_plugin(kcal_kabc ${kcal_kabc_PART_SRCS})



target_link_libraries(kcal_kabc  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim )

install(TARGETS kcal_kabc  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabc.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
install( FILES resourcekabc.h resourcekabcconfig.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kcal)
