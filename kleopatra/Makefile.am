bin_PROGRAMS = kgpgcertmanager
kgpgcertmanager_SOURCES=main.cpp certmanager.cpp certbox.cpp certitem.cpp agent.cpp \
	certificatewizard.ui certificatewizardimpl.cpp \
	certificateinfowidget.ui certificateinfowidgetimpl.cpp 
noinst_HEADERS=certmanager.h certbox.h certitem.h agent.h \
	certificatewizardimpl.h \
	certificateinfowidgetimpl.h
METASOURCES = AUTO
INCLUDES =  -I$(top_srcdir)/libkdenetwork -I../kmail $(all_includes)

LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD    = ../libkdenetwork/libkdenetwork.la $(LIB_KIO) -lkdeprint

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kpgpcertmanager.pot
