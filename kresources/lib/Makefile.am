INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkgroupwarebase.la libkgroupwaredav.la

libkgroupwarebase_la_SOURCES = folderlister.cpp folderconfig.cpp \
  webdavhandler.cpp groupwaredataadaptor.cpp groupwaredownloadjob.cpp \
  calendaradaptor.cpp addressbookadaptor.cpp groupwareresourcejob.cpp \
  groupwareuploadjob.cpp kcal_resourcegroupwarebase.cpp kabc_resourcegroupwarebase.cpp \
  kcal_groupwareprefs.kcfgc kabc_groupwareprefs.kcfgc \
  kcal_resourcegroupwarebaseconfig.cpp kabc_resourcegroupwarebaseconfig.cpp \
  davgroupwareglobals.cpp
libkgroupwarebase_la_LDFLAGS = $(all_libraries) -no-undefined
libkgroupwarebase_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la

libkgroupwaredav_la_SOURCES = davfolderlister.cpp webdavhandler.cpp \
  davgroupwareglobals.cpp
libkgroupwaredav_la_LDFLAGS = $(all_libraries) -no-undefined
libkgroupwaredav_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la libkgroupwarebase.la

METASOURCES = AUTO
