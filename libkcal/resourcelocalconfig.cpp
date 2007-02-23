/*
    This file is part of libkcal.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"

#include "resourcelocal.h"

#include "resourcelocalconfig.h"

using namespace KCal;

ResourceLocalConfig::ResourceLocalConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  mURL = new KURLRequester( this );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mURL, 1, 1 );

  formatGroup = new QButtonGroup( 1, Horizontal, i18n( "Calendar Format" ), this );

  icalButton = new QRadioButton( i18n("iCalendar"), formatGroup );
  vcalButton = new QRadioButton( i18n("vCalendar"), formatGroup );

  mainLayout->addWidget( formatGroup, 2, 1 );
}

void ResourceLocalConfig::loadSettings( KRES::Resource *resource )
{
  ResourceLocal* res = static_cast<ResourceLocal*>( resource );
  if ( res ) {
    mURL->setURL( res->mURL.prettyURL() );
    kdDebug(5800) << "Format typeid().name(): " << typeid( res->mFormat ).name() << endl;
    if ( typeid( *(res->mFormat) ) == typeid( ICalFormat ) )
      formatGroup->setButton( 0 );
    else if ( typeid( *(res->mFormat) ) == typeid( VCalFormat ) )
      formatGroup->setButton( 1 );
    else 
      kdDebug(5800) << "ERROR: ResourceLocalConfig::loadSettings(): Unknown format type" << endl;
  } else
    kdDebug(5800) << "ERROR: ResourceLocalConfig::loadSettings(): no ResourceLocal, cast failed" << endl;
}

void ResourceLocalConfig::saveSettings( KRES::Resource *resource )
{
  QString url = mURL->url();

  if( url.isEmpty() ) {
    KStandardDirs dirs;
    QString saveFolder = dirs.saveLocation( "data", "korganizer" );
    QFile file( saveFolder + "/std.ics" );
    
    // find a non-existent name
    for( int i = 0; file.exists(); ++i )
      file.setName( saveFolder + "/std" + QString::number(i) + ".ics" );
    
    KMessageBox::information( this, i18n( "You did not specify a URL for this resource. Therefore, the resource will be saved in %1. It is still possible to change this location by editing the resource properties." ).arg( file.name() ) );
    
    url = file.name();
  }

  ResourceLocal* res = static_cast<ResourceLocal*>( resource );
  if (res) {
    res->mURL = url;

    delete res->mFormat;
    if ( icalButton->isOn() ) {
      res->mFormat = new ICalFormat();
    } else {
      res->mFormat = new VCalFormat();
    }
  } else
    kdDebug(5800) << "ERROR: ResourceLocalConfig::saveSettings(): no ResourceLocal, cast failed" << endl;
}

#include "resourcelocalconfig.moc"
