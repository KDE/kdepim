kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_python_PART_SRCS python-conduit.cc python-factory.cc )

kde4_automoc(${conduit_python_PART_SRCS})

kde4_add_ui_files(conduit_python_PART_SRCS python-setup.ui )

kde4_add_kcfg_files(conduit_python_PART_SRCS pythonconduit.kcfgc )

kde4_add_plugin(conduit_python ${conduit_python_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_python )

target_link_libraries(conduit_python  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_python )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES python-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) $(PERL_CFLAGS) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = python-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_python.la
#
#
#conduit_python_la_SOURCES = pythonconduit.kcfgc python-setup.ui \
#	python-conduit.cc python-factory.cc
#conduit_python_la_LDFLAGS = -module $(KDE_PLUGIN)  $(all_libraries)
#conduit_python_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
#
