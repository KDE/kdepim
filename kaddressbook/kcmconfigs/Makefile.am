INCLUDES= -I$(top_builddir)/kaddressbook -I$(top_srcdir)/kaddressbook \
          -I$(top_srcdir)/kaddressbook/common \
          -I$(top_srcdir)/kaddressbook/interfaces \
          -I$(top_srcdir) -I$(top_builddir)/kaddressbook/common $(all_includes)

kde_module_LTLIBRARIES = kcm_kabconfig.la kcm_kabldapconfig.la \
                         kcm_kabcustomfields.la

kcm_kabconfig_la_SOURCES = kcmkabconfig.cpp kabconfigwidget.cpp \
                           extensionconfigdialog.cpp addresseewidget.cpp
kcm_kabconfig_la_LDFLAGS  = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_kabconfig_la_LIBADD = $(LIB_KIO) \
                          $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
                          $(top_builddir)/libkdepim/libkdepim.la \
                          $(top_builddir)/kaddressbook/common/libkabcommon.la
kcm_kabconfig_la_COMPILE_FIRST = $(top_builddir)/kaddressbook/common/kabprefs_base.h

kcm_kabldapconfig_la_SOURCES = addhostdialog.cpp kcmkabldapconfig.cpp \
                               ldapoptionswidget.cpp
kcm_kabldapconfig_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                               -avoid-version -no-undefined
kcm_kabldapconfig_la_LIBADD = $(LIB_KIO) $(LIB_KABC) $(top_builddir)/libkdepim/libkdepim.la

kcm_kabcustomfields_la_SOURCES = kcmkabcustomfields.cpp
kcm_kabcustomfields_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                               -avoid-version -no-undefined
kcm_kabcustomfields_la_LIBADD = $(LIB_KDEUI) -lqui \
                                $(top_builddir)/kaddressbook/common/libkabcommon.la
kcm_kabcustomfields_la_COMPILE_FIRST = $(top_builddir)/kaddressbook/common/kabprefs_base.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkabconfig.pot

kde_services_DATA = kabconfig.desktop kabldapconfig.desktop \
                    kabcustomfields.desktop
