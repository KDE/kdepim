METASOURCES = AUTO

check_PROGRAMS = runscripts

runscripts_SOURCES = script.cpp runscripts.cpp
runscripts_LDFLAGS = $(all_libraries) $(KDE_RPATH)
runscripts_LDADD = $(LIB_KIO)

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES = -I$(srcdir) -I$(top_srcdir) $(all_includes)

TESTS = runscripts
