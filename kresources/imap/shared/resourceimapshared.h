 /*
    This file is part of libkcal.

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

#ifndef IMAPSHARED_H
#define IMAPSHARED_H

#include <qstring.h>
#include <qmap.h>

class QCString;
class QStringList;

namespace ResourceIMAPBase {
  class KMailConnection;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class ResourceIMAPShared {
public:
  ResourceIMAPShared( const QCString& objId );
  virtual ~ResourceIMAPShared();

  // These are the methods called by KMail when the resource changes
  virtual bool addIncidence( const QString& type, const QString& resource,
                             const QString& ical ) = 0;
  virtual void deleteIncidence( const QString& type, const QString& resource,
                                const QString& uid ) = 0;
  virtual void slotRefresh( const QString& type,
                            const QString& resource ) = 0;
  virtual void subresourceAdded( const QString& type,
                                 const QString& resource ) = 0;
  virtual void subresourceDeleted( const QString& type,
                                   const QString& resource ) = 0;
  virtual void asyncLoadResult( const QStringList&, const QString&, const QString& ) = 0;

protected:
  /** Do the connection to KMail. */
  bool connectToKMail() const;

  // These are the KMail dcop functions
  bool kmailIncidences( QStringList& lst, const QString& type,
                        const QString& resource ) const;
  bool kmailSubresources( QStringList& lst, const QString& type ) const;
  bool kmailAddIncidence( const QString& type, const QString& resource,
                          const QString& uid, const QString& incidence );
  bool kmailDeleteIncidence( const QString& type, const QString& resource,
                             const QString& uid );
  bool kmailUpdate( const QString& type, const QString& resource,
                    const QStringList& lst );
  bool kmailUpdate( const QString& type, const QString& resource,
                    const QString& uid, const QString& incidence );
  bool kmailIsWritableFolder( const QString& type, const QString& resource );

  /** Get the full path of the config file. */
  QString configFile( const QString& type ) const;

  /** If only one of these is writable, return that. Otherwise return null. */
  QString findWritableResource( const QMap<QString, bool>& resources,
                                const QString& type );
  QString findWritableResource( const QStringList& resources,
                                const QString& type );

  bool mSilent;

private:
  mutable KMailConnection* mConnection;
};

}

#endif // IMAPSHARED_H
