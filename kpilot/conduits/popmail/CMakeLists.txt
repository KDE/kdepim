kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_popmail_PART_SRCS popmail-factory.cc setupDialog.cc popmail-conduit.cc )

kde4_automoc(${conduit_popmail_PART_SRCS})

kde4_add_ui_files(conduit_popmail_PART_SRCS setup-dialog.ui )

kde4_add_kcfg_files(conduit_popmail_PART_SRCS popmailSettings.kcfgc )

kde4_add_plugin(conduit_popmail ${conduit_popmail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_popmail )

target_link_libraries(conduit_popmail  ${QT_AND_KDECORE_LIBS} kpilot kio )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_popmail )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES popmail.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES popmail-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile for the popmail conduit
####
#
#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#METASOURCES = AUTO
#
#servicedir  = $(kde_servicesdir)
#service_DATA = popmail-conduit.desktop
#kde_kcfg_DATA = popmail.kcfg
#
######## This part is very kpilot specific
## you can add here more. This one gets installed 
#kde_module_LTLIBRARIES = conduit_popmail.la
#
## Which sources should be compiled for popmail_conduit
#conduit_popmail_la_SOURCES = popmailSettings.kcfgc setup-dialog.ui \
#	popmail-factory.cc setupDialog.cc \
#	popmail-conduit.cc
#conduit_popmail_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_popmail_la_LIBADD  =  ../../lib/libkpilot.la $(LIB_KIO)
#
## this option you can leave out. Just, if you use "make dist", you need it
#noinst_HEADERS = popmail-conduit.h setupDialog.h
#
