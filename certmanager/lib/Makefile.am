INCLUDES = -I$(top_srcdir)/libkdenetwork -I$(top_builddir)/libkdepim \
   $(GPGME_CFLAGS) $(all_includes)

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

# Disabled for kdepim-3.3, see also kleo/cryptobackendfactory.cpp
# backends/kpgp/libkleopatra_backend_kpgp.la

libkleopatra_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined

METASOURCES = AUTO

# remove the .desktop suffix on install (which is needed for translation)
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/libkleopatrarc.desktop $(DESTDIR)$(kde_confdir)/libkleopatrarc

uninstall-local:
	rm -f $(DESTDIR)$(kde_confdir)/libkleopatrarc

messages: #rc.cpp
	$(EXTRACTRC) */*.ui > rc.cpp
	$(XGETTEXT) *.cpp *.h \
	kleo/*.cpp kleo/*.h \
	ui/*.cpp ui/*.h \
	backends/*/*.cpp backends/*/*.h -o $(podir)/libkleopatra.pot

include ../../admin/Doxyfile.am
