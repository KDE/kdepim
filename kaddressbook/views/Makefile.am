INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook $(all_includes) 

noinst_HEADERS = cardview.h colorlistbox.h configurecardviewdialog.h \
  configuretableviewdialog.h contactlistview.h kaddressbookcardview.h \
  kaddressbookiconview.h kaddressbooktableview.h

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = cardview.desktop iconview.desktop tableview.desktop

kde_module_LTLIBRARIES = libkaddrbk_cardview.la \
  libkaddrbk_iconview.la \
  libkaddrbk_tableview.la

libkaddrbk_cardview_la_SOURCES = cardview.cpp colorlistbox.cpp \
  configurecardviewdialog.cpp kaddressbookcardview.cpp
libkaddrbk_cardview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_cardview_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_cardview_la_METASOURCES = AUTO

libkaddrbk_iconview_la_SOURCES = kaddressbookiconview.cpp
libkaddrbk_iconview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_iconview_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_iconview_la_METASOURCES = AUTO

libkaddrbk_tableview_la_SOURCES = configuretableviewdialog.cpp \
  contactlistview.cpp kaddressbooktableview.cpp 
libkaddrbk_tableview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_tableview_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_tableview_la_METASOURCES = AUTO
