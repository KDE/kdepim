KDE_ICON = abbrowser

KDE_CXXFLAGS = $(USE_RTTI)

APPSDIR = $(kde_appsdir)/Utilities

INCLUDES= -I$(top_srcdir)/kab2/lib $(all_includes)

SUBDIRS = doc pics

SRCDOC_DEST=$(kde_htmldir)/en/abbrowser

bin_PROGRAMS = abbrowser

abbrowser_SOURCES = abbrowser.cpp undo.cpp attributes.cpp selectfields.cpp viewoptions.cpp namevaluewidget.cpp browserentryeditor.cpp entryeditorwidget.cpp datepickerdialog.cpp browserwidget.cpp main.cpp

abbrowser_METASOURCES = AUTO

abbrowser_LDFLAGS = -L$(top_builddir)/kab2/lib $(all_libraries) $(KDE_RPATH)

abbrowser_LDADD = -lkab2 -lkimgio $(LIB_KFILE)

noinst_HEADERS = abbrowser.h undo.h attributes.h selectfields.h viewoptions.h namevaluewidget.h browserentryeditor.h entryeditorwidget.h datepickerdialog.h browserwidget.h

messages:
	$(XGETTEXT) $(abbrowser_SOURCES) -o $(podir)/abbrowser.pot

srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) abbrowser $(noinst_HEADERS) -lqt

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/abbrowser.desktop $(APPSDIR)

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/abbrowser.desktop

