INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kcm_kabconfig.la kcm_kabldapconfig.la

kcm_kabconfig_la_SOURCES = kcmkabconfig.cpp kabconfigwidget.cpp \
                           extensionconfigdialog.cpp addresseewidget.cpp
kcm_kabconfig_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_kabconfig_la_LIBADD = $(XTESTLIB) $(LIB_KIO) $(LIB_KDEUI) \
  $(top_builddir)/libkdepim/libkdepim.la ../libkabcommon.la

kcm_kabldapconfig_la_SOURCES = addhostdialog.cpp kcmkabldapconfig.cpp \
  ldapoptionswidget.cpp
kcm_kabldapconfig_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_kabldapconfig_la_LIBADD = $(XTESTLIB) $(LIB_KIO) $(LIB_KDEUI) -lkabc

kcm_kabconfig_la_METASOURCES = AUTO
kcm_kabldapconfig_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkabconfig.pot

data_DATA = kabconfig.desktop kabldapconfig.desktop
datadir = $(kde_appsdir)/PIM

EXTRA_DIST = $(data_DATA)
