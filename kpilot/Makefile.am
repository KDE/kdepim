## kdepim/kpilot/Makefile.am
## (C) 1997 Stephan Kulow
## Modified by Adriaan de Groot

SUBDIRS = lib kpilot conduits

messages:
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui"`  > rc.cpp
	$(XGETTEXT) `find . -name "*.cc" -o -name "*.cpp"` -o $(podir)/kpilot.pot   
	-rm -f rc.cpp

include ../admin/Doxyfile.am

# Just for the KPilot developers: grab all the strings, for checking.
devel-pot :
	extractrc `find . -name "*.rc" -o -name "*.ui"` > /tmp/rc.cpp
	xgettext -C --keyword=i18n --keyword=I18N_NOOP -o /tmp/kpilot.po \
		`find . -name "*.cc"` /tmp/rc.cpp

