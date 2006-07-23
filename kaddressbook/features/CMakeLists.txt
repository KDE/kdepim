
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook  )


########### next target ###############

set(kaddrbk_distributionlist_PART_SRCS distributionlistwidget.cpp )

kde4_automoc(${kaddrbk_distributionlist_PART_SRCS})

kde4_add_plugin(kaddrbk_distributionlist WITH_PREFIX ${kaddrbk_distributionlist_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_distributionlist )

target_link_libraries(kaddrbk_distributionlist  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook ${LIB_KDE3SUPPORT})

install(TARGETS kaddrbk_distributionlist  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_resourceselection_PART_SRCS resourceselection.cpp )

kde4_automoc(${kaddrbk_resourceselection_PART_SRCS})

kde4_add_plugin(kaddrbk_resourceselection WITH_PREFIX ${kaddrbk_resourceselection_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_resourceselection )

target_link_libraries(kaddrbk_resourceselection  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_resourceselection  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES distributionlist.desktop resourceselection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook \
#           -I$(top_srcdir)  $(all_includes)
#
#kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la libkaddrbk_resourceselection.la
#
#XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#         $(top_builddir)/libkdepim/libkdepim.la
#
#libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
#libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_resourceselection_la_SOURCES = resourceselection.cpp
#libkaddrbk_resourceselection_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddrbk_resourceselection_la_LIBADD = $(XXLIBS)
#
#noinst_HEADERS = distributionlistwidget.h resourceselection.h
#
#METASOURCES = AUTO
#
#servicedir  = $(kde_servicesdir)/kaddressbook
#service_DATA = distributionlist.desktop resourceselection.desktop
