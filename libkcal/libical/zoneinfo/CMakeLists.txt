kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/libical/zoneinfo FILES zones.tab )

kde4_footer()



#original Makefile.am contents follow:

#
#zoneinfodir = $(kde_datadir)/libical/zoneinfo
#
#zoneinfo_DATA = zones.tab
#
#DIRS = \
#	Africa			\
#	America			\
#	America/Indiana		\
#	America/Kentucky	\
#	Antarctica		\
#	Arctic			\
#	Asia			\
#	Atlantic		\
#	Australia		\
#	Europe			\
#	Indian			\
#	Pacific
#
#
#install-data-local:
#	for dir in $(DIRS); do \
#	  $(mkinstalldirs) $(DESTDIR)$(zoneinfodir)/$$dir; \
#	  for tzfile in $(srcdir)/$$dir/*.ics; do \
#	    $(INSTALL_DATA) $$tzfile $(DESTDIR)$(zoneinfodir)/$$dir; \
#	  done \
#	done
#
#dist-hook:
#	for dir in $(DIRS); do \
#	  mkdir $(distdir)/$$dir; \
#	  for tzfile in $(srcdir)/$$dir/*.ics; do \
#	    cp -p $$tzfile $(distdir)/$$dir; \
#	  done \
#	done
#
#EXTRA_DIST = $(zoneinfo_DATA)
