kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kpilot_LIB_SRCS 
   options.cc 
   uiDialog.cc 
   plugin.cc 
   syncAction.cc 
   kpilotlink.cc 
   pilotRecord.cc 
   pilotDatabase.cc 
   pilotLocalDatabase.cc 
   pilotSerialDatabase.cc 
   pilotMemo.cc 
   pilotAddress.cc 
   pilotDateEntry.cc 
   pilotTodoEntry.cc 
   recordConduit.cc )

kde4_automoc(${kpilot_LIB_SRCS})

kde4_add_kcfg_files(kpilot_LIB_SRCS kpilotlibSettings.kcfgc )

kde4_add_library(kpilot SHARED ${kpilot_LIB_SRCS})

target_link_libraries(kpilot  ${QT_AND_KDECORE_LIBS} kdeui kcal )

set_target_properties(kpilot PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kpilot )


########### next target ###############

set(testactions_SRCS testactions.cc )

kde4_automoc(${testactions_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testactions ${testactions_SRCS})

target_link_libraries(testactions  ${QT_AND_KDECORE_LIBS} kpilot )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testconstants_SRCS testconstants.cc )

kde4_automoc(${testconstants_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testconstants ${testconstants_SRCS})

target_link_libraries(testconstants  ${QT_AND_KDECORE_LIBS} kpilot )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testdatabase_SRCS testdatabase.cc )

kde4_automoc(${testdatabase_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testdatabase ${testdatabase_SRCS})

target_link_libraries(testdatabase  ${QT_AND_KDECORE_LIBS} kpilot )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES kpilotlib.kcfg )
install_files( /include/kpilot FILES kpilotlink.h uiDialog.h  	plugin.h pluginfactory.h syncAction.h  	pilotRecord.h pilotDatabase.h  	pilotLocalDatabase.h pilotSerialDatabase.h )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile.am for kpilot/lib
####
####
#
#METASOURCES = AUTO
#
#INCLUDES = $(PISOCK_INCLUDE) -I$(top_srcdir) $(all_includes)
#
#### If you must get debugging output on a platform where
#### the libs are built without debugging support, define
#### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
####
#### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
###KDE_CXXFLAGS=-DNDEBUG -UDEBUG
###KDE_CXXFLAGS=-DDEBUG
#
#lib_LTLIBRARIES = libkpilot.la
#
#libkpilot_la_SOURCES = kpilotlibSettings.kcfgc options.cc uiDialog.cc plugin.cc syncAction.cc \
#	kpilotlink.cc \
#	pilotRecord.cc pilotDatabase.cc \
#	pilotLocalDatabase.cc pilotSerialDatabase.cc \
#	pilotMemo.cc pilotAddress.cc \
#	pilotDateEntry.cc pilotTodoEntry.cc \
#	recordConduit.cc
#
#libkpilot_la_LDFLAGS = -no-undefined $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_EXTRA_RPATH) $(KDE_RPATH)
#libkpilot_la_LIBADD = $(LIB_KDEUI) $(PISOCK_LIB) $(LIB_KABC) $(top_builddir)/libkcal/libkcal.la
#
#kpilotincludedir = $(includedir)/kpilot
#kpilotinclude_HEADERS = kpilotlink.h uiDialog.h \
#	plugin.h pluginfactory.h syncAction.h \
#	pilotRecord.h pilotDatabase.h \
#	pilotLocalDatabase.h pilotSerialDatabase.h
#
#kde_kcfg_DATA = kpilotlib.kcfg
#
#check_PROGRAMS = testactions testconstants testdatabase
#TESTS = testactions testconstants testdatabase
#
#testactions_SOURCES = testactions.cc
#testactions_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#testactions_LDADD  = libkpilot.la $(PISOCK_LIB)
#
#testconstants_SOURCES = testconstants.cc
#testconstants_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#testconstants_LDADD   = libkpilot.la $(PISOCK_LIB)
#
#testdatabase_SOURCES  = testdatabase.cc
#testdatabase_LDFLAGS  = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#testdatabase_LDADD    = libkpilot.la $(PISOCK_LIB)
#testdatabase_CXXFLAGS = '-DSOURCE="$(srcdir)"'
#
#check-local:
#	rm -f FAILED
#	for i in $(srcdir)/*.h ; do \
#		( echo "#include <kdemacros.h>" ; echo "#include \"$$i\"" ; echo "int main(int argc,char **argv){return 0;}" ) > header-test.cc; \
#		echo "$$i" ; \
#		g++ $(all_includes) $(PISOCK_INCLUDE) -c header-test.cc || echo "$$i" >> FAILED; \
#	done
#	test ! -e FAILED
#
