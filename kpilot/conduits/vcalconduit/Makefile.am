INCLUDES= @all_includes@ -I$(top_srcdir)/kpilot/kpilot -I$(srcdir)/versit 
SUBDIRS = versit
METASOURCES = AUTO
bin_PROGRAMS = vcalconduit

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed
# noinst_PROGRAMS = vcalconduit

# Which sources should be compiled for vcalconduit
vcalconduit_SOURCES= vcal-conduit.cc vcal-setup.cc

# the library search path
vcalconduit_LDFLAGS = $(all_libraries) \
	$(KDE_RPATH) \
	-L./versit -L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
vcalconduit_LDADD   = ../../kpilot/pilotDateEntry.o -lconduit \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) \
	@LIBSOCKET@ -lversit -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = vcal-conduit.h vcal-setup.h

