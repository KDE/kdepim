
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

# just set the variable
APPSDIR = $(kde_appsdir)/Utilities
# set the include path for X, qt and KDE
INCLUDES= @all_includes@ -I../../kpilot -I../vcalconduit/versit -I../../kpilot/libpilot/include
# claim, which subdirectories you want to install
SUBDIRS = 

####### This part is very kpilot specific
# you can add here more. This one gets installed 
noinst_PROGRAMS = todoconduit

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for todoconduit
todoconduit_SOURCES= todo-conduit.cc todo-setup.cc

# the library search path
todoconduit_LDFLAGS = @all_libraries@ -L`pwd`/../vcalconduit/versit -L`pwd`/../../kpilot -L/usr/local/lib

METASOURCES = AUTO

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
todoconduit_LDADD   = ../../kpilot/pilotTodoEntry.o -lconduit \
	-lkfile -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext @LIBSOCKET@ -lversit -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = todo-conduit.h todo-setup.h

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/kpilot
	$(mkinstalldirs) $(kde_datadir)/kpilot/conduits
	$(INSTALL_PROGRAM) todoconduit $(kde_datadir)/kpilot/conduits

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(kde_datadir)/kpilot/conduits/todoconduit

