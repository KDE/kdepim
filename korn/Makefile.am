KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = -I$(top_srcdir)/mimelib -I$(top_srcdir)/libkmime $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

bin_PROGRAMS = korn
korn_LDADD = $(top_builddir)/mimelib/libmimelib.la $(LIB_KFILE) $(LIBSOCKET) $(top_builddir)/libkmime/libkmime.la

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
  subjectsdlg.cpp \
  maildlg.cpp \
  mailid.cpp \
  mailsubject.cpp \
  intid.cpp \
  stringid.cpp \
  kornapp.cpp \
  kio.cpp \
  kio_count.cpp \
  kio_subjects.cpp \
  kio_single_subject.cpp \
  kio_read.cpp \
  kio_delete.cpp \
  kiocfg.cpp \
  sortedmailsubject.cpp

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
  subjectsdlg.h \
  maildlg.h \
  mailid.h \
  mailsubject.h \
  intid.h \
  stringid.h \
  kornapp.h \
  kio.h \
  kio_count.h \
  kio_subjects.h \
  kio_single_subject.h \
  kio_read.h \
  kio_delete.h \
  kiocfg.h \
  sortedmailsubject.h \
  imap_proto.h \
  imaps_proto.h \
  kio_proto.h \
  maildir_proto.h \
  nntp_proto.h \
  pop3_proto.h \
  pop3s_proto.h \
  process_proto.h \
  qmail_proto.h 

srcdoc:
	kdoc -p -u "/~ssk/src/korn/" -n korn -d /home/ssk/web/src/korn *.h \
		-lqt -lkdeui -lkdecore

messages:
	$(XGETTEXT) *.{h,cpp} -o $(podir)/korn.pot

KDE_ICON = korn

xdg_apps_DATA = KOrn.desktop
