kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_rfc822_PART_SRCS kfile_rfc822.cpp )

kde4_automoc(${kfile_rfc822_PART_SRCS})

kde4_add_plugin(kfile_rfc822 ${kfile_rfc822_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_rfc822 )

target_link_libraries(kfile_rfc822  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_rfc822 )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_rfc822.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for rfc822 file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
## these are the headers for your project
#noinst_HEADERS   = kfile_rfc822.h
#
#kde_module_LTLIBRARIES = kfile_rfc822.la
#
#kfile_rfc822_la_SOURCES = kfile_rfc822.cpp 
#kfile_rfc822_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_rfc822_la_LIBADD = $(LIB_KSYCOCA)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_rfc822.cpp -o $(podir)/kfile_rfc822.pot
#
#services_DATA = kfile_rfc822.desktop
#servicesdir = $(kde_servicesdir)
