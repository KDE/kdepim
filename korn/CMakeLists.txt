project(korn)

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
   qmail_proto.cpp
   settings.cpp )

# run moc on label.h and add the resulting stuff to the korn_SRCS list
qt4_wrap_cpp(korn_SRCS label.h)


kde4_add_ui_files(korn_SRCS 
   kornaccountcfg.ui 
   kornboxcfg.ui 
   korncfg.ui 
   progress_dialog.ui )

qt4_add_dbus_adaptor(korn_SRCS org.kde.korn.BoxContainerItem.xml boxcontaineritem.h BoxContainerItem )
qt4_add_dbus_adaptor(korn_SRCS org.kde.korn.MailDrop.xml dbusdrop.h DBUSDrop )

kde4_add_executable(korn ${korn_SRCS})

target_link_libraries(korn  ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} ${KDE4_KDE3SUPPORT_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS korn  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES KOrn.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES org.kde.korn.BoxContainerItem.xml org.kde.korn.MailDrop.xml DESTINATION ${DBUS_INTERFACES_DIR} )


kde4_install_icons( ${ICON_INSTALL_DIR}   )
