INCLUDES= @all_includes@ -I$(LIBPISOCK_PATH) \
	-I$(top_srcdir)/kpilot -I$(top_srcdir)/kpilot/kpilot -I$(srcdir)/../vcalconduit/versit
METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = todo-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed 
bin_PROGRAMS = todoconduit

# Which sources should be compiled for todoconduit
todoconduit_SOURCES= todo-conduit.cc todo-setup.cc

# the library search path
todoconduit_LDFLAGS = $(all_libraries) -L../vcalconduit/versit \
	-L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
todoconduit_LDADD   = ../../kpilot/pilotTodoEntry.o -lconduit \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) \
	@LIBSOCKET@ -lversit $(LIBPISOCK)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = todo-conduit.h todo-setup.h

