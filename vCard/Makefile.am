INCLUDES= -I include -I include/generated $(all_includes)

lib_LTLIBRARIES = libvcard.la

libvcard_la_SOURCES = vCard.cpp

test: test.o libvcard.a
	$(CXX) -o test test.cpp libvcard.a $(all_libraries) -lqt $(INCLUDES)
