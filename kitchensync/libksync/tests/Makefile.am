INCLUDES = -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir) $(all_includes)

check_PROGRAMS = synctest1 synctest2 synctest3 synctest4 synctest5

METASOURCES = AUTO

synctest1_SOURCES = synctest1.cpp synctesthelper.cpp
synctest1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest1_LDADD = ../libksync2.la

synctest2_SOURCES = synctest2.cpp synctesthelper.cpp
synctest2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest2_LDADD = ../libksync2.la

synctest3_SOURCES = synctest3.cpp synctesthelper.cpp
synctest3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest3_LDADD = ../libksync2.la -lkabc_file

synctest4_SOURCES = synctest4.cpp synctesthelper.cpp
synctest4_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest4_LDADD = ../libksync2.la -lkabc_file

synctest5_SOURCES = synctest5.cpp synctesthelper.cpp
synctest5_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest5_LDADD = ../libksync2.la $(KDE_KIO)

check: synctest1 synctest2
	mkdir -p ./data
	./synctest1 ./data
	./synctest2 ./data
	./synctest3 ./data
	./synctest4 ./data
	./synctest5 ./data
	perl $(srcdir)/checksync.pl ./data $(srcdir)/data 
