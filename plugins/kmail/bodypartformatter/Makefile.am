KDE_OPTIONS = nofinal
KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = -I$(top_srcdir)/kmail \
           -I$(top_srcdir)/libkdepim \
           -I$(top_srcdir)/libkmime \
           -I$(top_srcdir)/mimelib \
           -I$(top_srcdir) \
           $(all_includes)

kde_module_LTLIBRARIES = libkmail_bodypartformatter_text_vcard.la \
                         libkmail_bodypartformatter_text_calendar.la \
                         libkmail_bodypartformatter_text_xdiff.la

libkmail_bodypartformatter_text_vcard_la_SOURCES = text_vcard.cpp
libkmail_bodypartformatter_text_vcard_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
libkmail_bodypartformatter_text_vcard_la_LIBADD  = $(LIB_QT) $(LIB_KABC) \
                                                   ../../../libkdepim/libkdepim.la \
                                                   ../../../libkmime/libkmime.la

libkmail_bodypartformatter_text_calendar_la_SOURCES = text_calendar.cpp
libkmail_bodypartformatter_text_calendar_la_LDFLAGS = \
  $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
libkmail_bodypartformatter_text_calendar_la_LIBADD  = $(LIB_QT) $(LIB_KABC) \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/kmail/libkmailprivate.la

libkmail_bodypartformatter_text_xdiff_la_SOURCES = text_xdiff.cpp
libkmail_bodypartformatter_text_xdiff_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
libkmail_bodypartformatter_text_xdiff_la_LIBADD  = $(LIB_QT)


linkdir = $(kde_datadir)/kmail/plugins/bodypartformatter
link_DATA = text_vcard.desktop text_calendar.desktop text_xdiff.desktop

METASOURCES = AUTO

KDE_ICON = AUTO

messages:
	$(XGETTEXT) text_vcard.cpp -o $(podir)/kmail_text_vcard_plugin.pot
	$(XGETTEXT) text_calendar.cpp -o $(podir)/kmail_text_calendar_plugin.pot
	$(XGETTEXT) text_xdiff.cpp -o $(podir)/kmail_text_xdiff_plugin.pot
