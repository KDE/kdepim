LIB_KSCRIPT = $(top_builddir)/../kdelibs/interfaces/kscript/libkscript.la

METASOURCES = AUTO

check_PROGRAMS = runscripts

runscripts_SOURCES = pythonscript.cpp runscripts.cpp
runscripts_LDFLAGS = $(all_libraries) $(KDE_RPATH)
runscripts_LDADD = $(LIB_KSCRIPT)

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES = -I$(top_srcdir)/../kdelibs/interfaces -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/../kdelibs/interfaces/kscript $(all_includes)

TESTS = runscripts
