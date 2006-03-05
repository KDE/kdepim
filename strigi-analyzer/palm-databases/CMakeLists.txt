kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_palm_PART_SRCS kfile_palm.cpp )

kde4_automoc(${kfile_palm_PART_SRCS})

kde4_add_plugin(kfile_palm ${kfile_palm_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_palm )

target_link_libraries(kfile_palm  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_palm )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_palm.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for palm database meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(PISOCK_INCLUDE) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
## these are the headers for your project
#noinst_HEADERS   = kfile_palm.h
#
#kde_module_LTLIBRARIES = kfile_palm.la
#
#kfile_palm_la_SOURCES = kfile_palm.cpp 
#kfile_palm_la_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) -module $(KDE_PLUGIN)
#kfile_palm_la_LIBADD = $(LIB_KSYCOCA) $(PISOCK_LIB)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_palm.cpp -o $(podir)/kfile_palm.pot
#
#services_DATA = kfile_palm.desktop
#servicesdir = $(kde_servicesdir)
