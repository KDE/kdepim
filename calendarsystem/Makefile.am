INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkcalsystem.la
libkcalsystem_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:1:0 -module
libkcalsystem_la_LIBADD  = $(LIB_QT) $(LIB_KDEUI)
libkcalsystem_la_SOURCES = kcalendarsystem.cpp kcalendarsystemhijri.cpp hconv.c

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkcalsystem.pot

check_PROGRAMS = calendar3

calendar3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
calendar3_LDADD   = $(LIB_KDEUI) ./libkcalsystem.la
calendar3_SOURCES = calendar3.cpp  mkdatepicker.cpp mkdatetbl.cpp

METASOURCES = AUTO

include $(top_srcdir)/admin/Doxyfile.am
