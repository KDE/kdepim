kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_notepad_PART_SRCS notepad-conduit.cc notepad-factory.cc )

kde4_automoc(${conduit_notepad_PART_SRCS})

kde4_add_ui_files(conduit_notepad_PART_SRCS notepad-setup.ui )

kde4_add_kcfg_files(conduit_notepad_PART_SRCS notepadconduit.kcfgc )

kde4_add_plugin(conduit_notepad ${conduit_notepad_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_notepad )

target_link_libraries(conduit_notepad  ${QT_AND_KDECORE_LIBS} kpilot kdeui kio )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_notepad )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES notepad-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = notepad-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_notepad.la
#
#conduit_notepad_la_SOURCES = notepadconduit.kcfgc notepad-setup.ui \
#	notepad-conduit.cc notepad-factory.cc
#conduit_notepad_la_LDFLAGS= -module $(KDE_PLUGIN) $(all_libraries)
#conduit_notepad_la_LIBADD= ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KIO)
#
