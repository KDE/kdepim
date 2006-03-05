kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_knotes_PART_SRCS knotes-factory.cc knotes-setup.cc knotes-action.cc )

kde4_automoc(${conduit_knotes_PART_SRCS})

kde4_add_ui_files(conduit_knotes_PART_SRCS setup_base.ui )

kde4_add_dcop_stubs(conduit_knotes_PART_SRCS KNotesIface.h )

kde4_add_kcfg_files(conduit_knotes_PART_SRCS knotesconduitSettings.kcfgc )

kde4_add_plugin(conduit_knotes ${conduit_knotes_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_knotes )

target_link_libraries(conduit_knotes  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_knotes )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES knotesconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES knotes-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = knotes-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_knotes.la
#
#
#conduit_knotes_la_SOURCES = knotesconduitSettings.kcfgc knotes-factory.cc \
#	knotes-setup.cc knotes-action.cc \
#	setup_base.ui \
#	KNotesIface.stub
#conduit_knotes_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_knotes_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
#
#kde_kcfg_DATA = knotesconduit.kcfg
