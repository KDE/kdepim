
bin_PROGRAMS = kmailcvt
kmailcvt_SOURCES = kimportpagedlg.ui kimportpage.cpp kselfilterpagedlg.ui \
                   kselfilterpage.cpp pablib.cxx mapihd.cpp \
                   filters.cxx filter_pab.cxx filter_oe.cxx kmailcvt.cpp \
                   main.cpp filter_eudora_ab.cxx filter_ldif.cxx \
                   filter_pmail.cxx filter_plain.cxx filter_csv.cxx
kmailcvt_LDADD   = -lkabc $(LIB_KFILE)

EXTRA_DIST = main.cpp kmailcvt.cpp kimportpage.cpp kimportpage.h \
             kselfilterpage.cpp kselfilterpage.h kmailcvt.h \
             kmailcvt.desktop filter_oe.cxx filter_oe.hxx \
             filter_eudora_ab.cxx filter_eudora_ab.hxx filter_pab.cxx \
             filter_pab.hxx filters.cxx filters.hxx mapihd.cpp mapihd.h \
             pablib.cxx pablib.hxx pabtypes.h \
             PAB_format.html sv9429424.gif filter_ldif.hxx \
             filter_ldif.cxx filter_pmail.cxx filter_pmail.hxx \
             filter_plain.cxx filter_plain.hxx \
             filter_csv.cxx filter_csv.hxx

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/kmailcvt.desktop

SUBDIRS = pics

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

