# Makefile.am for null-conduit
#
# Copyright (C) 1998,1999 Dan Pilone
#
# This file is distributed under the Gnu General Public Licence (GPL).
# The GPL should have been included with this file in a file called
# COPYING. 



# REVISION HISTORY 
#
# 3.1b12	By Adriaan de Groot
#

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
INCLUDES= @all_includes@ -I../../kpilot -I../../kpilot/libpilot/include
# claim, which subdirectories you want to install
#SUBDIRS = 

####### This part is very kpilot specific
# you can add here more. This one gets installed 
noinst_PROGRAMS = null_conduit

# KDE magic file
MAGIC=$(kde_mimedir)/magic

METASOURCES = AUTO

# Which sources should be compiled for vcalconduit
null_conduit_SOURCES= null-conduit.cc setupDialog.cc

# the library search path
null_conduit_LDFLAGS = $(all_libraries) -L$(srcdir)/../../kpilot

null_conduit_LDADD   =  -lconduit $(LIB_KDEUI) -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = null-conduit.h setupDialog.h

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/kpilot/conduits
	$(INSTALL_PROGRAM) popmail_conduit $(kde_datadir)/kpilot/conduits

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(kde_datadir)/kpilot/conduits/popmail_conduit

