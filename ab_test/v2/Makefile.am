INCLUDES= -I _include $(all_includes)

SUBDIRS = filters

lib_LTLIBRARIES = libab_test2.la

AM_CXXFLAGS = -rdynamic
libab_test2_la_LIBADD = -L/lib/ -ldl
libab_test2_la_SOURCES = ab2.cpp

