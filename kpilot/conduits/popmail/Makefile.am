INCLUDES= @all_includes@ -I$(top_srcdir)/kpilot/kpilot -I$(top_srcdir)/kpilot/kpilot/libpilot/include
METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = popmail-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed 
bin_PROGRAMS = popmail_conduit

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

