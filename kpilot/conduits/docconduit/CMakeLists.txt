kde4_header()

add_subdirectory( Icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(conduit_doc_PART_SRCS 
   doc-factory.cc 
   doc-setup.cc 
   doc-conduit.cc 
   doc-conflictdialog.cc )

kde4_automoc(${conduit_doc_PART_SRCS})

kde4_add_ui_files(conduit_doc_PART_SRCS doc-setupdialog.ui )

kde4_add_kcfg_files(conduit_doc_PART_SRCS docconduitSettings.kcfgc )

kde4_add_plugin(conduit_doc ${conduit_doc_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_doc )

target_link_libraries(conduit_doc  ${QT_AND_KDECORE_LIBS} kpilot palmdoc_shared kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_doc )


########### next target ###############

set(kpalmdoc_SRCS kpalmdoc_dlg.cc kpalmdoc.cpp )

kde4_automoc(${kpalmdoc_SRCS})

kde4_add_ui_files(kpalmdoc_SRCS kpalmdoc_dlgbase.ui )

kde4_add_kcfg_files(kpalmdoc_SRCS kpalmdocSettings.kcfgc )

kde4_add_executable(kpalmdoc ${kpalmdoc_SRCS})

target_link_libraries(kpalmdoc  ${QT_AND_KDECORE_LIBS} kpilot palmdoc_shared )

install_targets(/bin kpalmdoc )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kpalmdoc.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES docconduit.kcfg kpalmdoc.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES doc_conduit.desktop )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES  )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kpalmdoc.upd )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile for KPilot's doc  conduit
####
#### The doc conduit is Copyright (C) 2002 by Reinhold Kainhofer
#### the files makedoc9.{h,cpp} are also Copyright (C) 2000 by Pat Beirne
#
#SUBDIRS = Icons
#
#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#METASOURCES = AUTO
#
#
#servicedir = $(kde_servicesdir)
#service_DATA = doc_conduit.desktop
#
#noinst_LTLIBRARIES = libpalmdoc_shared.la
#kde_module_LTLIBRARIES = conduit_doc.la
#bin_PROGRAMS = kpalmdoc
#
#libpalmdoc_shared_la_SOURCES = makedoc9.cc pilotDOCHead.cc pilotDOCEntry.cc pilotDOCBookmark.cc DOC-converter.cc
#
#conduit_doc_la_SOURCES = docconduitSettings.kcfgc doc-factory.cc doc-setup.cc doc-conduit.cc doc-setupdialog.ui doc-conflictdialog.cc
#conduit_doc_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_doc_la_LIBADD = ../../lib/libkpilot.la libpalmdoc_shared.la \
#	$(LIB_KDEUI) $(LIB_KFILE)
#
###libpalmdoc_shared.la
#
#kpalmdoc_SOURCES = kpalmdocSettings.kcfgc kpalmdoc_dlgbase.ui kpalmdoc_dlg.cc kpalmdoc.cpp
#kpalmdoc_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#kpalmdoc_LDADD = $(LIB_KFILE) $(PISOCK_LIB) ../../lib/libkpilot.la libpalmdoc_shared.la
#
#xdg_apps_DATA = kpalmdoc.desktop
#servicetypedir = $(kde_servicetypesdir)
#kde_kcfg_DATA = docconduit.kcfg kpalmdoc.kcfg
#
#update_DATA = kpalmdoc.upd
#updatedir = $(kde_datadir)/kconf_update
#
