 /*
    This file is part of the kolab resource.

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

#ifndef KMAILCONNECTION_H
#define KMAILCONNECTION_H

#include <dcopobject.h>

class KURL;
class DCOPClient;
class KMailICalIface_stub;

namespace ResourceKolab {

class ResourceKolabBase;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class KMailConnection : public QObject, public DCOPObject {
  Q_OBJECT
  K_DCOP

  // These are the methods called by KMail when the resource changes
#if 0
  // TODO: Reimplement these
//k_dcop:
  bool addIncidence( const QString& type, const QString& resource,
                     const QString& ical );
  void deleteIncidence( const QString& type, const QString& resource,
                        const QString& uid );
  void slotRefresh( const QString& type, const QString& resource );
  void subresourceAdded( const QString& type, const QString& resource );
  void subresourceDeleted( const QString& type, const QString& resource );
#endif

public:
  KMailConnection( ResourceKolabBase* resource, const QCString& objId );
  virtual ~KMailConnection();

  /**
   * Do the connection to KMail.
   */
  bool connectToKMail();

  // Call the DCOP methods
  bool kmailSubresources( QMap<QString, bool>& lst,
                          const QString& annotation );
  bool kmailIncidences( QMap<QString, QString>& lst,
                        const QString& mimetype,
                        const QString& resource );
  bool kmailGetAttachment( KURL& url, const QString& resource,
                           const QString& sernum,
                           const QString& filename );
  bool kmailDeleteIncidence( const QString& resource, const QString& sernum );
  bool kmailUpdate( const QString& resource, const QString& sernum,
                    const QStringList& attachments,
                    const QStringList& deletedAttachments );

private slots:
  virtual void unregisteredFromDCOP( const QCString& );

private:
  /** Connect a signal from KMail to a local slot. */
  bool connectKMailSignal( const QCString&, const QCString& );

  ResourceKolabBase* mResource;
  DCOPClient* mDCOPClient;
  KMailICalIface_stub* mKMailIcalIfaceStub;
};

}

#endif // KMAILCONNECTION_H
