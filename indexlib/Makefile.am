KDE_CXXFLAGS=-include $(srcdir)/compat.h $(USE_EXCEPTIONS)
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libindex.la

libindex_la_SOURCES = bitstream.cpp compressed.cpp create.cpp exception.cpp ifile.cpp leafdata.cpp leafdatavector.cpp lockfile.cpp logfile.cpp match.cpp mmap_manager.cpp quotes.cpp slow.cpp stringarray.cpp stringset.cpp tokenizer.cpp

indexlibincludedir=$(includedir)/index
indexlibinclude_HEADERS = create.h index.h lockfile.h
bin_SCRIPTS = indexlib-config

