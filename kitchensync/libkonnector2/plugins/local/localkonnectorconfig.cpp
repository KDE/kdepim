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

#include "localkonnector.h"

#include <libkcal/resourcelocal.h>

#include <kconfig.h>
#include <klocale.h>
#include <kabc/resourcefile.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klineedit.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

using namespace KSync;

LocalKonnectorConfig::LocalKonnectorConfig( QWidget *parent )
  : KRES::ConfigWidget( parent, 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  topLayout->addWidget( new QLabel( i18n("Calendar file:"), this ) );

  mCalendarFile = new KURLRequester( this );
  mCalendarFile->setMode( KFile::File | KFile::LocalOnly );
  topLayout->addWidget( mCalendarFile );

  QPushButton *button =
      new QPushButton( i18n("Select From Existing Calendars..."), this );
  connect( button, SIGNAL( clicked() ), SLOT( selectCalendarResource() ) );
  topLayout->addWidget( button );

  topLayout->addSpacing( 4 );

  topLayout->addWidget( new QLabel( i18n("Address book file:"), this ) );

  mAddressBookFile = new KURLRequester( this );
  mAddressBookFile->setMode( KFile::File | KFile::LocalOnly );
  topLayout->addWidget( mAddressBookFile );

  button = new QPushButton( i18n("Select From Existing Address Books..."), this );
  connect( button, SIGNAL( clicked() ), SLOT( selectAddressBookResource() ) );
  topLayout->addWidget( button );
}

LocalKonnectorConfig::~LocalKonnectorConfig()
{
}

void LocalKonnectorConfig::loadSettings( KRES::Resource *r )
{
  LocalKonnector *konnector = dynamic_cast<LocalKonnector *>( r );
  if ( konnector ) {
    mCalendarFile->setURL( konnector->calendarFile() );
    mAddressBookFile->setURL( konnector->addressBookFile() );
  }
}

void LocalKonnectorConfig::saveSettings( KRES::Resource *r )
{
  LocalKonnector *konnector = dynamic_cast<LocalKonnector *>( r );
  if ( konnector ) {
    konnector->setCalendarFile( mCalendarFile->url() );
    konnector->setAddressBookFile( mAddressBookFile->url() );
  }
}

void LocalKonnectorConfig::selectAddressBookResource()
{
  QStringList files;

  KRES::Manager<KABC::Resource> manager( "contact" );
  manager.readConfig();

  KRES::Manager<KABC::Resource>::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    if ( (*it)->inherits( "KABC::ResourceFile" ) ) {
      KABC::ResourceFile *r = static_cast<KABC::ResourceFile *>( *it );
      files.append( r->fileName() );
    }
  }

  if ( files.isEmpty() ) {
    KMessageBox::sorry( this, i18n("No file resources found.") );
  } else {
    QString file = KInputDialog::getItem( i18n("Select File"),
        i18n("Please select an addressbook file:"), files, 0, false, 0, this );
    if ( !file.isEmpty() ) {
      mAddressBookFile->lineEdit()->setText( file );
    }
  }
}

void LocalKonnectorConfig::selectCalendarResource()
{
  QStringList files;

  KCal::CalendarResourceManager manager( "calendar" );
  manager.readConfig();

  KCal::CalendarResourceManager::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    if ( (*it)->inherits( "KCal::ResourceLocal" ) ) {
      KCal::ResourceLocal *r = static_cast<KCal::ResourceLocal *>( *it );
      files.append( r->fileName() );
    }
  }

  if ( files.isEmpty() ) {
    KMessageBox::sorry( this, i18n("No file resources found.") );
  } else {
    QString file = KInputDialog::getItem( i18n("Select File"),
        i18n("Please select a calendar file:"), files, 0, false, 0, this );
    if ( !file.isEmpty() ) {
      mCalendarFile->lineEdit()->setText( file );
    }
  }
}

#include "localkonnectorconfig.moc"
