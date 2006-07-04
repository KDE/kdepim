
add_subdirectory( protocols ) 
add_subdirectory( icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${GPGME_INCLUDES} 
${QT_INCLUDES} )


########### next target ###############

set(kaddrbk_cryptosettings_PART_SRCS cryptowidget.cpp )

kde4_automoc(${kaddrbk_cryptosettings_PART_SRCS})

kde4_add_plugin(kaddrbk_cryptosettings WITH_PREFIX ${kaddrbk_cryptosettings_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_cryptosettings )

target_link_libraries(kaddrbk_cryptosettings  ${KDE4_KDECORE_LIBS} kabc kleopatra kaddressbook)

install(TARGETS kaddrbk_cryptosettings  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES cryptosettings.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbookimprotocol.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook/interfaces \
#           -I$(top_srcdir)/kaddressbook \
#           -I$(top_srcdir)/libkleo \
#            \
#           -I$(top_builddir)/kaddressbook/editors \
#           $(GPGME_CFLAGS) \
#           $(all_includes)
#
#kde_module_LTLIBRARIES = libkaddrbk_cryptosettings.la
#
#XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#         $(top_builddir)/libkleo/libkleopatra.la
#
#libkaddrbk_cryptosettings_la_SOURCES = cryptowidget.cpp
#libkaddrbk_cryptosettings_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) # -no-undefined
#libkaddrbk_cryptosettings_la_LIBADD = $(XXLIBS)
#
#noinst_HEADERS = cryptowidget.h imeditorwidget.h imaddresswidget.h
#
#METASOURCES = AUTO
#
#SUBDIRS = protocols icons
#servicedir  = $(kde_servicesdir)/kaddressbook
#service_DATA = cryptosettings.desktop
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kaddressbookimprotocol.desktop
#
#noinst_LTLIBRARIES = libkabim.la
#libkabim_la_SOURCES = imeditorwidget.cpp imeditorbase.ui imaddressbase.ui imaddresswidget.cpp
#libkabim_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libkabim_la_LIBADD = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la
