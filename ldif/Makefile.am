INCLUDES = \
	-I$(top_srcdir)/ldif/include \
	-I$(top_srcdir)/ldif/include/generated \
	$(all_includes)

noinst_LIBRARIES = libldif.a

libldif_a_SOURCES = ldif.cpp

test: test.o libldif.a
	$(CXX) -o test test.o libldif.a $(all_libraries) -lqt
