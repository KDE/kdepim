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

#include <tqapplication.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstringhandler.h>
#include <kwallet.h>

#include "jobs.h"
#include "settings.h"

#include "passwordpage.h"

PasswordPage::PasswordPage( TQWidget *parent )
  : TQWidget( parent ), mJob( 0 )
{
  TQGridLayout *layout = new TQGridLayout( this, 2, 3, 11, 6 );

  TQLabel *label = new TQLabel( i18n( "New password:" ), this );
  layout->addWidget( label, 0, 0 );

  mPassword = new TQLineEdit( this );
  mPassword->setEchoMode( TQLineEdit::Password );
  label->setBuddy( mPassword );
  layout->addWidget( mPassword, 0, 1 );

  label = new TQLabel( i18n( "Retype new password:" ), this );
  layout->addWidget( label, 1, 0 );

  mPasswordRetype = new TQLineEdit( this );
  mPasswordRetype->setEchoMode( TQLineEdit::Password );
  label->setBuddy( mPasswordRetype );
  layout->addWidget( mPasswordRetype, 1, 1 );

  mButton = new TQPushButton( i18n( "Change" ), this );
  mButton->setEnabled( false );
  layout->addWidget( mButton, 2, 1 );

  layout->setRowSpacing( 3, 1 );

  connect( mPassword, TQT_SIGNAL( textChanged( const TQString& ) ), this, TQT_SLOT( textChanged() ) );
  connect( mPasswordRetype, TQT_SIGNAL( textChanged( const TQString& ) ), this, TQT_SLOT( textChanged() ) );
  connect( mButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( buttonClicked() ) );
}

void PasswordPage::buttonClicked()
{
  if ( !mJob ) {
    if ( mPassword->text() != mPasswordRetype->text() ) {
      KMessageBox::error( this, i18n( "The two passwords differ!" ) );
      return;
    }

    mJob = Scalix::setPassword( Settings::self()->globalSlave(), Settings::self()->accountUrl(),
                                Settings::self()->accountPassword(), mPassword->text() );
    connect( mJob, TQT_SIGNAL( result( KIO::Job* ) ), this, TQT_SLOT( finished( KIO::Job* ) ) );

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
  mJob = 0;

  updateState( false );

  if ( job->error() ) {
    KMessageBox::error( this, i18n( "Unable to change the password" ) + "\n" + job->errorString() );
    return;
  }

  // Update configuration files to the new password as well

  const TQString newPassword = mPassword->text();

  { // ScalixAdmin config
    KConfig config( "scalixadminrc" );
    KConfigGroup group( &config, "Account" );
    group.writeEntry( "pass", KStringHandler::obscure( newPassword ) );
  }

  { // ScalixWizard config
    KConfig config( "scalixrc" );
    KConfigGroup group( &config, "General" );
    group.writeEntry( "Password", KStringHandler::obscure( newPassword ) );
  }

  { // KMail config
    KConfig config( "kmailrc" );

    // Try to find account group for Scalix
    TQString scalixAccount;
    const TQStringList groupList = config.groupList();
    for ( uint i = 0; i < groupList.count(); ++i ) {
      if ( groupList[ i ].startsWith( "Account " ) ) {
        KConfigGroup group( &config, groupList[ i ] );
        if ( group.hasKey( "groupwareType" ) && group.readNumEntry( "groupwareType" ) == 2 ) {
          scalixAccount = groupList[ i ];
          break;
        }
      }
    }

    if ( scalixAccount.isEmpty() ) {
      qWarning( "No Scalix Groupware Account found in kmailrc!" );
      return;
    }

    const int accountId = scalixAccount.mid( 8 ).toInt();

    KConfigGroup group( &config, scalixAccount );

    // Save only if the user choose it before
    bool storePassword = group.readBoolEntry( "store-passwd", false );
    if ( storePassword ) {
      // First try to store in KWallet
      if ( KWallet::Wallet::isEnabled() ) {
        WId window = 0;
        if ( qApp->activeWindow() )
          window = qApp->activeWindow()->winId();

        KWallet::Wallet *wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), window );
        if ( wallet ) {
          if ( !wallet->hasFolder( "kmail" ) )
            wallet->createFolder( "kmail" );
          wallet->setFolder( "kmail" );
          wallet->writePassword( "account-" + TQString::number( accountId ), newPassword );
        }
      } else {
        group.writeEntry( "pass", KStringHandler::obscure( newPassword ) );
      }

      KConfigGroup fileGroup( &config, TQString( "Folder-%1" ).arg( group.readNumEntry( "Folder" ) ) );
      fileGroup.writeEntry( "pass", KStringHandler::obscure( newPassword ) );
    }
  }

  KMessageBox::information( this, i18n( "Password was changed successfully" ) );
}

#include "passwordpage.moc"
