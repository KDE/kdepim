
lib_LTLIBRARIES = libkgantt.la
libkgantt_la_SOURCES = KGantt.cpp xQGanttBarViewPort.cpp  \
xQGanttListViewPort.cpp \
KGanttItem.cpp KGanttRelation.cpp \
xQGanttBarViewPort_Events.cpp \
xQGanttBarView.cpp  xQGanttListView.cpp KGanttBarConfig.cpp

libkgantt_la_LDFLAGS = -version-info 0:2:0 $(all_libraries) -no-undefined
libkgantt_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) 

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

kganttincludedir = $(includedir)/kgantt
kganttinclude_HEADERS = KGantt.h \
KGanttItem.h  KGanttRelation.h \
xQGanttBarView.h  \
xQGanttBarViewPort.h  xQGanttListView.h  xQGanttListViewPort.h KGanttBarConfig.h

messages:  rc.cpp
	 $(XGETTEXT) *.cpp -o $(podir)/kgantt.pot
