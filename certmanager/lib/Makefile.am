INCLUDES = -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

SUBDIRS = pics kleo backends ui . tests

lib_LTLIBRARIES = libkleopatra.la

libkleopatra_la_SOURCES = \
			cryptplug.cpp \
			cryptplugwrapper.cpp \
			cryptplugwrapperlist.cpp \
			cryptplugfactory.cpp

libkleopatra_la_LIBADD = \
			kleo/libkleopatra_core.la \
			backends/qgpgme/libkleopatra_backend_qgpgme.la \
			ui/libkleopatra_ui.la \
			\
			$(top_builddir)/libkdenetwork/qgpgme/libqgpgme.la \
			$(LIB_KDEUI)

libkleopatra_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h kleo/*.cpp kleo/*.h -o $(podir)/libkleopatra.pot

include ../../admin/Doxyfile.am
