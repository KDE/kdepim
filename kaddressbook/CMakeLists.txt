kde4_header()

set(libkabcommon_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/common/locationmap.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/common/filter.cpp
	)
#kde4_add_kcfg_files(libkabcommon_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )

set(libkabinterfaces_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces/configurewidget.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces/contacteditorwidget.cpp
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces/core.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces/extensionwidget.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces/xxport.cpp
)


set(libprinter_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/detailledstyle.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/kabentrypainter.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/mikesstyle.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/ringbinderstyle.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/printingwizard.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/printprogress.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/printstyle.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/selectionpage.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/stylepage.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/printsortmode.cpp
)
kde4_add_ui3_files(libprinter_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/ds_appearance.ui
	${CMAKE_SOURCE_DIR}/kaddressbook/printing/rbs_appearance.ui
)

set(libkabim_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/editors/imeditorwidget.cpp 
	${CMAKE_SOURCE_DIR}/kaddressbook/editors/imaddresswidget.cpp
)

kde4_add_ui3_files(libkabim_SRCS
	${CMAKE_SOURCE_DIR}/kaddressbook/editors/imeditorbase.ui
    ${CMAKE_SOURCE_DIR}/kaddressbook/editors/imaddressbase.ui
)

add_subdirectory( common )
add_subdirectory( interfaces ) 
add_subdirectory( printing ) 
add_subdirectory( editors ) 
add_subdirectory( views ) 
add_subdirectory( features ) 
add_subdirectory( kcmconfigs ) 
add_subdirectory( xxport ) 
add_subdirectory( pics ) 
add_subdirectory( csv-templates ) 
add_subdirectory( thumbnailcreator ) 

