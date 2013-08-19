
########### next target ###############
include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim/
)


set(kcal_resourceblog_LIB_SRCS resourceblog.cpp resourceblogconfig.cpp )

kde4_add_library(kcal_resourceblog ${LIBRARY_TYPE} ${kcal_resourceblog_LIB_SRCS})

target_link_libraries(kcal_resourceblog ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KBLOG_LIBS} kdepim)

set_target_properties(kcal_resourceblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kcal_resourceblog ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kcal_blog_PART_SRCS resourceblog_plugin.cpp )

kde4_add_plugin(kcal_blog ${kcal_blog_PART_SRCS})

target_link_libraries(kcal_blog
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${QT_QTGUI_LIBRARY}
  kcal_resourceblog
  kdepim
)

install(TARGETS kcal_blog  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES blog.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
