# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_sounddir      Where system sounds should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# this is where the kdelnk file will go 
APPSDIR = $(kde_appsdir)/Utilities

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I../kab2/lib

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = doc pics

# this is where the source documentation will be installed
SRCDOC_DEST=$(kde_htmldir)/en/abbrowser

####### This part is very abbrowser specific
# this is the program that gets installed
bin_PROGRAMS = 	abbrowser

# Which sources should be compiled for abbrowser.
abbrowser_SOURCES = abbrowser.cpp undo.cpp attributes.cpp selectfields.cpp viewoptions.cpp namevaluewidget.cpp browserentryeditor.cpp entryeditorwidget.cpp datepickerdialog.cpp browserwidget.cpp main.cpp

# let automoc handle all of the meta source files (moc)
abbrowser_METASOURCES = AUTO

# the library search path. 
abbrowser_LDFLAGS = -L../kab2/lib $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
abbrowser_LDADD = -lkab2 -lkimgio $(LIB_KFILE)

# these are the headers for your project
noinst_HEADERS = abbrowser.h undo.h attributes.h selectfields.h viewoptions.h namevaluewidget.h browserentryeditor.h entryeditorwidget.h datepickerdialog.h browserwidget.h

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(abbrowser_METASOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(abbrowser_SOURCES) && mv messages.po ../po/abbrowser.pot

# generate source documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) abbrowser 	     $(noinst_HEADERS) -lqt

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) abbrowser.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) abbrowser.png $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-abbrowser.png $(kde_minidir)/abbrowser.png

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/abbrowser.desktop
	-rm -f $(kde_icondir)/abbrowser.png
	-rm -f $(kde_minidir)/abbrowser.png
