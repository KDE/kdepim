INCLUDES= @all_includes@ -I../../kpilot -Iversit -I../../kpilot/libpilot/include
SUBDIRS = versit
METASOURCES = AUTO

####### This part is very kpilot specific
# you can add here more. This one gets installed
noinst_PROGRAMS = vcalconduit

# Which sources should be compiled for vcalconduit
vcalconduit_SOURCES= vcal-conduit.cc vcal-setup.cc

# the library search path
vcalconduit_LDFLAGS = @all_libraries@ -L`pwd`/versit -L`pwd`/../../kpilot -L/usr/local/lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
vcalconduit_LDADD   = ../../kpilot/pilotDateEntry.o -lconduit \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lversit -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = vcal-conduit.h vcal-setup.h

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kpilot_SOURCES)