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

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "jobs.h"
#include "settings.h"

#include "passwordpage.h"

PasswordPage::PasswordPage( QWidget *parent )
  : QWidget( parent ), mJob( 0 )
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, 11, 6 );

  QLabel *label = new QLabel( i18n( "New password:" ), this );
  layout->addWidget( label, 0, 0 );

  mPassword = new QLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );
  label->setBuddy( mPassword );
  layout->addWidget( mPassword, 0, 1 );

  label = new QLabel( i18n( "Retype new password:" ), this );
  layout->addWidget( label, 1, 0 );

  mPasswordRetype = new QLineEdit( this );
  mPasswordRetype->setEchoMode( QLineEdit::Password );
  label->setBuddy( mPasswordRetype );
  layout->addWidget( mPasswordRetype, 1, 1 );

  mButton = new QPushButton( i18n( "Change" ), this );
  mButton->setEnabled( false );
  layout->addWidget( mButton, 2, 1 );

  layout->setRowSpacing( 3, 1 );

  connect( mPassword, SIGNAL( textChanged( const QString& ) ), this, SLOT( textChanged() ) );
  connect( mPasswordRetype, SIGNAL( textChanged( const QString& ) ), this, SLOT( textChanged() ) );
  connect( mButton, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
}

void PasswordPage::buttonClicked()
{
  if ( !mJob ) {
    if ( mPassword->text() != mPasswordRetype->text() ) {
      KMessageBox::error( this, i18n( "The two passwords differ!" ) );
      return;
    }

    mJob = Scalix::setPassword( Settings::self()->globalSlave(), Settings::self()->accountUrl(), mPassword->text() );
    connect( mJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( finished( KIO::Job* ) ) );

    updateState( true );
  } else {
    mJob->kill();
    mJob = 0;

    updateState( false );
  }
}

void PasswordPage::updateState( bool isWorking )
{
  if ( isWorking ) {
    mPassword->setEnabled( false );
    mPasswordRetype->setEnabled( false );
    mButton->setText( i18n( "Stop" ) );
  } else {
    mPassword->setEnabled( true );
    mPasswordRetype->setEnabled( true );
    mButton->setText( i18n( "Change" ) );
  }
}

void PasswordPage::textChanged()
{
  mButton->setEnabled( !mPassword->text().isEmpty() &&
                       !mPasswordRetype->text().isEmpty() );
}

void PasswordPage::finished( KIO::Job* job )
{
  updateState( false );

  if ( job->error() ) {
    KMessageBox::error( this, i18n( "Unable to change the password" ) + "\n" + job->errorString() );
    return;
  }

  KMessageBox::information( this, i18n( "Password was changed successfully" ) );
}

#include "passwordpage.moc"
