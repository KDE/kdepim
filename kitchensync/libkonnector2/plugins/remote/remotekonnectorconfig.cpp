/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "remotekonnectorconfig.h"

#include "remotekonnector.h"

#include <libkcal/resourcelocal.h>

#include <kconfig.h>
#include <klocale.h>
#include <kabc/resourcefile.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klineedit.h>

#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

using namespace KSync;

RemoteKonnectorConfig::RemoteKonnectorConfig( QWidget *parent )
  : KRES::ConfigWidget( parent, 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QPushButton *button = new QPushButton( i18n("Standard Setup..."), this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( setupStandard() ) );

  topLayout->addWidget( new QLabel( i18n("Calendar file:"), this ) );

  mCalendarUrl = new KURLRequester( this );
  mCalendarUrl->setMode( KFile::File );
  topLayout->addWidget( mCalendarUrl );

  topLayout->addSpacing( 4 );

  topLayout->addWidget( new QLabel( i18n("Address book file:"), this ) );

  mAddressBookUrl = new KURLRequester( this );
  mAddressBookUrl->setMode( KFile::File );
  topLayout->addWidget( mAddressBookUrl );
}

RemoteKonnectorConfig::~RemoteKonnectorConfig()
{
}

void RemoteKonnectorConfig::loadSettings( KRES::Resource *r )
{
  RemoteKonnector *konnector = dynamic_cast<RemoteKonnector *>( r );
  if ( konnector ) {
    mCalendarUrl->setURL( konnector->calendarUrl() );
    mAddressBookUrl->setURL( konnector->addressBookUrl() );
  }
}

void RemoteKonnectorConfig::saveSettings( KRES::Resource *r )
{
  RemoteKonnector *konnector = dynamic_cast<RemoteKonnector *>( r );
  if ( konnector ) {
    konnector->setCalendarUrl( mCalendarUrl->url() );
    konnector->setAddressBookUrl( mAddressBookUrl->url() );
  }
}

void RemoteKonnectorConfig::setupStandard()
{
  bool ok = false;

  QString hostname = QInputDialog::getText( i18n( "Remote Host" ), i18n( "Enter remote host name" ),
                                            QLineEdit::Normal, QString::null, &ok, this );

  if ( hostname.isEmpty() || !ok )
    return;

  QString username = QInputDialog::getText( i18n( "Remote User" ), i18n( "Enter remote user name" ),
                                            QLineEdit::Normal, QString::null, &ok, this );

  if ( username.isEmpty() || !ok )
    return;

  QString urlBase = "fish://" + hostname + "/~" + username + "/";
  mCalendarUrl->setURL( urlBase + ".kde/share/apps/korganizer/std.ics" );
  mAddressBookUrl->setURL( urlBase + ".kde/share/apps/kabc/std.vcf" );
}

#include "remotekonnectorconfig.moc"
