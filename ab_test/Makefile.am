INCLUDES= -I include -I ../vCard/include -I ../vCard/include/generated \
          -I ../ldif/include -I ../ldif/include/generated  $(all_includes)

SUBDIRS = filters

lib_LTLIBRARIES = libab_test.la

AM_CXXFLAGS = -rdynamic
libab_test_la_LIBADD = -L/lib/ -ldl
libab_test_la_SOURCES = ab_test.cpp

#test: test.o ab_test.la
#  $(CXX) -o test test.o -lab_test -lvcard $(all_libraries) $(LDFLAGS) \
#  -lqt -rdynamic -ldl
