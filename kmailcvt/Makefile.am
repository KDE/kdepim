####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kmailcvt
kmailcvt_SOURCES = oe4_2mbox.cpp liboe.cxx filter_oe4.cxx oe5_2mbox.cpp pablib.cxx mapihd.cpp filters.cxx filter_pab.cxx filter_oe5.cxx kmailcvt.cpp main.cpp 
kmailcvt_LDADD   = -lkab -lkfile -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)


EXTRA_DIST = main.cpp kmailcvt.cpp kmailcvt.h kmailcvt.desktop kmailcvt.xpm mini-kmailcvt.xpm filter_oe5.cxx filter_oe5.hxx filter_pab.cxx filter_pab.hxx filters.cxx filters.hxx harray.hxx mapihd.cpp mapihd.h pablib.cxx pablib.hxx pabtypes.h liboe.h oe5_2mbox.cpp oe5_2mbox.h filter_oe4.cxx filter_oe4.hxx liboe.cxx oe4_2mbox.cpp oe4_2mbox.h PAB_format.html sv9429424.gif 

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kmailcvt.desktop

#install-data-local:
#	$(mkinstalldirs) $(kde_appsdir)/Applications/
#	$(INSTALL_DATA) kmailcvt.desktop $(kde_appsdir)/Applications/kmailcvt.desktop
#	$(mkinstalldirs) $(kde_icondir)/medium/locolor/apps/
#	$(INSTALL_DATA) kmailcvt.xpm $(kde_icondir)/medium/locolor/apps/kmailcvt.xpm
#	$(mkinstalldirs) $(kde_icondir)/small/locolor/apps/
#	$(INSTALL_DATA) mini-kmailcvt.xpm $(kde_icondir)/small/locolor/apps/kmailcvt.xpm

#uninstall-local:
#	-rm -f $(kde_appsdir)/Applications/kmailcvt.desktop
#	-rm -f $(kde_icondir)/medium/locolor/apps/kmailcvt.xpm
#	-rm -f $(kde_icondir)/small/locolor/apps/kmailcvt.xpm

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

kmailcvt_METASOURCES = AUTO

# the library search path. 
kmailcvt_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(kmailcvt_METASOURCES)

KDE_ICON=AUTO

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) -C -ki18n -x $(kde_includes)/kde.pot $$LIST -o ../po/kmailcvt.pot; \
	fi

