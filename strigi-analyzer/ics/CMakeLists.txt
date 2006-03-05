kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_ics_PART_SRCS kfile_ics.cpp )

kde4_automoc(${kfile_ics_PART_SRCS})

kde4_add_plugin(kfile_ics ${kfile_ics_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_ics )

target_link_libraries(kfile_ics  ${QT_AND_KDECORE_LIBS} kcal kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_ics )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_ics.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Makefile.am for ics file meta info plugin
#
## set the include path for X, kdepim, qt and KDE
#INCLUDES = \
#  -I$(top_srcdir) \
#  $(all_includes)
#
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
## these are the headers for your project
#noinst_HEADERS   = kfile_ics.h
#
#kde_module_LTLIBRARIES = kfile_ics.la
#
#kfile_ics_la_SOURCES = kfile_ics.cpp
#kfile_ics_la_LDFLAGS =  $(all_libraries) -module $(KDE_PLUGIN)
#kfile_ics_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(LIB_KSYCOCA)
#
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_ics.cpp -o $(podir)/kfile_ics.pot
#
#services_DATA = kfile_ics.desktop
#servicesdir = $(kde_servicesdir)
