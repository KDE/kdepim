
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/common ${CMAKE_SOURCE_DIR}/kaddressbook/ ${CMAKE_BINARY_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_BINARY_DIR}/kaddressbook/common )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kcal_kabc_PART_SRCS resourcekabc.cpp resourcekabcconfig.cpp )

kde4_add_plugin(kcal_kabc ${kcal_kabc_PART_SRCS})

target_link_libraries(kcal_kabc  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KRESOURCES_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} kdepim kabcommon)

install(TARGETS kcal_kabc  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabc.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
