SUBDIRS = pics

lib_LTLIBRARIES = libkpgp.la

libkpgp_la_SOURCES = \
			  kpgpblock.cpp \
                          kpgpui.cpp \
                          kpgpkey.cpp \
                          kpgp.cpp \
                          kpgpbase.cpp \
                          kpgpbaseG.cpp \
                          kpgpbase2.cpp \
                          kpgpbase5.cpp \
                          kpgpbase6.cpp

libkpgp_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 4:0:2

libkpgp_la_LIBADD = $(LIB_KDEUI) $(LIB_POLL)

METASOURCES = AUTO

updatedir = $(kde_datadir)/kconf_update
update_DATA = kpgp.upd
update_SCRIPTS = kpgp-3.1-upgrade-address-data.pl

messages: 
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkpgp.pot

include ../admin/Doxyfile.am
