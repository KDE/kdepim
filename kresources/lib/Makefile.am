INCLUDES = -I$(top_srcdir) -I$(top_builddir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkgroupwarebase.la libkgroupwaredav.la

libkgroupwarebase_la_SOURCES = folderlister.cpp folderconfig.cpp \
  groupwaredataadaptor.cpp groupwaredownloadjob.cpp \
  calendaradaptor.cpp addressbookadaptor.cpp groupwareresourcejob.cpp \
  groupwareuploadjob.cpp kcal_resourcegroupwarebase.cpp kabc_resourcegroupwarebase.cpp \
  kcal_groupwareprefs.kcfgc kabc_groupwareprefs.kcfgc \
  kcal_resourcegroupwarebaseconfig.cpp kabc_resourcegroupwarebaseconfig.cpp
libkgroupwarebase_la_LDFLAGS = $(all_libraries) -no-undefined
libkgroupwarebase_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la

libkgroupwaredav_la_SOURCES = davcalendaradaptor.cpp davaddressbookadaptor.cpp webdavhandler.cpp \
  davgroupwareglobals.cpp
libkgroupwaredav_la_LDFLAGS = $(all_libraries) -no-undefined
libkgroupwaredav_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la libkgroupwarebase.la

METASOURCES = AUTO

### We use three (or more) namespaces here in here - KPIM, KABC and KCal.  
### When using multiple "using namespace foo" declarations, namespaced
### definitions cannot be resolved properly; we could use namespace foo { }
### instead, but this breaks on KConfigXT generated code, which still uses
### "using namespace" and can't be fixed due to our backwards-compatibility 
### requirements. This is only fatal when doing --enable-final, since only
### then do all the namespaces get mixed together.
KDE_OPTIONS = nofinal

