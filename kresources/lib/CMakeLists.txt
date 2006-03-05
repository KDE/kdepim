kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kgroupwarebase_LIB_SRCS 
   folderlister.cpp 
   folderconfig.cpp 
   folderlistview.cpp 
   groupwaredataadaptor.cpp 
   groupwaredownloadjob.cpp 
   calendaradaptor.cpp 
   addressbookadaptor.cpp 
   groupwareresourcejob.cpp 
   groupwareuploadjob.cpp 
   kcal_resourcegroupwarebase.cpp 
   kabc_resourcegroupwarebase.cpp 
   kcal_resourcegroupwarebaseconfig.cpp 
   kabc_resourcegroupwarebaseconfig.cpp 
   kcal_cachesettingsdlg.cpp )

kde4_automoc(${kgroupwarebase_LIB_SRCS})

kde4_add_kcfg_files(kgroupwarebase_LIB_SRCS kresources_groupwareprefs.kcfgc )

kde4_add_library(kgroupwarebase SHARED ${kgroupwarebase_LIB_SRCS})

target_link_libraries(kgroupwarebase  ${QT_AND_KDECORE_LIBS} kcal kdepim )

set_target_properties(kgroupwarebase PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kgroupwarebase )


########### next target ###############

set(kgroupwaredav_LIB_SRCS 
   davcalendaradaptor.cpp 
   davaddressbookadaptor.cpp 
   webdavhandler.cpp 
   davgroupwareglobals.cpp )

kde4_automoc(${kgroupwaredav_LIB_SRCS})

kde4_add_library(kgroupwaredav SHARED ${kgroupwaredav_LIB_SRCS})

target_link_libraries(kgroupwaredav  ${QT_AND_KDECORE_LIBS} kcal kdepim kgroupwarebase )

set_target_properties(kgroupwaredav PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kgroupwaredav )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) \
#-I$(top_srcdir)/libkcal/libical/src/libical -I$(top_srcdir)/libkcal/libical/src/libicalss \
#    -I$(top_builddir)/libkcal/libical/src/libical -I$(top_builddir)/libkcal/libical/src/libicalss \
#  $(all_includes)
#
#lib_LTLIBRARIES = libkgroupwarebase.la libkgroupwaredav.la
#
#libkgroupwarebase_la_SOURCES = folderlister.cpp folderconfig.cpp folderlistview.cpp \
#  groupwaredataadaptor.cpp groupwaredownloadjob.cpp \
#  calendaradaptor.cpp addressbookadaptor.cpp groupwareresourcejob.cpp \
#  groupwareuploadjob.cpp kcal_resourcegroupwarebase.cpp kabc_resourcegroupwarebase.cpp \
#  kresources_groupwareprefs.kcfgc \
#  kcal_resourcegroupwarebaseconfig.cpp kabc_resourcegroupwarebaseconfig.cpp \
#	kcal_cachesettingsdlg.cpp
#libkgroupwarebase_la_LDFLAGS = $(all_libraries) -no-undefined
#libkgroupwarebase_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                      $(top_builddir)/libkdepim/libkdepim.la
#
#libkgroupwaredav_la_SOURCES = davcalendaradaptor.cpp davaddressbookadaptor.cpp webdavhandler.cpp \
#  davgroupwareglobals.cpp
#libkgroupwaredav_la_LDFLAGS = $(all_libraries) -no-undefined
#libkgroupwaredav_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                      $(top_builddir)/libkdepim/libkdepim.la libkgroupwarebase.la
#
#METASOURCES = AUTO
#
#### We use three (or more) namespaces here in here - KPIM, KABC and KCal.  
#### When using multiple "using namespace foo" declarations, namespaced
#### definitions cannot be resolved properly; we could use namespace foo { }
#### instead, but this breaks on KConfigXT generated code, which still uses
#### "using namespace" and can't be fixed due to our backwards-compatibility 
#### requirements. This is only fatal when doing --enable-final, since only
#### then do all the namespaces get mixed together.
#KDE_OPTIONS = nofinal
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kdepimresources.pot
