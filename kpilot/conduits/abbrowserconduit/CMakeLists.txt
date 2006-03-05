kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_address_PART_SRCS 
   resolutionDialog.cc 
   abbrowser-factory.cc 
   abbrowser-setup.cc 
   abbrowser-conduit.cc )

kde4_automoc(${conduit_address_PART_SRCS})

kde4_add_ui_files(conduit_address_PART_SRCS resolutionDialog_base.ui kaddressbookConduit.ui )

kde4_add_kcfg_files(conduit_address_PART_SRCS abbrowserSettings.kcfgc )

kde4_add_plugin(conduit_address ${conduit_address_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_address )

target_link_libraries(conduit_address  ${QT_AND_KDECORE_LIBS} kpilot kabc kabc_file )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_address )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kaddressbook/contacteditorpages FILES KPilotCustomFieldEditor.ui )
install_files( ${KCFG_INSTALL_DIR} FILES abbrowserconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES abbrowser_conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#kde_module_LTLIBRARIES = conduit_address.la
#
#conduit_address_la_SOURCES = resolutionDialog_base.ui kaddressbookConduit.ui \
#	abbrowserSettings.kcfgc \
#	resolutionDialog.cc abbrowser-factory.cc \
#	abbrowser-setup.cc abbrowser-conduit.cc
#conduit_address_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_address_la_LIBADD = ../../lib/libkpilot.la -lkabc -lkabc_file
#
#servicedir = $(kde_servicesdir)
#service_DATA = abbrowser_conduit.desktop
#kde_kcfg_DATA = abbrowserconduit.kcfg
#
#kabcustompagedir = $(kde_datadir)/kaddressbook/contacteditorpages
#kabcustompage_DATA = KPilotCustomFieldEditor.ui
#
#METASOURCES = AUTO
