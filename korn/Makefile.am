KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = -I$(top_srcdir)/mimelib $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

bin_PROGRAMS = korn
korn_LDADD = $(top_builddir)/mimelib/libmimelib.la $(LIB_KFILE) $(LIBSOCKET)

korn_SOURCES = \
  dropdlg.cpp \
  dropman.cpp \
  ecfg.cpp \
  edrop.cpp \
  gencfg.cpp \
  comcfg.cpp \
  imap.cpp \
  imapcfg.cpp \
  kbiffimap.cpp \
  kfbrowsebtn.cpp \
  kornbutt.cpp \
  kornset.cpp \
  maildrop.cpp \
  main.cpp \
  moncfg.cpp \
  news.cpp \
  newscfg.cpp \
  optdlg.cpp \
  pollcfg.cpp \
  polldrop.cpp \
  pop.cpp \
  popcfg.cpp \
  qmail.cpp \
  qmailcfg.cpp \
  shell.cpp \
  typedlg.cpp \
  unixcfg.cpp \
  unixdrop.cpp \
  btnstyle.cpp \
  kornapp.cpp

noinst_HEADERS = \
  dropdlg.h \
  dropman.h \
  ecfg.h \
  edrop.h \
  gencfg.h \
  comcfg.h \
  imap.h \
  imapcfg.h \
  kbiffimap.h \
  kfbrowsebtn.h \
  kornbutt.h \
  kornset.h \
  maildrop.h \
  moncfg.h \
  news.h \
  newscfg.h \
  optdlg.h \
  pollcfg.h \
  polldrop.h \
  pop.h \
  popcfg.h \
  qmail.h \
  qmailcfg.h \
  shell.h \
  typedlg.h \
  unixcfg.h \
  unixdrop.h \
  btnstyle.h \
  kornapp.h

srcdoc:
	kdoc -p -u "/~ssk/src/korn/" -n korn -d /home/ssk/web/src/korn *.h \
		-lqt -lkdeui -lkdecore

messages:
	$(XGETTEXT) $(korn_SOURCES) -o $(podir)/korn.pot

KDE_ICON = korn

appsdir = $(kde_appsdir)/Internet/More
apps_DATA = KOrn.desktop

