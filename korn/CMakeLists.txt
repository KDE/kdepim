
add_subdirectory( kconf_update ) 




########### next target ###############

set(korn_SRCS 
   accountmanager.cpp 
   account_input.cpp 
   boxcontainer.cpp 
   boxcontaineritem.cpp 
   dbusdrop.cpp 
   dbus_proto.cpp 
   dockedcontainer.cpp 
   dockeditem.cpp 
   hvcontainer.cpp 
   hvitem.cpp 
   keditlistboxman.cpp 
   nntp_proto.cpp 
   maildrop.cpp 
   main.cpp 
   polldrop.cpp 
   process_drop.cpp 
   protocols.cpp 
   kornshell.cpp 
   subjectsdlg.cpp 
   maildlg.cpp 
   mailsubject.cpp 
   mbox_proto.cpp 
   imap_proto.cpp 
   kornapp.cpp 
   kio.cpp 
   kio_conn.cpp 
   kio_count.cpp 
   kio_proto.cpp 
   kio_subjects.cpp 
   kio_single_subject.cpp 
   kio_read.cpp 
   kio_delete.cpp 
   kmail_proto.cpp 
   korncfgimpl.cpp 
   kornaccountcfgimpl.cpp 
   kornboxcfgimpl.cpp 
   password.cpp 
   pop3_proto.cpp 
   systemtray.cpp 
   process_proto.cpp 
   progress_dialog_impl.cpp 
   qmail_proto.cpp )

# run moc on label.h and add the resulting stuff to the korn_SRCS list
qt4_wrap_cpp(korn_SRCS label.h)

kde4_automoc(${korn_SRCS})

kde4_add_ui_files(korn_SRCS 
   kornaccountcfg.ui 
   kornboxcfg.ui 
   korncfg.ui 
   progress_dialog.ui )

qt4_add_dbus_adaptor(korn_SRCS org.kde.korn.BoxContainerItem.xml boxcontaineritem.h BoxContainerItem )
qt4_add_dbus_adaptor(korn_SRCS org.kde.korn.MailDrop.xml dbusdrop.h DBUSDrop )

kde4_add_executable(korn ${korn_SRCS})

target_link_libraries(korn  ${KDE4_KIO_LIBS} phononcore)

install(TARGETS korn  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES KOrn.desktop  DESTINATION ${XDG_APPS_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(USE_RTTI)
#
#INCLUDES = $(all_includes)
##INCLUDES = -I$(top_srcdir)/mimelib -I$(top_srcdir)/libkmime $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#METASOURCES = AUTO
#
#bin_PROGRAMS = korn
#korn_LDADD = $(LIB_KIO)
##korn_LDADD = $(top_builddir)/mimelib/libmimelib.la $(LIB_KFILE) $(LIBSOCKET) $(top_builddir)/libkmime/libkmime.la
#
#korn_SOURCES = \
#  accountmanager.cpp \
#  account_input.cpp \
#  boxcontainer.cpp \
#  boxcontaineritem.cpp boxcontaineritem.skel \
#  dbusdrop.cpp \
#  dbusdropif.cpp dbusdropif.skel \
#  dbus_proto.cpp \
#  dockedcontainer.cpp \
#  dockeditem.cpp \
#  hvcontainer.cpp \
#  hvitem.cpp \
#  keditlistboxman.cpp \
#  nntp_proto.cpp \
#  maildrop.cpp \
#  main.cpp \
#  polldrop.cpp \
#  process_drop.cpp \
#  protocols.cpp \
#  kornshell.cpp \
#  subjectsdlg.cpp \
#  maildlg.cpp \
#  mailsubject.cpp \
#  mbox_proto.cpp \
#  imap_proto.cpp \
#  kornapp.cpp \
#  kio.cpp \
#  kio_conn.cpp \
#  kio_count.cpp \
#  kio_proto.cpp \
#  kio_subjects.cpp \
#  kio_single_subject.cpp \
#  kio_read.cpp \
#  kio_delete.cpp \
#  kmail_proto.cpp \
#  korncfgimpl.cpp \
#  kornaccountcfgimpl.cpp \
#  kornboxcfgimpl.cpp \
#  kornaccountcfg.ui \
#  kornboxcfg.ui \
#  korncfg.ui \
#  password.cpp \
#  pop3_proto.cpp \
#  systemtray.cpp \
#  process_proto.cpp \
#  progress_dialog.ui \
#  progress_dialog_impl.cpp \
#  qmail_proto.cpp
#
#noinst_HEADERS = \
#  accountmanager.h \
#  account_input.h \
#  boxcontainer.h \
#  boxcontaineritem.h \
#  dbus_proto.h \
#  dbusdrop.h \
#  dbusdropif.h \
#  dockeditem.h \
#  hvcontainer.h \
#  hvitem.h \
#  keditlistboxman.h \
#  kmail_proto.h \
#  kornaccountcfgimpl.h \
#  kornboxcfg.h \
#  kornboxcfgimpl.h \
#  korncfg.h \
#  korncfgimpl.h \
#  maildrop.h \
#  process_drop.h \
#  progress_dialog.ui.h \
#  protocols.h \
#  kornshell.h \
#  subjectsdlg.h \
#  maildlg.h \
#  mailsubject.h \
#  mbox_proto.h \
#  kornapp.h \
#  kio.h \
#  kio_conn.h \
#  kio_count.h \
#  kio_subjects.h \
#  kio_single_subject.h \
#  kio_read.h \
#  kio_delete.h \
#  imap_proto.h \
#  kio_proto.h \
#  nntp_proto.h \
#  pop3_proto.h \
#  password.h \
#  process_proto.h \
#  progress_dialog_impl.h \
#  qmail_proto.h \
#  systemtray.h
#
#srcdoc:
#	kdoc -p -u "/~ssk/src/korn/" -n korn -d /home/ssk/web/src/korn *.h \
#		-lqt -lkdeui -lkdecore
#
#messages: rc.cpp
#	$(XGETTEXT) *.{h,cpp} -o $(podir)/korn.pot
#
#KDE_ICON = korn
#
#xdg_apps_DATA = KOrn.desktop
#
#SUBDIRS = kconf_update
