/*
    This file is part of the IMAP resources.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

#include "resourceimapshared.h"
#include "kmailconnection.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>

using namespace ResourceIMAPBase;


ResourceIMAPShared::ResourceIMAPShared( const QCString& objId )
  : mSilent( false )
{
  mConnection = new KMailConnection( this, objId );
}

ResourceIMAPShared::~ResourceIMAPShared()
{
  delete mConnection;
}


bool ResourceIMAPShared::kmailIncidences( QStringList& lst,
                                          const QString& type,
                                          const QString& resource ) const
{
  return mSilent || mConnection->kmailIncidences( lst, type, resource );
}

bool ResourceIMAPShared::kmailSubresources( QStringList& lst,
                                            const QString& type ) const
{
  return mSilent || mConnection->kmailSubresources( lst, type );
}

bool ResourceIMAPShared::kmailAddIncidence( const QString& type,
                                            const QString& resource,
                                            const QString& uid,
                                            const QString& incidence )
{
  return mSilent ||
    mConnection->kmailAddIncidence( type, resource, uid, incidence );
}

bool ResourceIMAPShared::kmailDeleteIncidence( const QString& type,
                                               const QString& resource,
                                               const QString& uid )
{
  return mSilent || mConnection->kmailDeleteIncidence( type, resource, uid );
}

bool ResourceIMAPShared::kmailUpdate( const QString& type,
                                      const QString& resource,
                                      const QStringList& lst )
{
  return mSilent || mConnection->kmailUpdate( type, resource, lst );
}

bool ResourceIMAPShared::kmailUpdate( const QString& type,
                                      const QString& resource,
                                      const QString& uid,
                                      const QString& incidence )
{
  return mSilent || mConnection->kmailUpdate( type, resource, uid, incidence );
}

bool ResourceIMAPShared::kmailIsWritableFolder( const QString& type,
                                                const QString& resource )
{
  return mConnection->kmailIsWritableFolder( type, resource );
}

QString ResourceIMAPShared::configFile( const QString& type ) const
{
  return locateLocal( "config",
                      QString( "kresources/imap/%1rc" ).arg( type ) );
}

bool ResourceIMAPShared::connectToKMail() const
{
  return mConnection->connectToKMail();
}

QString ResourceIMAPShared::findWritableResource( const QMap<QString, bool>& resources,
                                                  const QString& type )
{
  QStringList possible;
  QMap<QString, bool>::ConstIterator it;
  for ( it = resources.begin(); it != resources.end(); ++it )
    if ( it.data() )
      // Writable possibility
      possible << it.key();
  return findWritableResource( possible, type );
}

QString ResourceIMAPShared::findWritableResource( const QStringList& resources,
                                                  const QString& type )
{
  QStringList possible;
  QStringList::ConstIterator it;
  for ( it = resources.begin(); it != resources.end(); ++it ) {
    // Ask KMail if this one is writable
    if ( kmailIsWritableFolder( type, *it ) )
      // Writable possibility
      possible << *it;
  }

  if ( possible.isEmpty() )
    // None found!!
    return QString::null;
  if ( possible.count() == 1 )
    // Just one found
    return possible[ 0 ];

  // Several found, ask the user
  return KInputDialog::getItem( i18n( "Select Resource Folder" ),
                                i18n( "You have more than one writable resource folder. "
                                      "Please select the one you want to write to:" ),
                                possible );
}
