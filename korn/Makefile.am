KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = $(all_includes)
#INCLUDES = -I$(top_srcdir)/mimelib -I$(top_srcdir)/libkmime $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

bin_PROGRAMS = korn
korn_LDADD = $(LIB_KIO)
#korn_LDADD = $(top_builddir)/mimelib/libmimelib.la $(LIB_KFILE) $(LIBSOCKET) $(top_builddir)/libkmime/libkmime.la

korn_SOURCES = \
  accountmanager.cpp \
  boxcontainer.cpp \
  boxcontaineritem.cpp boxcontaineritem.skel \
  dcopdrop.cpp \
  dcopdropif.cpp dcopdropif.skel \
  dockedcontainer.cpp \
  dockeditem.cpp \
  hvcontainer.cpp \
  hvitem.cpp \
  keditlistboxman.cpp \
  maildrop.cpp \
  main.cpp \
  polldrop.cpp \
  protocols.cpp \
  kornshell.cpp \
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
  sortedmailsubject.cpp \
  korncfgimpl.cpp \
  kornaccountcfgimpl.cpp \
  kornboxcfgimpl.cpp \
  kornaccountcfg.ui \
  kornboxcfg.ui \
  korncfg.ui \
  systemtray.cpp \
  progress_dialog.ui 

noinst_HEADERS = \
  accountmanager.h \
  boxcontainer.h \
  boxcontaineritem.h \
  dcopdrop.h \
  dcopdropif.h \
  dockeditem.h \
  hvcontainer.h \
  hvitem.h \
  keditlistboxman.h \
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
  process_proto.h \
  qmail_proto.h \
  systemtray.h

srcdoc:
	kdoc -p -u "/~ssk/src/korn/" -n korn -d /home/ssk/web/src/korn *.h \
		-lqt -lkdeui -lkdecore

messages: rc.cpp
	$(XGETTEXT) *.{h,cpp} -o $(podir)/korn.pot

KDE_ICON = korn

xdg_apps_DATA = KOrn.desktop

SUBDIRS = kconf_update
