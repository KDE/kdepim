kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_mal_PART_SRCS mal-factory.cc mal-setup.cc mal-conduit.cc )

kde4_automoc(${conduit_mal_PART_SRCS})

kde4_add_ui_files(conduit_mal_PART_SRCS mal-setup_dialog.ui )

kde4_add_kcfg_files(conduit_mal_PART_SRCS malconduitSettings.kcfgc )

kde4_add_plugin(conduit_mal ${conduit_mal_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_mal )

target_link_libraries(conduit_mal  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_mal )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES malconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES mal_conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile for the avantgo conduit
####
#### The mal conduit is Copyright (C) 2002 by Reinhold Kainhofer
#
#
#INCLUDES= $(PISOCK_INCLUDE)  $(MAL_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = mal_conduit.desktop
#kde_kcfg_DATA = malconduit.kcfg
#
#kde_module_LTLIBRARIES = conduit_mal.la
#
#conduit_mal_la_SOURCES = malconduitSettings.kcfgc mal-setup_dialog.ui mal-factory.cc mal-setup.cc mal-conduit.cc 
#conduit_mal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_mal_la_LIBADD = ../../lib/libkpilot.la $(MAL_LIB) $(LIB_KDEUI)
#
