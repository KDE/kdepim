INCLUDES = -I$(top_srcdir) \
           -I$(top_srcdir)/kaddressbook/interfaces \
           -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)/libkdenetwork \
           $(GPGME_CFLAGS) \
           $(all_includes) 

if compile_GNOKIIXXPORT
  gnokii_MODULE = libkaddrbk_gnokii_xxport.la
  gnokii_RCFILE = gnokii_xxportui.rc 
endif

kde_module_LTLIBRARIES = libkaddrbk_csv_xxport.la libkaddrbk_vcard_xxport.la \
                         libkaddrbk_kde2_xxport.la libkaddrbk_bookmark_xxport.la \
                         libkaddrbk_eudora_xxport.la libkaddrbk_ldif_xxport.la \
                         libkaddrbk_opera_xxport.la libkaddrbk_pab_xxport.la $(gnokii_MODULE)

AM_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_csv_xxport_la_SOURCES = csv_xxport.cpp csvimportdialog.cpp dateparser.cpp
libkaddrbk_csv_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_vcard_xxport_la_SOURCES = vcard_xxport.cpp
libkaddrbk_vcard_xxport_la_LIBADD = $(XXLIBS) \
                                    $(top_builddir)/kaddressbook/libkaddressbook.la \
                                    $(top_builddir)/libkdenetwork/qgpgme/libqgpgme.la

libkaddrbk_kde2_xxport_la_SOURCES = kde2_xxport.cpp
libkaddrbk_kde2_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_bookmark_xxport_la_SOURCES = bookmark_xxport.cpp
libkaddrbk_bookmark_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_eudora_xxport_la_SOURCES = eudora_xxport.cpp
libkaddrbk_eudora_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_ldif_xxport_la_SOURCES = ldif_xxport.cpp
libkaddrbk_ldif_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_gnokii_xxport_la_SOURCES = gnokii_xxport.cpp
libkaddrbk_gnokii_xxport_la_LIBADD = $(XPMLIB) $(LIB_GNOKII) $(XXLIBS)

libkaddrbk_opera_xxport_la_SOURCES = opera_xxport.cpp
libkaddrbk_opera_xxport_la_LIBADD = $(XXLIBS)

libkaddrbk_pab_xxport_la_SOURCES = pab_xxport.cpp pab_mapihd.cpp pab_pablib.cpp
libkaddrbk_pab_xxport_la_LIBADD = $(XXLIBS)

noinst_HEADERS = csvimportdialog.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kaddressbook
service_DATA = csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop \
               bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop \
               gnokii_xxport.desktop opera_xxport.desktop pab_xxport.desktop

rcdir = $(kde_datadir)/kaddressbook
rc_DATA = csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc \
          bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc \
          opera_xxportui.rc pab_xxportui.rc $(gnokii_RCFILE)
