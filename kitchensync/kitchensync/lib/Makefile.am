INCLUDES = -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2 $(all_includes)

lib_LTLIBRARIES = libkitchensyncui.la

libkitchensyncui_la_SOURCES = partbar.cpp actionpart.cpp \
    configuredialog.cpp aboutdata.cpp \
    systemtray.cpp core.cpp actionmanager.cpp mainwindow.cpp kitchensync.cpp \
    profile.cpp \
    actionpartservice.cpp profilemanager.cpp \
    profileconfig.cpp \
    profilecheckitem.cpp  profilewizard.cpp \
    profileitem.cpp profiledialog.cpp syncalgo.cpp \
    synchelper.cpp konnectorbar.cpp syncconfig.cpp engine.cpp \
    konnectorview.cpp
    
libkitchensyncui_la_LDFLAGS = $(all_libraries) -no-undefined 
libkitchensyncui_la_LIBADD = \
  $(top_builddir)/kitchensync/libkonnector2/libkonnector.la \
  -lkutils $(LIB_KPARTS) 
ksyncincludedir = $(includedir)/kitchensync
ksyncinclude_HEADERS = partbar.h actionpart.h systemtray.h mainwindow.h \
  profile.h konnectorview.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kitchensync.desktop

kde_module_LTLIBRARIES = libkitchensyncpart.la

libkitchensyncpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkitchensyncpart_la_LIBADD  = libkitchensyncui.la
libkitchensyncpart_la_SOURCES = kitchensync_part.cpp
