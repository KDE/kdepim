INCLUDES= -I$(top_srcdir)/kpilot/kpilot -I$(top_srcdir)/abbrowser $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed
bin_PROGRAMS = abbrowser_conduit

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for abbrowser_conduit
abbrowser_conduit_SOURCES= abbrowser-conduit.cc \
	abbrowserConduitConfig_base.ui \
	abbrowserConduitConfig.cc \
	setupDialog.cc \
	contactentry.cpp

# the library search path
abbrowser_conduit_LDFLAGS = $(all_libraries) \
	$(KDE_RPATH) \
	-L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
abbrowser_conduit_LDADD   =  ../../kpilot/pilotAddress.o \
	-lconduit -lpilotdb \
	$(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) \
	@LIBSOCKET@ -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = abbrowser-conduit.h abbrowserConduitConfig.h setupDialog.h
