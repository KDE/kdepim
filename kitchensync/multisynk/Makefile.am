SUBDIRS = pics

INCLUDES = -I$(top_srcdir)/kitchensync/libksync \
           -I$(top_srcdir)/kitchensync/libkonnector2 \
           -I$(top_srcdir)/kitchensync/multsynk \
           -I$(top_srcdir) \
            $(all_includes)

multisynkdatadir = $(kde_datadir)/multisynk
multisynkdata_DATA = multisynkui.rc

METASOURCES = AUTO

bin_PROGRAMS = multisynk

multisynk_SOURCES = engine.cpp konnectormanager.cpp konnectorpair.cpp \
                    konnectorpairmanager.cpp konnectorpairview.cpp \
                    logdialog.cpp main.cpp mainwindow.cpp paireditordialog.cpp \
                    paireditorwidget.cpp plugineditorwidget.cpp
multisynk_LDADD = ../libkonnector2/libkonnector.la \
                  ../libksync/libksync2.la
multisynk_LDFLAGS = $(KDE_RPATH) $(all_libraries)
