



########### next target ###############

set(kslox_LIB_SRCS 
   sloxaccounts.cpp 
   webdavhandler.cpp 
   sloxfolder.cpp 
   sloxfoldermanager.cpp 
   sloxfolderdialog.cpp 
   sloxbase.cpp )

kde4_automoc(${kslox_LIB_SRCS})

kde4_add_library(kslox SHARED ${kslox_LIB_SRCS})

target_link_libraries(kslox  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim )

set_target_properties(kslox PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kslox  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kabc_slox_LIB_SRCS kabcresourceslox.cpp kabcresourcesloxconfig.cpp )

kde4_automoc(${kabc_slox_LIB_SRCS})

kde4_add_kcfg_files(kabc_slox_LIB_SRCS kabcsloxprefs.kcfgc )

kde4_add_library(kabc_slox SHARED ${kabc_slox_LIB_SRCS})

target_link_libraries(kabc_slox  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kslox)

set_target_properties(kabc_slox PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kabc_slox  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_slox_LIB_SRCS kcalresourceslox.cpp kcalresourcesloxconfig.cpp )

kde4_automoc(${kcal_slox_LIB_SRCS})

kde4_add_kcfg_files(kcal_slox_LIB_SRCS kcalsloxprefs.kcfgc )

kde4_add_library(kcal_slox SHARED ${kcal_slox_LIB_SRCS})

target_link_libraries(kcal_slox  ${KDE4_KDECORE_LIBS} kslox ${KDE4_KCAL_LIBS})

set_target_properties(kcal_slox PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kcal_slox  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_slox_plugin_PART_SRCS kcalresourceslox_plugin.cpp )

kde4_automoc(${kcal_slox_plugin_PART_SRCS})

kde4_add_plugin(kcal_slox_plugin ${kcal_slox_plugin_PART_SRCS})
set_target_properties(kcal_slox_plugin PROPERTIES OUTPUT_NAME kcal_slox)

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_slox_plugin )

target_link_libraries(kcal_slox_plugin  ${KDE4_KDECORE_LIBS} kcal_slox )

install(TARGETS kcal_slox_plugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kabc_slox_plugin_PART_SRCS kabcresourceslox_plugin.cpp)

kde4_automoc(${kabc_slox_plugin_PART_SRCS})

kde4_add_plugin(kabc_slox_plugin ${kabc_slox_plugin_PART_SRCS})
set_target_properties(kabc_slox_plugin PROPERTIES OUTPUT_NAME kabc_slox)

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kabc_slox_plugin )

target_link_libraries(kabc_slox_plugin  ${KDE4_KDECORE_LIBS} kabc_slox )

install(TARGETS kabc_slox_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})



########### next target ###############

#set(testsloxaccounts_SRCS testsloxaccounts.cpp )

#kde4_automoc(${testsloxaccounts_SRCS})

#if(KDE4_BUILD_TESTS)

#kde4_add_executable(testsloxaccounts ${testsloxaccounts_SRCS})

#target_link_libraries(testsloxaccounts  ${KDE4_KDECORE_LIBS} kslox )

#endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES kabc_slox.desktop kabc_ox.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kcal_slox.desktop kcal_ox.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#
#lib_LTLIBRARIES = libkslox.la libkabc_slox.la libkcal_slox.la
#
#libkslox_la_SOURCES = sloxaccounts.cpp webdavhandler.cpp \
#                      sloxfolder.cpp sloxfoldermanager.cpp \
#                      sloxfolderdialog.cpp sloxbase.cpp
#libkslox_la_LDFLAGS = $(all_libraries)
#libkslox_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                      $(top_builddir)/libkdepim/libkdepim.la
#
#libkcal_slox_la_SOURCES = kcalresourceslox.cpp kcalresourcesloxconfig.cpp \
#                          kcalsloxprefs.kcfgc
#libkcal_slox_la_LDFLAGS = $(all_libraries)
#libkcal_slox_la_LIBADD  = libkslox.la $(top_builddir)/libkcal/libkcal.la
#
#libkabc_slox_la_SOURCES = kabcresourceslox.cpp kabcresourcesloxconfig.cpp \
#                          kabcsloxprefs.kcfgc
#libkabc_slox_la_LDFLAGS = $(all_libraries)
#libkabc_slox_la_LIBADD  = libkslox.la  -lkabc
#
#
#kde_module_LTLIBRARIES = kcal_slox.la kabc_slox.la
#
#kcal_slox_la_SOURCES = kcalresourceslox_plugin.cpp
#kcal_slox_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_slox_la_LIBADD  = libkcal_slox.la
#
#kabc_slox_la_SOURCES = kabcresourceslox_plugin.cpp
#kabc_slox_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kabc_slox_la_LIBADD  = libkabc_slox.la
#
#
#EXTRA_PROGRAMS = testsloxaccounts
#
#testsloxaccounts_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testsloxaccounts_LDADD = libkslox.la
#testsloxaccounts_SOURCES = testsloxaccounts.cpp
#
#
#kcal_servicedir = $(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}
#kcal_service_DATA = kcal_slox.desktop kcal_ox.desktop
#
#kabc_servicedir = $(kde_servicesdir)/kresources/kabc
#kabc_service_DATA = kabc_slox.desktop kabc_ox.desktop
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_slox.pot
