## kdepim/kpilot/Makefile.am
## (C) 1997 Stephan Kulow
## Modified by Adriaan de Groot

COMPILE_FIRST = lib 
SUBDIRS = $(COMPILE_FIRST) kpilot conduits

messages:
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui"`  > rc.cpp
	$(XGETTEXT) `find . -name "*.cc" -o -name "*.cpp"` -o $(podir)/kpilot.pot   
	-rm -f rc.cpp
	ls @abs_top_srcdir@

# Just for the KPilot developers: grab all the strings, for checking.
devel-pot :
	$(XGETTEXT) -C --keyword=i18n --keyword=I18N_NOOP -o /tmp/kpilot.po \
		`find . -name "*.cc"`

include $(top_srcdir)/admin/Doxyfile.am
