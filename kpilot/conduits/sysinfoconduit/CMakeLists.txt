kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_sysinfo_PART_SRCS sysinfo-factory.cc sysinfo-setup.cc sysinfo-conduit.cc )

kde4_automoc(${conduit_sysinfo_PART_SRCS})

kde4_add_ui_files(conduit_sysinfo_PART_SRCS sysinfo-setup_dialog.ui )

kde4_add_kcfg_files(conduit_sysinfo_PART_SRCS sysinfoSettings.kcfgc )

kde4_add_plugin(conduit_sysinfo ${conduit_sysinfo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_sysinfo )

target_link_libraries(conduit_sysinfo  ${QT_AND_KDECORE_LIBS} kpilot kdeui kio )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_sysinfo )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES sysinfoconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES sysinfo_conduit.desktop )
install_files( ${DATA_INSTALL_DIR}/kpilot/sysinfoconduit FILES Template.html Template.txt )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile for the sysinfo conduit
####
#### The sysinfo conduit is Copyright (C) 2003 by Reinhold Kainhofer
#
#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = sysinfo_conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_sysinfo.la
#
#conduit_sysinfo_la_SOURCES = sysinfoSettings.kcfgc \
#	sysinfo-factory.cc \
#	sysinfo-setup.cc \
#	sysinfo-conduit.cc \
#	sysinfo-setup_dialog.ui
#conduit_sysinfo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_sysinfo_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KIO)
#
#kpilot_sysinfo_data_DATA = Template.html Template.txt
#kpilot_sysinfo_datadir = $(kde_datadir)/kpilot/sysinfoconduit
#EXTRA_DIST = $(kpilot_sysinfo_data_DATA)
#kde_kcfg_DATA = sysinfoconduit.kcfg