include_directories( 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces 
	${CMAKE_SOURCE_DIR}/kaddressbook/features 
	${CMAKE_SOURCE_DIR}/kaddressbook/views 
	${CMAKE_SOURCE_DIR}/kaddressbook/printing 
	${CMAKE_SOURCE_DIR}/kaddressbook/common 
	${CMAKE_SOURCE_DIR}/kaddressbook/editors 
	${CMAKE_SOURCE_DIR}/akregator/src 
	${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kaddressbook_bin_SRCS ${libkabcommon_SRCS}
	main.cpp 
	kaddressbookmain.cpp 
	)

kde4_automoc(${kaddressbook_bin_SRCS})

kde4_add_dcop_skels(kaddressbook_bin_SRCS kaddressbookiface.h )

kde4_add_executable(kaddressbook_bin ${kaddressbook_bin_SRCS})
set_target_properties(kaddressbook_bin PROPERTIES OUTPUT_NAME kaddressbook)

target_link_libraries(kaddressbook_bin  ${KDE4_KDECORE_LIBS} kaddressbook printer kabinterfaces kdepim kcal kabc_file khtml kabim rsslocal kdeprint )

install_targets(/bin kaddressbook_bin )


########### next target ###############

set(kaddressbookpart_PART_SRCS kaddressbook_part.cpp )

kde4_automoc(${kaddressbookpart_PART_SRCS})

kde4_add_plugin(kaddressbookpart WITH_PREFIX ${kaddressbookpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddressbookpart )

target_link_libraries(kaddressbookpart  ${KDE4_KDECORE_LIBS} kaddressbook )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddressbookpart )


########### next target ###############

set(kaddressbook_LIB_SRCS ${libkabinterfaces_SRCS} ${libprinter_SRCS} ${libkabim_SRCS}
	addresseeconfig.cpp
   addresseeeditordialog.cpp
   addresseeeditorwidget.cpp
   addresseeeditorbase.cpp
   addresseeeditorextension.cpp
   addresseeutil.cpp
   addresseditwidget.cpp
   addviewdialog.cpp
   emaileditwidget.cpp
   extensionmanager.cpp
   filtereditdialog.cpp
   filterselectionwidget.cpp
   geowidget.cpp
   imagewidget.cpp
   incsearchwidget.cpp
   jumpbuttonbar.cpp
   kabcore.cpp
   kablock.cpp
   kaddressbookservice.cpp
   kaddressbookview.cpp
   keywidget.cpp
   ldapsearchdialog.cpp
   nameeditdialog.cpp
   phoneeditwidget.cpp
   secrecywidget.cpp
   soundwidget.cpp
   viewconfigurefieldspage.cpp
   viewconfigurefilterpage.cpp
   viewconfigurewidget.cpp
   undocmds.cpp
   viewmanager.cpp
   xxportmanager.cpp
   xxportselectdialog.cpp
   contacteditorwidgetmanager.cpp
   simpleaddresseeeditor.cpp
   advancedcustomfields.cpp
   customfieldswidget.cpp
   freebusywidget.cpp
   searchmanager.cpp
   imeditwidget.cpp
   kabtools.cpp 
)

kde4_automoc(${kaddressbook_LIB_SRCS})

kde4_add_library(kaddressbook SHARED ${kaddressbook_LIB_SRCS})

target_link_libraries(kaddressbook  ${KDE4_KDECORE_LIBS} )

set_target_properties(kaddressbook PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kaddressbook )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kaddressbook.desktop )
install_files( ${DATA_INSTALL_DIR}/kaddressbook FILES zone.tab )
install_files( ${DATA_INSTALL_DIR}/kaddressbook FILES kaddressbookui.rc kaddressbook_part.rc )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kaddressbook_view.desktop dcopaddressbook.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = interfaces common printing editors . views features kcmconfigs xxport pics csv-templates \
#          thumbnailcreator
#SRCDOC_DEST = $(kde_htmldir)/en/kaddressbook
#
#
#INCLUDES= -I$(top_srcdir)/kaddressbook/interfaces \
#          -I$(top_srcdir)/kaddressbook/features \
#          -I$(top_srcdir)/kaddressbook/views \
#          -I$(top_srcdir)/kaddressbook/printing \
#          -I$(top_srcdir)/kaddressbook/common \
#          -I$(top_srcdir)/kaddressbook/editors \
#          -I$(top_builddir)/kaddressbook/common \
#          -I$(top_srcdir) \
#          -I$(top_buildir)/kaddressbook/editors \
#          -I$(top_srcdir)/akregator/src \
#          $(all_includes)
#
#bin_PROGRAMS = kaddressbook
#
#kaddressbook_SOURCES = main.cpp kaddressbookmain.cpp
#kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kaddressbook_LDADD = libkaddressbook.la
#
#kde_module_LTLIBRARIES = libkaddressbookpart.la
#
#libkaddressbookpart_la_SOURCES = kaddressbook_part.cpp
#libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddressbookpart_la_LIBADD = libkaddressbook.la
#
#lib_LTLIBRARIES = libkaddressbook.la
#libkaddressbook_la_SOURCES = addresseeconfig.cpp addresseeeditordialog.cpp \
#    addresseeeditorwidget.cpp addresseeeditorbase.cpp addresseeeditorextension.cpp addresseeutil.cpp addresseditwidget.cpp \
#    addviewdialog.cpp emaileditwidget.cpp extensionmanager.cpp \
#    filtereditdialog.cpp filterselectionwidget.cpp geowidget.cpp \
#    imagewidget.cpp incsearchwidget.cpp jumpbuttonbar.cpp kabcore.cpp kablock.cpp \
#    kaddressbookiface.skel kaddressbookservice.cpp \
#    kaddressbookview.cpp keywidget.cpp ldapsearchdialog.cpp nameeditdialog.cpp \
#    phoneeditwidget.cpp secrecywidget.cpp soundwidget.cpp \
#    viewconfigurefieldspage.cpp viewconfigurefilterpage.cpp \
#    viewconfigurewidget.cpp undocmds.cpp viewmanager.cpp \
#    xxportmanager.cpp xxportselectdialog.cpp contacteditorwidgetmanager.cpp \
#    simpleaddresseeeditor.cpp advancedcustomfields.cpp customfieldswidget.cpp \
#    freebusywidget.cpp searchmanager.cpp imeditwidget.cpp kabtools.cpp
#libkaddressbook_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libkaddressbook_la_LIBADD = $(top_builddir)/kaddressbook/printing/libprinter.la \
#                            $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#                            $(top_builddir)/libkdepim/libkdepim.la \
#                            $(top_builddir)/libkcal/libkcal.la \
#                            -lkabc_file $(LIB_KABC) $(LIB_KHTML) \
#                            $(top_builddir)/kaddressbook/common/libkabcommon.la \
#                            $(top_builddir)/kaddressbook/editors/libkabim.la \
#                            ../akregator/src/librss/librsslocal.la \
#                            -lkdeprint
#libkaddressbook_la_COMPILE_FIRST = $(top_srcdir)/libkdepim/categoryselectdialog.h \
#                                   $(top_srcdir)/libkdepim/categoryeditdialog.h \
#                                   $(top_builddir)/kaddressbook/common/kabprefs_base.h
#
#
#noinst_HEADERS = \
#    addresseditwidget.h addresseeconfig.h \
#    addresseeeditordialog.h addresseeeditorwidget.h addresseeutil.h \
#    addviewdialog.h emaileditwidget.h extensionmanager.h \
#    filtereditdialog.h filterselectionwidget.h geowidget.h imagewidget.h \
#    incsearchwidget.h jumpbuttonbar.h kablock.h kaddressbook_part.h \
#    kaddressbookservice.h kaddressbookview.h keywidget.h \
#    ldapsearchdialog.h nameeditdialog.h phoneeditwidget.h secrecywidget.h \
#    soundwidget.h typecombo.h undocmds.h viewconfigurefieldspage.h \
#    viewconfigurefilterpage.h viewconfigurewidget.h viewmanager.h xxportmanager.h \
#    xxportselectdialog.h advancedcustomfields.h imeditwidget.h
#
#kaddressbook_METASOURCES = AUTO
#
#rcdir = $(kde_datadir)/kaddressbook
#rc_DATA = kaddressbookui.rc kaddressbook_part.rc
#
#kabdatadir = $(kde_datadir)/kaddressbook
#kabdata_DATA = zone.tab
#
#xdg_apps_DATA = kaddressbook.desktop
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kaddressbook_view.desktop dcopaddressbook.desktop
#
#KDE_ICON = kaddressbook
#
#messages: rc.cpp
#	$(EXTRACTRC) */*.ui */*.kcfg >> rc.cpp
#	$(XGETTEXT) *.h *.cpp common/*.cpp editors/*.cpp features/*.cpp printing/*.cpp views/*.cpp xxport/*.cpp interfaces/*.h -o $(podir)/kaddressbook.pot
#
#srcdoc:
#	$(mkinstalldirs) $(SRCDOC_DEST)
#	kdoc -H -d $(SRCDOC_DEST) kaddressbook $(noinst_HEADERS) -lqt
#
#include $(top_srcdir)/admin/Doxyfile.am
