/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "localkonnectorconfig.h"

#include <kconfig.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

using namespace KSync;

LocalKonnectorConfig::LocalKonnectorConfig( QWidget *parent )
  : ConfigWidget( parent, 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  
  KConfig cfg( "localkonnectorrc" );
  QString calendarFile = cfg.readPathEntry( "CalendarFile" );
  QString addressBookFile = cfg.readPathEntry( "AddressBookFile" );
  
  mCalendarFile = new KURLRequester( calendarFile, this );
  topLayout->addWidget( mCalendarFile );
  
  mAddressBookFile = new KURLRequester( addressBookFile, this );
  topLayout->addWidget( mAddressBookFile );

  QPushButton *button = new QPushButton( i18n("Save"), this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( writeSettings() ) );
}

LocalKonnectorConfig::~LocalKonnectorConfig()
{
}

void LocalKonnectorConfig::writeSettings()
{
  KConfig cfg( "localkonnectorrc" );
  cfg.writePathEntry( "CalendarFile", mCalendarFile->url() );
  cfg.writePathEntry( "AddressBookFile", mAddressBookFile->url() );
}

#include "localkonnectorconfig.moc"
