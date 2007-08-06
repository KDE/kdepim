KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = -I$(top_srcdir)/libkmime \
  	$(all_includes)
#INCLUDES = -I$(top_srcdir)/mimelib -I$(top_srcdir)/libkmime $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

bin_PROGRAMS = korn
korn_LDADD = $(LIB_KIO) $(top_builddir)/libkmime/libkmime.la
#korn_LDADD = $(top_builddir)/mimelib/libmimelib.la $(LIB_KFILE) $(LIBSOCKET) $(top_builddir)/libkmime/libkmime.la

korn_SOURCES = \
  accountmanager.cpp \
  account_input.cpp \
  boxcontainer.cpp \
  boxcontaineritem.cpp boxcontaineritem.skel \
  dcopdrop.cpp \
  dcopdropif.cpp dcopdropif.skel \
  dcop_proto.cpp \
  dockedcontainer.cpp \
  dockeditem.cpp \
  hvcontainer.cpp \
  hvitem.cpp \
  keditlistboxman.cpp \
  nntp_proto.cpp \
  maildrop.cpp \
  main.cpp \
  polldrop.cpp \
  protocols.cpp \
  kornshell.cpp \
  subjectsdlg.cpp \
  maildlg.cpp \
  mailid.cpp \
  mailsubject.cpp \
  mbox_proto.cpp \
  imap_proto.cpp \
  intid.cpp \
  stringid.cpp \
  kornapp.cpp \
  kio.cpp \
  kio_count.cpp \
  kio_proto.cpp \
  kio_subjects.cpp \
  kio_single_subject.cpp \
  kio_read.cpp \
  kio_delete.cpp \
  kmail_proto.cpp \
  sortedmailsubject.cpp \
  korncfgimpl.cpp \
  kornaccountcfgimpl.cpp \
  kornboxcfgimpl.cpp \
  kornaccountcfg.ui \
  kornboxcfg.ui \
  korncfg.ui \
  password.cpp \
  pop3_proto.cpp \
  systemtray.cpp \
  process_proto.cpp \
  progress_dialog.ui \
  qmail_proto.cpp

noinst_HEADERS = \
  accountmanager.h \
  account_input.h \
  boxcontainer.h \
  boxcontaineritem.h \
  dcop_proto.h \
  dcopdrop.h \
  dcopdropif.h \
  dockeditem.h \
  hvcontainer.h \
  hvitem.h \
  keditlistboxman.h \
  kmail_proto.h \
  kornaccountcfgimpl.h \
  kornboxcfg.h \
  kornboxcfgimpl.h \
  korncfg.h \
  korncfgimpl.h \
  maildrop.h \
  progress_dialog.ui.h \
  protocols.h \
  kornshell.h \
  subjectsdlg.h \
  maildlg.h \
  mailid.h \
  mailsubject.h \
  mbox_proto.h \
  intid.h \
  stringid.h \
  kornapp.h \
  kio.h \
  kio_count.h \
  kio_subjects.h \
  kio_single_subject.h \
  kio_read.h \
  kio_delete.h \
  sortedmailsubject.h \
  imap_proto.h \
  imaps_proto.h \
  kio_proto.h \
  maildir_proto.h \
  nntp_proto.h \
  pop3_proto.h \
  pop3s_proto.h \
  password.h \
  process_proto.h \
  qmail_proto.h \
  systemtray.h

srcdoc:
	kdoc -p -u "/~ssk/src/korn/" -n korn -d /home/ssk/web/src/korn *.h \
		-lqt -lkdeui -lkdecore

messages: rc.cpp
	$(XGETTEXT) *.h *.cpp -o $(podir)/korn.pot

KDE_ICON = korn

xdg_apps_DATA = KOrn.desktop

SUBDIRS = kconf_update
