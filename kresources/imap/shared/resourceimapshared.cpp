/*
    This file is part of the IMAP resources.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

#include <klocale.h>
#include <kstandarddirs.h>

#include "resourceimapshared.h"
#include "kmailconnection.h"

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

QString ResourceIMAPShared::configFile( const QString& type ) const
{
  return locateLocal( "config",
                      QString( "kresources/imap/%1rc" ).arg( type ) );
}

bool ResourceIMAPShared::connectToKMail() const
{
  return mConnection->connectToKMail();
}
