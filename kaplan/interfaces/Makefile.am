INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkpinterfaces.la

libkpinterfaces_la_SOURCES = kpcore.cpp kpplugin.cpp 
libkpinterfaces_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libkpinterfaces_la_LIBADD = $(LIB_KPARTS)

kpincludedir = $(includedir)/kaplan
kpinclude_HEADERS = kpcore.h kpplugin.h 

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kaplanplugin.desktop
