



set(libfeatureplancommon_SRCS)

kode_add_xml_parser(libfeatureplancommon_SRCS kde-features.rng)
kde4_add_kcfg_files(libfeatureplancommon_SRCS prefs.kcfgc )

########### next target ###############

set(kcal_resourcefeatureplan_LIB_SRCS ${libfeatureplancommon_SRCS} kcal_resourcefeatureplan.cpp kcal_resourcefeatureplanconfig.cpp )

kde4_automoc(${kcal_resourcefeatureplan_LIB_SRCS})

kde4_add_library(kcal_resourcefeatureplan SHARED ${kcal_resourcefeatureplan_LIB_SRCS})

target_link_libraries(kcal_resourcefeatureplan  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim kcal_resourcefeatureplan )

set_target_properties(kcal_resourcefeatureplan PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kcal_resourcefeatureplan  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### next target ###############

set(kcal_resourcefeatureplan_PART_SRCS kcal_resourcefeatureplanplugin.cpp ${kcal_resourcefeatureplan_LIB_SRCS} ${libfeatureplancommon_SRCS})

kde4_automoc(${kcal_resourcefeatureplan_PART_SRCS})

kde4_add_plugin(kcal_resourcefeatureplan ${kcal_resourcefeatureplan_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_resourcefeatureplan )

target_link_libraries(kcal_resourcefeatureplan  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcal_resourcefeatureplan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

if(KDE4_BUILD_TESTS)
set(dumpfeaturelist_SRCS dumpfeaturelist.cpp ${libfeatureplancommon_SRCS})

kde4_automoc(${dumpfeaturelist_SRCS})


kde4_add_executable(dumpfeaturelist ${dumpfeaturelist_SRCS})

target_link_libraries(dumpfeaturelist  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############
if(KDE4_BUILD_TESTS)
set(benchmarkfeaturelist_SRCS benchmarkfeaturelist.cpp ${libfeatureplancommon_SRCS})

kde4_automoc(${benchmarkfeaturelist_SRCS})


kde4_add_executable(benchmarkfeaturelist ${benchmarkfeaturelist_SRCS})

target_link_libraries(benchmarkfeaturelist  ${KDE4_KDECORE_LIBS})

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES kcal_resourcefeatureplan.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libkcal_resourcefeatureplan.la
#
#noinst_LTLIBRARIES = libfeatureplancommon.la
#libfeatureplancommon_la_SOURCES = kde-features.cpp kde-features_parser.cpp \
#                                  prefs.kcfgc
#
#libkcal_resourcefeatureplan_la_SOURCES = kcal_resourcefeatureplan.cpp \
#                                         kcal_resourcefeatureplanconfig.cpp 
#libkcal_resourcefeatureplan_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
#                                         -version-info 1:0:0 -no-undefined
#libkcal_resourcefeatureplan_la_LIBADD = libfeatureplancommon.la \
#                                        $(top_builddir)/libkcal/libkcal.la \
#                                        $(top_builddir)/libkdepim/libkdepim.la
#libkcal_resourcefeatureplan_la_COMPILE_FIRST = kde-features.h prefs.h
#
#
#kde_module_LTLIBRARIES = kcal_resourcefeatureplan.la
#
#kcal_resourcefeatureplan_la_SOURCES = kcal_resourcefeatureplanplugin.cpp 
#kcal_resourcefeatureplan_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_resourcefeatureplan_la_LIBADD = libkcal_resourcefeatureplan.la
#kcal_resourcefeatureplan_la_COMPILE_FIRST = kde-features.h prefs.h
#
#kcal_servicedir = $(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}
#kcal_service_DATA = kcal_resourcefeatureplan.desktop
#
#check_PROGRAMS = dumpfeaturelist benchmarkfeaturelist
#
#dumpfeaturelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#dumpfeaturelist_LDADD   = libfeatureplancommon.la $(LIB_KDECORE)
#dumpfeaturelist_SOURCES = dumpfeaturelist.cpp
#dumpfeaturelist_COMPILE_FIRST = kde-features.h
#
#benchmarkfeaturelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#benchmarkfeaturelist_LDADD   = libfeatureplancommon.la $(LIB_KDECORE)
#benchmarkfeaturelist_SOURCES = benchmarkfeaturelist.cpp
#benchmarkfeaturelist_COMPILE_FIRST = kde-features.h
#
#kde-features.cpp: kde-features.h
#kde-features_parser.h kde-features_parser.cpp: kde-features.h
#
#kde-features.h kde-features.cpp kde-features_parser.h kde-features_parser.cpp: $(srcdir)/kde-features.rng \
#  $(top_builddir)/kode/kxml_compiler/kxml_compiler
#	$(top_builddir)/kode/kxml_compiler/kxml_compiler \
#          --external-parser $(srcdir)/kde-features.rng
#
#METASOURCES = AUTO
#DISTCLEANFILES = kde-features.h kde-features.cpp \
#	kde-features_parser.h kde-features_parser.cpp \
#	kde-features.h.backup kde-features.cpp.backup \
#	kde-features_parser.h.backup kde-features_parser.cpp.backup
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kres_featureplan.pot
