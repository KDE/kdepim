
bin_PROGRAMS = kmailcvt
kmailcvt_SOURCES = kimportpagedlg.ui kimportpage.cpp kselfilterpagedlg.ui \
                   kselfilterpage.cpp filters.cxx filter_oe.cxx kmailcvt.cpp \
                   main.cpp filter_pmail.cxx filter_plain.cxx \
                   filter_mbox.cxx
kmailcvt_LDADD   = $(LIB_KFILE)
kmailcvt_COMPILE_FIRST = kimportpagedlg.ui kselfilterpagedlg.ui

kimportpage.cpp : kimportpagedlg.h

kselfilterpage.cpp : kselfilterpagedlg.h

EXTRA_DIST = main.cpp kmailcvt.cpp kimportpage.cpp kimportpage.h \
             kselfilterpage.cpp kselfilterpage.h kmailcvt.h \
             kmailcvt.desktop filter_oe.cxx filter_oe.hxx \
             filters.cxx filters.hxx \
             PAB_format.html sv9429424.gif \
             filter_pmail.cxx filter_pmail.hxx \
             filter_plain.cxx filter_plain.hxx \
             filter_mbox.cxx filter_mbox.hxx

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

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmailcvt.pot; \
	fi

