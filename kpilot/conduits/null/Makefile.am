INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) \
	-I$(top_srcdir)/kpilot -I$(top_srcdir)/kpilot/kpilot
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = null-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed
# bin_PROGRAMS = null_conduit
kde_module_LTLIBRARIES = libnullconduit.la

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for vcalconduit
# null_conduit_SOURCES= null-conduit.cc setupDialog.cc
libnullconduit_la_SOURCES = null-factory.cc setup_base.ui
libnullconduit_la_LIBADD = ../../kpilot/uiDialog.o ../../kpilot/options.o

# the library search path
# null_conduit_LDFLAGS = $(all_libraries) \
	# $(KDE_RPATH) -L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
# null_conduit_LDADD   =  \
	# -lconduit -lpilotdb \
	# $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) \
	# @LIBSOCKET@ -lpisock -lconduit

# this option you can leave out. Just, if you use "make dist", you need it
# noinst_HEADERS = null-conduit.h setupDialog.h
