KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = \
  $(all_includes) \
  -I$(top_srcdir)/empath/lib \
  -I$(top_srcdir)/rmm/

lib_LTLIBRARIES = libtwisterkernel.la

libtwisterkernel_la_LDFLAGS = $(all_libraries) \
	-version-info 1:0:0 -module 
	
#	Define this for testing.
#	-no-undefined

libtwisterkernel_la_LIBADD = $(LIB_KDECORE) \
	$(top_builddir)/empath/lib/libempathkernel.la
	$(top_builddir)/rmm/librmm.la

METASOURCES = AUTO

libtwisterkernel_la_SOURCES = Twister.cpp
