INCLUDES= @all_includes@ -I$(top_srcdir)/kpilot/kpilot -I$(top_srcdir)/kpilog/kpilot/libpilot/include
METASOURCES = AUTO

####### This part is very kpilot specific
# you can add here more. This one gets installed
noinst_PROGRAMS = null_conduit

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for vcalconduit
null_conduit_SOURCES= null-conduit.cc setupDialog.cc

# the library search path
null_conduit_LDFLAGS = @all_libraries@ -L`pwd`/../../kpilot -L/usr/local/lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
null_conduit_LDADD   =  -lconduit $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = null-conduit.h setupDialog.h
