INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector2 $(all_includes)

lib_LTLIBRARIES = libkitchensyncui.la

libkitchensyncui_la_SOURCES = partbar.cc manipulatorpart.cc \
    ksync_configuredialog.cc \
    ksync_systemtray.cc ksync_mainwindow.cc \
    ksync_configpart.cc ksync_profile.cc \
    manpartservice.cpp profilemanager.cpp \
    profilefilemanager.cpp konnectorprofile.cpp \
    konnectorprofilemanager.cpp \
    konnectorprofilefilemanager.cpp \
    konnectorprofilelistviewbase.ui \
    konnectordialog.cpp \
    konnectorcheckitem.cpp \
    konnectorprofilewizardintro.ui \
    konnectorwizard.cpp konnectorwizardoutro.ui \
    profilelistbase.ui profilewizardchooser.ui \
    profilewizardintro.ui profilecheckitem.cpp \
    profilewizardchooserimpl.cpp profilewizard.cpp \
    profileitem.cpp profiledialog.cpp syncalgo.cpp \
    synchelper.cpp konnectorbar.cpp syncconfig.cpp
    
libkitchensyncui_la_LDFLAGS = $(all_libraries) -no-undefined 
libkitchensyncui_la_LIBADD = $(LIB_KPARTS) $(top_builddir)/kitchensync/libkonnector2/libkonnector.la

ksyncincludedir = $(includedir)/kitchensync
ksyncinclude_HEADERS = partbar.h manipulatorpart.h ksync_systemtray.h ksync_mainwindow.h ksync_profile.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kitchensync.desktop
