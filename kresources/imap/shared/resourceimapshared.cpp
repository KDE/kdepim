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

#include <kdebug.h>
#include <klocale.h>

#include "resourceimapshared.h"
#include "kmailconnection.h"

using namespace ResourceIMAPBase;


ResourceIMAPShared::ResourceIMAPShared( const QCString& objId )
{
  mConnection = new KMailConnection( this, objId );
}

ResourceIMAPShared::~ResourceIMAPShared()
{
  delete mConnection;
}


bool ResourceIMAPShared::kmailIncidences( QStringList& lst,
                                          const QString& type )
{
  return mConnection->kmailIncidences( lst, type );
}

bool ResourceIMAPShared::kmailAddIncidence( const QString& type,
                                            const QString& uid,
                                            const QString& incidence )
{
  return mConnection->kmailAddIncidence( type, uid, incidence );
}

bool ResourceIMAPShared::kmailDeleteIncidence( const QString& type,
                                               const QString& uid )
{
  return mConnection->kmailDeleteIncidence( type, uid );
}

bool ResourceIMAPShared::kmailUpdate( const QString& type,
                                      const QStringList& lst )
{
  return mConnection->kmailUpdate( type, lst );
}

bool ResourceIMAPShared::kmailUpdate( const QString& type, const QString& uid,
                                      const QString& incidence )
{
  return mConnection->kmailUpdate( type, uid, incidence );
}

bool ResourceIMAPShared::connectToKMail() const
{
  return mConnection->connectToKMail();
}
