SUBDIRS = pics

INCLUDES = -I$(top_srcdir)/kitchensync/libksync \
           -I$(top_srcdir)/kitchensync/libkonnector2 \
           -I$(top_srcdir)/kitchensync/multisynk \
           -I$(top_srcdir) \
            $(all_includes)

# Application
bin_PROGRAMS = multisynk

multisynk_SOURCES = main.cpp multisynk_main.cpp
multisynk_LDFLAGS = $(KDE_RPATH) $(all_libraries)
multisynk_LDADD = libmultisynk.la

xdg_apps_DATA = multisynk.desktop

# Library
lib_LTLIBRARIES = libmultisynk.la

libmultisynk_la_SOURCES = engine.cpp konnectormanager.cpp konnectorpair.cpp \
                          konnectorpairmanager.cpp konnectorpairview.cpp \
                          logdialog.cpp mainwidget.cpp paireditordialog.cpp \
                          paireditorwidget.cpp plugineditorwidget.cpp
libmultisynk_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libmultisynk_la_LIBADD = $(top_builddir)/kitchensync/libkonnector2/libkonnector.la \
                         $(top_builddir)/kitchensync/libksync/libksync2.la
# KPart
kde_module_LTLIBRARIES = libmultisynkpart.la

libmultisynkpart_la_SOURCES = multisynk_part.cpp
libmultisynkpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libmultisynkpart_la_LIBADD = libmultisynk.la

# Resources
multisynkdatadir = $(kde_datadir)/multisynk
multisynkdata_DATA = multisynkui.rc multisynk_part.rc

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/multisynk.pot
