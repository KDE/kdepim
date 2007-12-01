/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>

#include <kdbusservicestarter.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <unistd.h>

#include "jobs.h"
#include "ldapdialog.h"
#include "otheruserview.h"
#include "settings.h"

#include "otheruserpage.h"

OtherUserPage::OtherUserPage( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 11 );
  layout->setSpacing( 6 );

  mView = new OtherUserView( &mManager, this );
  layout->addWidget( mView, 0, 0, 1, 2 );

  mAddButton = new QPushButton( i18n( "Add Account..." ), this );
  layout->addWidget( mAddButton, 1, 0 );

  mDeleteButton = new QPushButton( i18n( "Remove Account" ), this );
  mDeleteButton->setEnabled( false );
  layout->addWidget( mDeleteButton, 1, 1 );

  connect( mView, SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( addUser() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( removeUser() ) );

  loadAllUsers();
}

OtherUserPage::~OtherUserPage()
{
}

void OtherUserPage::loadAllUsers()
{
  Scalix::GetOtherUsersJob *job = Scalix::getOtherUsers( Settings::self()->globalSlave(),
                                                         Settings::self()->accountUrl() );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( allUsers( KJob* ) ) );
}

void OtherUserPage::addUser()
{
  LdapDialog dlg( this );
  if ( !dlg.exec() )
    return;

  const QString email = dlg.selectedUser();
  if ( email.isEmpty() )
    return;

  Scalix::AddOtherUserJob *job = Scalix::addOtherUser( Settings::self()->globalSlave(),
                                                       Settings::self()->accountUrl(), email );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( userAdded( KJob* ) ) );
}

void OtherUserPage::removeUser()
{
  const QString email = mView->selectedUser();
  if ( email.isEmpty() )
    return;

  Scalix::DeleteOtherUserJob *job = Scalix::deleteOtherUser( Settings::self()->globalSlave(),
                                                             Settings::self()->accountUrl(), email );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( userRemoved( KJob* ) ) );
}

void OtherUserPage::allUsers( KJob *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );

  Scalix::GetOtherUsersJob *userJob = static_cast<Scalix::GetOtherUsersJob*>( job );

  mManager.clear();

  const QStringList users = userJob->otherUsers();
  for ( int i = 0; i < users.count(); ++i )
    mManager.addOtherUser( users[ i ] );

  selectionChanged();
}

void OtherUserPage::userAdded( KJob *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
  else
    loadAllUsers(); // update the GUI

  updateKmail();
}

void OtherUserPage::userRemoved( KJob *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
  else
    loadAllUsers(); // update the GUI

  updateKmail();
}

void OtherUserPage::selectionChanged()
{
  if ( !mView->selectionModel() )
    return;

  mDeleteButton->setEnabled( mView->selectionModel()->selectedIndexes().count() != 0 );
}

void OtherUserPage::updateKmail()
{
  QMessageBox *msg = new QMessageBox( qApp->activeWindow() );
  msg->setText( i18n( "Updating account..." ) );
  msg->show();
  qApp->processEvents();
  sleep( 1 );
  qApp->processEvents();

  QString error;
  QString dbusService;
  int result = KDBusServiceStarter::self()->
    findServiceFor( "DBUS/ResourceBackend/IMAP", QString(),
                    &error, &dbusService );
  if ( result != 0 ) {
    KMessageBox::error( 0, i18n( "Unable to start KMail to trigger account update with Scalix server" ) );
    delete msg;
    return;
  }

  QDBusInterface ref(dbusService, "/KMail" , "org.kde.kmail.kmail", QDBusConnection::sessionBus());

  // loop until dcop iface is set up correctly
  QStringList list;
  while ( list.isEmpty() ) {
    QDBusReply<QStringList> ret = ref.call( "accounts" );
    list = ret;
  }

  ref.call( "checkAccount", i18n( "Scalix Server" ) );

  delete msg;
}

#include "otheruserpage.moc"
