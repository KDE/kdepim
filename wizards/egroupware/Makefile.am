INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
           -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libegroupwarewizard.la

libegroupwarewizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libegroupwarewizard_la_LIBADD = $(top_builddir)/kresources/egroupware/libkabc_xmlrpc.la \
                                $(top_builddir)/kresources/egroupware/libkcal_xmlrpc.la \
                                $(top_builddir)/libkcal/libkcal.la \
                                $(top_builddir)/libkdepim/libkdepim.la \
                                $(LIB_KDEUID)
libegroupwarewizard_la_SOURCES = editdialog.cpp egroupwarewizard.cpp
