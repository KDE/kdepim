
zoneinfodir = $(kde_datadir)/libical/zoneinfo

zoneinfo_DATA = zones.tab

DIRS = \
	Africa			\
	America			\
	America/Argentina	\
	America/Indiana		\
	America/Kentucky	\
	America/North_Dakota	\
	Antarctica		\
	Arctic			\
	Asia			\
	Atlantic		\
	Australia		\
	Europe			\
	Indian			\
	Pacific


install-data-local:
	for dir in $(DIRS); do \
	  $(mkinstalldirs) $(DESTDIR)$(zoneinfodir)/$$dir; \
	  for tzfile in $(srcdir)/$$dir/*.ics; do \
	    $(INSTALL_DATA) $$tzfile $(DESTDIR)$(zoneinfodir)/$$dir; \
	  done \
	done

dist-hook:
	for dir in $(DIRS); do \
	  mkdir $(distdir)/$$dir; \
	  for tzfile in $(srcdir)/$$dir/*.ics; do \
	    cp -p $$tzfile $(distdir)/$$dir; \
	  done \
	done

EXTRA_DIST = $(zoneinfo_DATA)
