INCLUDES= -I_include -I../../kab_backend_test/ $(all_includes)

SUBDIRS = filters backends

lib_LTLIBRARIES = libab_test2.la

AM_CXXFLAGS = -rdynamic
libab_test2_la_LIBADD = -L/lib/ -ldl
libab_test2_la_SOURCES = ab2.cpp

