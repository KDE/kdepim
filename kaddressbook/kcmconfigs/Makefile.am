INCLUDES= -I$(top_srcdir)/kaddressbook -I$(top_srcdir)/kaddressbook/interfaces \
          -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kcm_kabconfig.la kcm_kabldapconfig.la

kcm_kabconfig_la_SOURCES = kcmkabconfig.cpp kabconfigwidget.cpp \
                           extensionconfigdialog.cpp addresseewidget.cpp
kcm_kabconfig_la_LDFLAGS  = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_kabconfig_la_LIBADD = $(XTESTLIB) $(LIB_KIO) \
                          $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
                          $(top_builddir)/libkdepim/libkdepim.la ../libkabcommon.la

kcm_kabldapconfig_la_SOURCES = addhostdialog.cpp kcmkabldapconfig.cpp \
                               ldapoptionswidget.cpp
kcm_kabldapconfig_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                               -avoid-version -no-undefined
kcm_kabldapconfig_la_LIBADD = $(XTESTLIB) $(LIB_KIO) $(LIB_KABC)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkabconfig.pot

pim_DATA = kabconfig.desktop kabldapconfig.desktop
pimdir = $(kde_appsdir)/PIM
