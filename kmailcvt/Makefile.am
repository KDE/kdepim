
bin_PROGRAMS = kmailcvt
kmailcvt_SOURCES = oe4_2mbox.cpp liboe.cxx filter_oe4.cxx oe5_2mbox.cpp pablib.cxx mapihd.cpp filters.cxx filter_pab.cxx filter_oe5.cxx kmailcvt.cpp main.cpp filter_eudora_ab.cxx filter_ldif.cxx
kmailcvt_LDADD   = -lkab $(LIB_KFILE)

EXTRA_DIST = main.cpp kmailcvt.cpp kmailcvt.h kmailcvt.desktop kmailcvt.xpm mini-kmailcvt.xpm filter_oe5.cxx filter_oe5.hxx filter_eudora_ab.cxx filter_eudora_ab.hxx filter_pab.cxx filter_pab.hxx filters.cxx filters.hxx harray.hxx mapihd.cpp mapihd.h pablib.cxx pablib.hxx pabtypes.h liboe.h oe5_2mbox.cpp oe5_2mbox.h filter_oe4.cxx filter_oe4.hxx liboe.cxx oe4_2mbox.cpp oe4_2mbox.h PAB_format.html sv9429424.gif filter_ldif.hxx filter_ldif.cxx

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kmailcvt.desktop

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

kmailcvt_METASOURCES = AUTO

# the library search path. 
kmailcvt_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON=AUTO

messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmailcvt.pot; \
	fi

