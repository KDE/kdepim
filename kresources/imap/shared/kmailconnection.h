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

#ifndef IMAPSHARED_KMAILCONNECTION_H
#define IMAPSHARED_KMAILCONNECTION_H

#include <dcopobject.h>
#include <qstringlist.h>

class DCOPClient;
class KMailICalIface_stub;

namespace ResourceIMAPBase {

  class ResourceIMAPShared;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class KMailConnection : public QObject, public DCOPObject {
  Q_OBJECT
  K_DCOP

  // These are the methods called by KMail when the resource changes
k_dcop:
  bool addIncidence( const QString& type, const QString& resource,
                     const QString& ical );
  void deleteIncidence( const QString& type, const QString& resource,
                        const QString& uid );
  void slotRefresh( const QString& type, const QString& resource );
  void subresourceAdded( const QString& type, const QString& resource );
  void subresourceDeleted( const QString& type, const QString& resource );
  void asyncLoadResult( const QStringList& list, const QString& type,
                        const QString& folder );

public:
  KMailConnection( ResourceIMAPShared* resource, const QCString& objId );
  virtual ~KMailConnection();

  /**
   * Do the connection to KMail.
   */
  bool connectToKMail();

  // Call the DCOP methods
  bool kmailIncidences( QStringList& lst, const QString& type,
                        const QString& resource );
  bool kmailSubresources( QStringList& lst, const QString& type );
  bool kmailAddIncidence( const QString& type, const QString& uid,
                          const QString& incidence,
                          const QString& resource );
  bool kmailDeleteIncidence( const QString& type, const QString& uid,
                             const QString& resource );
  bool kmailUpdate( const QString& type, const QString& resource,
                    const QStringList& lst );
  bool kmailUpdate( const QString& type, const QString& resource,
                    const QString& uid, const QString& incidence );
  bool kmailIsWritableFolder( const QString& type,
                              const QString& resource );

private slots:
  virtual void unregisteredFromDCOP( const QCString& );

private:
  /** Connect a signal from KMail to a local slot. */
  bool connectKMailSignal( const QCString&, const QCString& );

  ResourceIMAPShared* mResource;
  DCOPClient* mDCOPClient;
  KMailICalIface_stub* mKMailIcalIfaceStub;
};

}

#endif // IMAPSHARED_KMAILCONNECTION_H
