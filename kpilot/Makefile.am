## kdepim/kpilot/Makefile.am
## (C) 1997 Stephan Kulow
## Modified by Adriaan de Groot

COMPILE_FIRST = kpilot
SUBDIRS = $(COMPILE_FIRST)

messages:
	$(XGETTEXT) `find . -name "*.cc" -o -name "*.cpp"` -o $(podir)/kpilot.pot   

# Just for the KPilot developers: grab all the strings, for checking.
devel-pot :
	$(XGETTEXT) -C --keyword=i18n --keyword=I18N_NOOP -o /tmp/kpilot.po \
		`find . -name "*.cc"`
