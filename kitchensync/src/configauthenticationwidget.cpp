/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QtGui/QFormLayout>

#include <klineedit.h>
#include <klocale.h>

#include "configauthenticationwidget.h"

ConfigAuthenticationWidget::ConfigAuthenticationWidget( const QSync::PluginAuthentication &authentication,
                                                        QWidget *parent )
  : QWidget( parent ),
    mAuthentication( authentication ),
    mUserName( 0 ), mPassword( 0 ), mReference( 0 )
{
  QFormLayout *layout = new QFormLayout( this );

  if ( authentication.isOptionSupported( QSync::PluginAuthentication::UserNameOption ) ) {
    mUserName = new KLineEdit( this );
    layout->addRow( i18n( "User:" ), mUserName );
  }

  if ( authentication.isOptionSupported( QSync::PluginAuthentication::PasswordOption ) ) {
    mPassword = new KLineEdit( this );
    mPassword->setEchoMode( KLineEdit::Password );
    layout->addRow( i18n( "Password:" ), mPassword );
  }

  if ( authentication.isOptionSupported( QSync::PluginAuthentication::ReferenceOption ) ) {
    mReference = new KLineEdit( this );
    layout->addRow( i18n( "Reference:" ), mReference );
  }
}

void ConfigAuthenticationWidget::load()
{
  if ( mUserName )
    mUserName->setText( mAuthentication.userName() );

  if ( mPassword )
    mPassword->setText( mAuthentication.password() );

  if ( mReference )
    mReference->setText( mAuthentication.reference() );
}

void ConfigAuthenticationWidget::save()
{
  if ( mUserName )
    mAuthentication.setUserName( mUserName->text() );

  if ( mPassword )
    mAuthentication.setPassword( mPassword->text() );

  if ( mReference )
    mAuthentication.setReference( mReference->text() );
}
