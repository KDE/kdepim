INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkabinterfaces.la

libkabinterfaces_la_SOURCES = configurewidget.cpp contacteditorwidget.cpp \
                              core.cpp extensionwidget.cpp xxport.cpp
libkabinterfaces_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libkabinterfaces_la_LIBADD = $(LIB_QTGUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KABC)

kabincludedir = $(includedir)/kaddressbook
kabinclude_HEADERS = configurewidget.h contacteditorwidget.h core.h \
                     extensionwidget.h xxport.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kaddressbook_contacteditorwidget.desktop \
                   kaddressbook_extension.desktop kaddressbook_xxport.desktop
