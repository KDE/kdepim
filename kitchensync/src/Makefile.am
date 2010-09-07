SUBDIRS = about pics

INCLUDES = -I$(top_srcdir)/kitchensync/src \
           -I$(top_srcdir)/kitchensync \
           -I$(top_srcdir)/libkdepim \
           -I$(top_srcdir) \
            $(all_includes)

# Application
bin_PROGRAMS = kitchensync

kitchensync_SOURCES = main.cpp mainwindow.cpp
kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kitchensync_LDADD = libkitchensync.la

xdg_apps_DATA = kitchensync.desktop

# Library
lib_LTLIBRARIES = libkitchensync.la

libkitchensync_la_SOURCES = aboutpage.cpp mainwidget.cpp groupconfigdialog.cpp \
                            groupconfig.cpp groupitem.cpp groupview.cpp memberconfig.cpp \
                            syncprocess.cpp syncprocessmanager.cpp connectionwidgets.cpp \
                            pluginpicker.cpp configgui.cpp configguiblank.cpp configguifile.cpp \
                            memberinfo.cpp groupconfigcommon.cpp kwidgetlist.cpp \
                            configguipalm.cpp conflictdialog.cpp singleconflictdialog.cpp \
                            addresseediffalgo.cpp calendardiffalgo.cpp xmldiffalgo.cpp \
                            htmldiffalgodisplay.cpp genericdiffalgo.cpp multiconflictdialog.cpp \
                            configguiirmc.cpp \
                            configguisyncmlobex.cpp configguisyncmlhttp.cpp configguiopie.cpp  \
                            configguignokii.cpp configguigcalendar.cpp configguildap.cpp configguigpe.cpp \
                            configguijescs.cpp configguievo2.cpp configguimoto.cpp configguisynce.cpp \
                            configguisunbird.cpp
libkitchensync_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libkitchensync_la_LIBADD = $(LIB_KIO) $(LIB_KHTML) $(top_builddir)/kitchensync/libqopensync/libqopensync.la \
                           $(LIB_KABC) $(top_builddir)/libkdepim/libkdepim.la $(top_builddir)/libkcal/libkcal.la

# KPart
kde_module_LTLIBRARIES = libkitchensyncpart.la

libkitchensyncpart_la_SOURCES = part.cpp
libkitchensyncpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkitchensyncpart_la_LIBADD = $(LIB_KPARTS) libkitchensync.la

# Resources
kitchensyncdatadir = $(kde_datadir)/kitchensync
kitchensyncdata_DATA = kitchensyncui.rc kitchensync_part.rc

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kitchensync.pot
