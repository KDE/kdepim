INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) -I$(top_srcdir)/kpilot/kpilot
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = expense-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed
bin_PROGRAMS = conduitExpense

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for this conduit
conduitExpense_SOURCES= expense.cc setupDialog.cc

# the library search path
conduitExpense_LDFLAGS = $(all_libraries) \
	$(KDE_RPATH) \
	-L../../kpilot

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
conduitExpense_LDADD   =  -lconduit \
	$(LIB_KDEUI) $(LIB_KFILE) $(LIB_KDECORE) \
	@LIBSOCKET@ -lpisock -lkdbcore -lkdb_postgres -lkdb_mysql

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = expense.h setupDialog.h 
