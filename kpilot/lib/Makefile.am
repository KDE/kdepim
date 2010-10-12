### Makefile.am for kpilot/lib
###
###

METASOURCES = AUTO

INCLUDES = $(PISOCK_INCLUDE) -I$(top_srcdir) $(all_includes)

### If you must get debugging output on a platform where
### the libs are built without debugging support, define
### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
###
### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
##KDE_CXXFLAGS=-DNDEBUG -UDEBUG
##KDE_CXXFLAGS=-DDEBUG

lib_LTLIBRARIES = libkpilot.la

libkpilot_la_SOURCES = kpilotlibSettings.kcfgc \
	options.cc plugin.cc syncAction.cc \
	kpilotlink.cc kpilotdevicelink.cc kpilotlocallink.cc \
	actions.cc actionQueue.cc \
	pilot.cc \
	pilotAppInfo.cc pilotRecord.cc pilotDatabase.cc \
	pilotLocalDatabase.cc pilotSerialDatabase.cc \
	pilotMemo.cc \
	pilotAddress.cc \
	pilotDateEntry.cc \
	pilotTodoEntry.cc

libkpilot_la_LDFLAGS = $(PISOCK_LDFLAGS) -no-undefined $(all_libraries) $(KDE_EXTRA_RPATH) $(KDE_RPATH)
libkpilot_la_LIBADD = $(PISOCK_LIB) $(LIB_KDEUI) $(LIB_KABC) $(top_builddir)/libkcal/libkcal.la

kpilotincludedir = $(includedir)/kpilot
kpilotinclude_HEADERS = \
	kpilotlink.h kpilotlocallink.h kpilotdevicelink.h \
	pilot.h \
	pilotDatabase.h \
	pilotLinkVersion.h \
	pilotLocalDatabase.h \
	pilotRecord.h \
	pilotSerialDatabase.h \
	plugin.h \
	pluginfactory.h \
	syncAction.h


kde_kcfg_DATA = kpilotlib.kcfg

check-local:
	rm -f FAILED
	for i in $(srcdir)/*.h ; do \
		( echo "#include <kdemacros.h>" ; echo "#include \"$$i\"" ; echo "int main(int argc,char **argv){return 0;}" ) > header-test.cc; \
		echo "$$i" ; \
		g++ $(all_includes) -I$(top_builddir) -DQT_THREAD_SUPPORT -c header-test.cc || echo "$$i" >> FAILED; \
	done
	test ! -e FAILED

DOXYGEN_REFERENCES=libkcal kdecore
include $(top_srcdir)/admin/Doxyfile.am
