INCLUDES= @all_includes@ -I$(top_srcdir)/kpilot/kpilot -I$(srcdir)/../vcalconduit/versit -I$(top_srcdir)/kpilot/kpilot/libpilot/include
METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = todo-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed 
noinst_PROGRAMS = todoconduit

# Which sources should be compiled for todoconduit
todoconduit_SOURCES= todo-conduit.cc todo-setup.cc

# the library search path
todoconduit_LDFLAGS = @all_libraries@ -L`pwd`/../vcalconduit/versit -L`pwd`/../../kpilot -L/usr/local/lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
todoconduit_LDADD   = ../../kpilot/pilotTodoEntry.o -lconduit \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lversit -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = todo-conduit.h todo-setup.h

