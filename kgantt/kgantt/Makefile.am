####### kdevelop will overwrite this part!!! (begin)##########
# bin_PROGRAMS = kgantt
# kgantt_SOURCES = 
# kgantt_LDADD   =  -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)

lib_LTLIBRARIES = libkgantt.la
libkgantt_la_SOURCES = KGantt.cpp xQGanttBarViewPort.cpp  \
xQGanttListViewPort.cpp \
KGanttItem.cpp KGanttRelation.cpp \
xQGanttBarViewPort_Events.cpp \
xQGanttBarView.cpp  xQGanttListView.cpp KGanttBarConfig.cpp

libkgantt_la_LDFLAGS = -version-info 0:2:0 $(all_libraries) -no-undefined
libkgantt_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) 


####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO


kganttincludedir = $(includedir)/kgantt
kganttinclude_HEADERS = KGantt.h \
KGanttItem.h  KGanttRelation.h \
xQGanttBarView.h  \
xQGanttBarViewPort.h  xQGanttListView.h  xQGanttListViewPort.h KGanttBarConfig.h

# the library search path. 
#kgantt_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE´s XML GUI builing
#rcdir = $(kde_datadir)/kgantt
#rc_DATA = kganttui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages:  rc.cpp
	: > rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kgantt.pot; \
	fi

