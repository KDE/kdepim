INCLUDES = \
	-I$(top_srcdir)/vCard/include \
	-I$(top_srcdir)/vCard/include/generated \
	$(all_includes)

lib_LTLIBRARIES = libvcard.la

libvcard_la_SOURCES = vCard.cpp

test: test.o libvcard.a
	$(CXX) -o test test.cpp libvcard.a $(all_libraries) -lqt $(INCLUDES)
