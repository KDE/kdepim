## kdepim/kpilot/Makefile.am
## (C) 1997 Stephan Kulow
## Modified by Adriaan de Groot

COMPILE_FIRST = kpilot
SUBDIRS = $(COMPILE_FIRST) conduits 

messages:
	$(XGETTEXT) `find . -name "*.cc" -o -name "*.cpp"` -o $(podir)/kpilot.pot   

