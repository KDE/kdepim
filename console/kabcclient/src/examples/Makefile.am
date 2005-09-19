INCLUDES= $(all_includes) -I$(top_srcdir)/kabcclient/src

noinst_PROGRAMS = converter

# Sources of the main program
KABCCLIENT_SOURCES = ../formatfactory.cpp \
                     ../inputformatimpls.cpp ../outputformatimpls.cpp \
                     ../csvtemplate.cpp ../csvtemplatefactory.cpp

converter_SOURCES = converter.cpp $(KABCCLIENT_SOURCES)

converter_METASOURCES = AUTO
converter_LDFLAGS = $(all_libraries) $(KDE_RPATH)

converter_LDADD = $(LIB_KABC)
