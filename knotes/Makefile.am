INCLUDES        = -I$(top_srcdir) $(all_includes)

SUBDIRS         = . pics icons

bin_PROGRAMS    = knotes

include_HEADERS = KNotesIface.h
noinst_HEADERS  = knotesapp.h knote.h knoteedit.h knotebutton.h knoteconfigdlg.h \
						knoteslegacy.h
knotes_SOURCES  = knotesapp.cpp knote.cpp knoteedit.cpp knotebutton.cpp \
					   knoteconfigdlg.cpp knoteconfig.kcfgc KNotesIface.skel \
					   knoteslegacy.cpp main.cpp
knotes_LDADD    = $(LIB_KIO) -lkdeprint $(top_builddir)/libkcal/libkcal.la
knotes_LDFLAGS  = $(all_libraries) $(KDE_RPATH)

METASOURCES     = AUTO

xdg_apps_DATA   = knotes.desktop
kde_kcfg_DATA   = knotes.kcfg

ui_DATA         = knotesui.rc
uidir           = $(kde_datadir)/knotes

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/knotes.pot

include ../admin/Doxyfile.am
