INCLUDES = -I$(top_srcdir) -I$(top_builddir)/kresources/blogging/ $(all_includes)

lib_LTLIBRARIES = libkblog.la
libkblog_la_SOURCES = bloginterface.cpp xmlrpciface.cpp \
                      api_blogger.cpp api_metaweblog.cpp api_movabletype.cpp \
											api_file.cpp api_drupal.cpp 
libkblog_la_LDFLAGS = $(all_libraries) -no-undefined
libkblog_la_LIBADD  = $(LIB_QT) $(LIB_KDECORE) $(top_builddir)/libkdepim/libkdepim.la

### libkcalincludedir = $(includedir)/libkcal
### libkcalinclude_HEADERS = resourcebloggingconfig.h resourceblogging.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_blogging.pot

