INCLUDES= @all_includes@ -I../../kpilot -I../../kpilot/libpilot/include
METASOURCES = AUTO

####### This part is very kpilot specific
# you can add here more. This one gets installed 
noinst_PROGRAMS = popmail_conduit

# Which sources should be compiled for popmail_conduit
popmail_conduit_SOURCES= popmail-conduit.cc \
	passworddialog.cc parsedate.c setupDialog.cc

# the library search path
popmail_conduit_LDFLAGS = @all_libraries@ -L`pwd`/../../kpilot -L/usr/local/lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
popmail_conduit_LDADD   =  -lconduit \
	$(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = popmail-conduit.h passworddialog.h setupDialog.h

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kpilot_SOURCES)
