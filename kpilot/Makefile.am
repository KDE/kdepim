## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

SUBDIRS = kpilot conduits po

# AUTOMAKE_OPTIONS = no-dependencies

#acinclude.m4: acinclude.m4.in libtool.m4.in kdminclude.m4
#	cat acinclude.m4.in libtool.m4.in kdminclude.m4 > acinclude.m4

messages:
	@for subdir in $(SUBDIRS); do \
	echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) messages) \
	done 
	(cd po && $(MAKE) merge)
