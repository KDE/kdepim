INCLUDES= @all_includes@ -I../../kpilot -I../vcalconduit/versit -I../../kpilot/libpilot/include
METASOURCES = AUTO

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

# just to make sure, automake makes them 
BUILTSOURCES = todo-setup.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kpilot_SOURCES)
