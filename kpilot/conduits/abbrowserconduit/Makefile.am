INCLUDES= $(all_includes) -I$(top_srcdir)/kpilot/kpilot -I$(top_srcdir)/abbrowser
METASOURCES = AUTO

# servicedir = $(kde_servicesdir)
# service_DATA = abbrowser-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed
bin_PROGRAMS = abbrowser_conduit

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for abbrowser_conduit
abbrowser_conduit_SOURCES= abbrowser-conduit.cc

# the library search path
abbrowser_conduit_LDFLAGS = $(all_libraries) \
	$(KDE_RPATH) \
	-L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
abbrowser_conduit_LDADD   =  ../../kpilot/pilotAddress.o \
	$(top_srcdir)/abbrowser/contactentry.o \
	$(top_srcdir)/abbrowser/contactentry.moc.o \
	-lconduit \
	$(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) \
	@LIBSOCKET@ -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = abbrowser-conduit.h 
