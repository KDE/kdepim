kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_vcf_PART_SRCS kfile_vcf.cpp )

kde4_automoc(${kfile_vcf_PART_SRCS})

kde4_add_plugin(kfile_vcf ${kfile_vcf_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_vcf )

target_link_libraries(kfile_vcf  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_vcf )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_vcf.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for vcf file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
## these are the headers for your project
#noinst_HEADERS   = kfile_vcf.h
#
#kde_module_LTLIBRARIES = kfile_vcf.la
#
#kfile_vcf_la_SOURCES = kfile_vcf.cpp 
#kfile_vcf_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_vcf_la_LIBADD = $(LIB_KSYCOCA)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_vcf.cpp -o $(podir)/kfile_vcf.pot
#
#services_DATA = kfile_vcf.desktop
#servicesdir = $(kde_servicesdir)
