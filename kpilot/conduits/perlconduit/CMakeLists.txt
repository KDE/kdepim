kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_perl_PART_SRCS perl-conduit.cc perl-factory.cc )

kde4_automoc(${conduit_perl_PART_SRCS})

kde4_add_ui_files(conduit_perl_PART_SRCS perl-setup.ui )

kde4_add_kcfg_files(conduit_perl_PART_SRCS perlconduit.kcfgc )

kde4_add_plugin(conduit_perl ${conduit_perl_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_perl )

target_link_libraries(conduit_perl  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_perl )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES perl-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) $(PERL_CFLAGS) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = perl-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_perl.la
#
#
#conduit_perl_la_SOURCES = perlconduit.kcfgc perl-setup.ui \
#	perl-conduit.cc perl-factory.cc
#conduit_perl_la_LDFLAGS = -module $(KDE_PLUGIN) $(PERL_LDFLAGS) $(all_libraries)
#conduit_perl_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
#
