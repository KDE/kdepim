###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

###
### Build the VCC library first.
###
SUBDIRS = versit

INCLUDES= @all_includes@ -I$(LIBPISOCK_PATH) \
	-I$(top_srcdir)/kpilot -I$(top_srcdir)/kpilot/kpilot -Iversit
METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = todo-conduit.desktop vcal-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed 
bin_PROGRAMS = todoconduit vcalconduit

# Which sources should be compiled for todoconduit
todoconduit_SOURCES= todo-conduit.cc todo-setup.cc vcalBase.cc
vcalconduit_SOURCES = vcal-conduit.cc vcal-setup.cc vcalBase.cc


# the library search path
todoconduit_LDFLAGS = $(all_libraries) -Lversit \
	-L../../kpilot
vcalconduit_LDFLAGS = $(all_libraries) -Lversit \
	-L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
todoconduit_LDADD   = \
	-lconduit -lpilotdb \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) \
	@LIBSOCKET@ -lversit $(LIBPISOCK)
vcalconduit_LDADD   = \
	-lconduit -lpilotdb \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) \
	@LIBSOCKET@ -lversit $(LIBPISOCK)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = todo-conduit.h todo-setup.h vcal-conduit.h vcal-setup.h

