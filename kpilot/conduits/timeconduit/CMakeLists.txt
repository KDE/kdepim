kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_time_PART_SRCS time-factory.cc time-setup.cc time-conduit.cc )

kde4_automoc(${conduit_time_PART_SRCS})

kde4_add_ui_files(conduit_time_PART_SRCS time-setup_dialog.ui )

kde4_add_kcfg_files(conduit_time_PART_SRCS timeConduitSettings.kcfgc )

kde4_add_plugin(conduit_time ${conduit_time_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_time )

target_link_libraries(conduit_time  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_time )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES timeconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES time_conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile for the time conduit
####
#### The time conduit is Copyright (C) 2002 by Reinhold Kainhofer
#
#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = time_conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_time.la
#
#
#conduit_time_la_SOURCES = timeConduitSettings.kcfgc \
#	time-factory.cc \
#	time-setup.cc \
#	time-conduit.cc \
#	time-setup_dialog.ui
#conduit_time_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_time_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
# 
#kde_kcfg_DATA = timeconduit.kcfg
