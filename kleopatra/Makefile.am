bin_PROGRAMS = certmanager
certmanager_SOURCES=main.cpp certmanager.cpp certbox.cpp certitem.cpp agent.cpp certificatewizard.ui certificatewizardimpl.cpp
noinst_HEADERS=certmanager.h certbox.h certitem.h agent.h certificatewizardimpl.h
METASOURCES = AUTO
INCLUDES =  -I$(top_srcdir)/libkdenetwork -I../kmail $(all_includes)

LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD    = ../libkdenetwork/libkdenetwork.la $(LIB_KIO) -lkdeprint

