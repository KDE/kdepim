INCLUDES = -I$(top_srcdir) -I$(top_builddir)/kaddressbook/common \
           -I$(top_srcdir)/kaddressbook/common \
           -I$(top_srcdir)/kaddressbook/interfaces \
           -I$(top_srcdir)/kaddressbook $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_cardview.la libkaddrbk_iconview.la \
                         libkaddrbk_tableview.la

XXLIBS = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_cardview_la_SOURCES = cardview.cpp colorlistbox.cpp \
                                 configurecardviewdialog.cpp \
                                 kaddressbookcardview.cpp
libkaddrbk_cardview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_cardview_la_LIBADD = $(XXLIBS)

libkaddrbk_iconview_la_SOURCES = kaddressbookiconview.cpp
libkaddrbk_iconview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_iconview_la_LIBADD = $(XXLIBS)

libkaddrbk_tableview_la_SOURCES = configuretableviewdialog.cpp \
                                  contactlistview.cpp \
                                  kaddressbooktableview.cpp 
libkaddrbk_tableview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_tableview_la_LIBADD = $(XXLIBS)

noinst_HEADERS = cardview.h colorlistbox.h configurecardviewdialog.h \
                 configuretableviewdialog.h contactlistview.h \
                 kaddressbookcardview.h kaddressbookiconview.h \
                 kaddressbooktableview.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = cardview.desktop iconview.desktop tableview.desktop
