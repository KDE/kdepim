
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

####### This part is very kpilot specific
# you can add here more. This one gets installed 
noinst_LIBRARIES = libversit.a

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for kpilot
libversit_a_SOURCES = vcc.c vobject.c

noinst_HEADERS = port.h vcc.h vobject.h

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 
$(srcdir)/vcc.c: vcc.y
	bison $(srcdir)/vcc.y -o $(srcdir)/vcc.c
