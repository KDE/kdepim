ADD_SUBDIRECTORY( about ) 
ADD_SUBDIRECTORY( pics ) 

INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} ${OSENGINE_INCLUDE_DIRS} )


########### next target ###############

SET(kitchensync_SRCS
main.cpp
mainwindow.cpp
aboutpage.cpp
mainwidget.cpp
groupconfigdialog.cpp
groupconfig.cpp
groupitem.cpp
groupview.cpp
memberconfig.cpp
syncprocess.cpp
syncprocessmanager.cpp
connectionwidgets.cpp
pluginpicker.cpp
configgui.cpp
configguiblank.cpp
configguifile.cpp
memberinfo.cpp
groupconfigcommon.cpp
kwidgetlist.cpp
configguipalm.cpp
conflictdialog.cpp
singleconflictdialog.cpp
addresseediffalgo.cpp
calendardiffalgo.cpp
htmldiffalgodisplay.cpp
genericdiffalgo.cpp
multiconflictdialog.cpp
configguiirmc.cpp
ksplashwidget.cpp
configguisyncmlobex.cpp
configguisyncmlhttp.cpp
configguiopie.cpp
configguignokii.cpp
configguigcalendar.cpp
configguildap.cpp
configguigpe.cpp
)

KDE4_AUTOMOC(${kitchensync_SRCS})

KDE4_ADD_EXECUTABLE(kitchensync ${kitchensync_SRCS})

TARGET_LINK_LIBRARIES(kitchensync ${QT_AND_KDECORE_LIBS} kio khtml qopensync kdepim kcal )

INSTALL(TARGETS kitchensync DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(kitchensyncprivate_LIB_SRCS
dummy.cpp
)

KDE4_AUTOMOC(${kitchensyncprivate_LIB_SRCS})

KDE4_ADD_LIBRARY(kitchensyncprivate SHARED ${kitchensyncprivate_LIB_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncprivate  ${QT_AND_KDECORE_LIBS} )

SET_TARGET_PROPERTIES(kitchensyncprivate PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS kitchensyncprivate DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

SET(kitchensyncpart_PART_SRCS
part.cpp
)

KDE4_AUTOMOC(${kitchensyncpart_PART_SRCS})

KDE4_ADD_PLUGIN(kitchensyncpart WITH_PREFIX ${kitchensyncpart_PART_SRCS})

TARGET_LINK_LIBRARIES(kitchensyncpart ${QT_AND_KDECORE_LIBS} kparts )

INSTALL(TARGETS kitchensyncpart DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

INSTALL( FILES kitchensync.desktop DESTINATION ${XDG_APPS_DIR} )
INSTALL( FILES kitchensyncui.rc kitchensync_part.rc DESTINATION ${DATA_INSTALL_DIR}/kitchensync )



#original Makefile.am contents follow:

#SUBDIRS = about pics
#
#INCLUDES = -I$(top_srcdir)/kitchensync/src \
#           -I$(top_srcdir)/kitchensync \
#           -I$(top_srcdir)/libkdepim \
#           -I$(top_srcdir) \
#            $(all_includes)
#
## Application
#bin_PROGRAMS = kitchensync
#
#kitchensync_SOURCES = main.cpp mainwindow.cpp
#kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kitchensync_LDADD = libkitchensync.la
#
#xdg_apps_DATA = kitchensync.desktop
#
## Library
#lib_LTLIBRARIES = libkitchensync.la
#
#libkitchensync_la_SOURCES = aboutpage.cpp mainwidget.cpp groupconfigdialog.cpp \
#                            groupconfig.cpp groupitem.cpp groupview.cpp memberconfig.cpp \
#                            syncprocess.cpp syncprocessmanager.cpp connectionwidgets.cpp \
#                            pluginpicker.cpp configgui.cpp configguiblank.cpp configguifile.cpp \
#                            memberinfo.cpp groupconfigcommon.cpp kwidgetlist.cpp \
#                            configguipalm.cpp conflictdialog.cpp singleconflictdialog.cpp \
#                            addresseediffalgo.cpp calendardiffalgo.cpp \
#                            htmldiffalgodisplay.cpp genericdiffalgo.cpp multiconflictdialog.cpp \
#                            configguiirmc.cpp ksplashwidget.cpp \
#                            configguisyncmlobex.cpp configguisyncmlhttp.cpp configguiopie.cpp  \
#                            configguignokii.cpp configguigcalendar.cpp configguildap.cpp configguigpe.cpp
#libkitchensync_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libkitchensync_la_LIBADD = $(LIB_KIO) $(LIB_KHTML) $(top_builddir)/kitchensync/libqopensync/libqopensync.la \
#                           $(LIB_KABC) $(top_builddir)/libkdepim/libkdepim.la -lkcal
#
## KPart
#kde_module_LTLIBRARIES = libkitchensyncpart.la
#
#libkitchensyncpart_la_SOURCES = part.cpp
#libkitchensyncpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkitchensyncpart_la_LIBADD = $(LIB_KPARTS) libkitchensync.la
#
## Resources
#kitchensyncdatadir = $(kde_datadir)/kitchensync
#kitchensyncdata_DATA = kitchensyncui.rc kitchensync_part.rc
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kitchensync.pot
