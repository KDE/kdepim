kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_vcal_PART_SRCS vcal-conduit.cc vcal-factory.cc vcal-setup.cc )

kde4_automoc(${conduit_vcal_PART_SRCS})

kde4_add_plugin(conduit_vcal ${conduit_vcal_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_vcal )

target_link_libraries(conduit_vcal  ${QT_AND_KDECORE_LIBS} kpilot kcal vcalconduit_shared )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_vcal )


########### next target ###############

set(conduit_todo_PART_SRCS todo-factory.cc todo-setup.cc todo-conduit.cc )

kde4_automoc(${conduit_todo_PART_SRCS})

kde4_add_plugin(conduit_todo ${conduit_todo_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_todo )

target_link_libraries(conduit_todo  ${QT_AND_KDECORE_LIBS} kpilot kcal vcalconduit_shared )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_todo )


########### next target ###############


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES vcalconduitbase.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES vcal-conduit.desktop todo-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

####
#### Makefile for vcal and todo conduits. These two conduits share most code,
#### so it seems logical to put them in one place.
####
#
#INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
#	$(PISOCK_INCLUDE) $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir  = $(kde_servicesdir)
#service_DATA = vcal-conduit.desktop todo-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_vcal.la conduit_todo.la
#noinst_LTLIBRARIES = libvcalconduit_shared.la
#
#libvcalconduit_shared_la_SOURCES = vcalconduitSettings.kcfgc \
#	korganizerConduit.ui \
#	vcal-factorybase.cc \
#	vcal-setupbase.cc \
#	vcal-conduitbase.cc
#
#conduit_vcal_la_SOURCES = vcal-conduit.cc vcal-factory.cc vcal-setup.cc
#conduit_vcal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_vcal_la_LIBADD = ../../lib/libkpilot.la \
#	../../../libkcal/libkcal.la \
#	libvcalconduit_shared.la
#conduit_vcal_la_COMPILE_FIRST = vcalconduitSettings.h korganizerConduit.h
#
#conduit_todo_la_SOURCES = todo-factory.cc todo-setup.cc todo-conduit.cc
#conduit_todo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_todo_la_LIBADD = ../../lib/libkpilot.la \
#	../../../libkcal/libkcal.la \
#	libvcalconduit_shared.la
#conduit_todo_la_COMPILE_FIRST = vcalconduitSettings.h korganizerConduit.h
#
#
#kde_kcfg_DATA = vcalconduitbase.kcfg
#
