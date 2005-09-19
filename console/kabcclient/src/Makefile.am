SUBDIRS = examples

INCLUDES= $(all_includes)

bin_PROGRAMS = kabcclient

kabcclient_SOURCES = main.cpp csvtemplate.cpp csvtemplatefactory.cpp \
                     formatfactory.cpp kabcclient.cpp \
                     inputformatimpls.cpp outputformatimpls.cpp  

kabcclient_METASOURCES = AUTO
kabcclient_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kabcclient_LDADD = $(LIB_KABC)

DOXYGEN_REFERENCES = kabc
DOXYGEN_EXCLUDE = main.cpp src.pro
include $(top_srcdir)/admin/Doxyfile.am
